#include <unistd.h>
#include <string.h>
#include "connection.hpp"
#include "log.hpp"
#include <sys/syscall.h>
Connection::Connection(EventLoop *loop, int fd, InetAddress *clientaddr) : loop_(loop), disconnect_(false)
{
    clientsock_ = std::make_unique<Socket>(fd);
    clientsock_->setNonBlocking(true); // 在边缘触发模式下必须将clintsock设为非阻塞模式
    clientsock_->setIp(clientaddr->ip());
    clientsock_->setPort(clientaddr->port());
    clientchannel_ = std::make_unique<Channel>(loop_, clientsock_->getFd());
    clientchannel_->setETMode(); // 一定要在start_monitor_read()之前调用设置边缘触发的方法
    clientchannel_->setReadCallBack(std::bind(&Connection::readCallBack, this));
    clientchannel_->setCloseCallBack(std::bind(&Connection::closeCallBack, this));
    clientchannel_->setErrorCallBack(std::bind(&Connection::errorCallBack, this));
    clientchannel_->setWriteCallBack(std::bind(&Connection::writeCallBack, this));
    clientchannel_->registerReadEvent(); // 加入epoll的监视，开始监视这个channel的可读事件
}
Connection::~Connection()
{
    std::string ip;
    in_port_t port;
    ip = clientsock_->getIP();
    port = clientsock_->getPort();
    logger.logMessage(DEBUG, __FILE__, __LINE__, "连接到(%s:%d)的Connection对象的生命周期已结束", ip.c_str(), port);
}
int Connection::getFd() const
{
    return clientsock_->getFd();
}
std::string Connection::getIP() const
{
    return clientsock_->getIP();
}
in_port_t Connection::getPort() const
{
    return clientsock_->getPort();
}

void Connection::closeCallBack()
{
    disconnect_ = true;
    clientchannel_->removeSelfFromLoop();
    closeCallBack_(shared_from_this());
}

void Connection::errorCallBack()
{
    disconnect_ = true;
    clientchannel_->removeSelfFromLoop();
    errorCallBack_(shared_from_this());
}

void Connection::setCloseCallBack(std::function<void(SharedConnectionPointer)> closeCallBack)
{
    closeCallBack_ = closeCallBack;
}

void Connection::setErrorCallBack(std::function<void(SharedConnectionPointer)> errorCallBack)
{
    errorCallBack_ = errorCallBack;
}

void Connection::readCallBack()
{
    char buffer[1024];
    while (true)
    {
        bzero(&buffer, sizeof(buffer));
        ssize_t nread = read(getFd(), buffer, sizeof(buffer));
        logger.logMessage(DEBUG, __FILE__, __LINE__, "一次read调用完成，实际读取到了%d个字节", nread);
        if (nread > 0)
        {
            logger.logMessage(DEBUG, __FILE__, __LINE__, "此次read调用尚未从底层输入缓冲区中读取所有数据，现将本次读取的数据放入inputBuffer_");
            inputBuffer_.append(buffer, nread);
        }
        else if (nread == -1 && errno == EINTR)
        {
            // 读取数据的时候被信号中断，继续读取。
            logger.logMessage(NORMAL, __FILE__, __LINE__, "读取数据的时候被信号中断，继续读取。");
            continue;
        }
        else if (nread == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK)))
        {
            logger.logMessage(DEBUG, __FILE__, __LINE__, "底层输入缓冲区读到头了");

            while (true)
            {
                // 可以把以下代码封装在Buffer类中，还可以支持固定长度、指定报文长度和分隔符等多种格式。
                int len;
                memcpy(&len, inputBuffer_.getData(), 4); // 从inputbuffer中获取报文头部。
                // 如果inputbuffer中的数据量小于报文头部，说明inputbuffer中的报文内容不完整。
                if (inputBuffer_.getSize() < len + 4)
                {
                    break;
                }
                std::string message(inputBuffer_.getData() + 4, len); // 从inputbuffer中获取一个报文。
                inputBuffer_.erase(0, len + 4);                       // 从inputbuffer中删除刚才已获取的报文。
                logger.logMessage(NORMAL, __FILE__, __LINE__, "thread %d recv from client(fd=%d,ip=%s,port=%u):%s", syscall(SYS_gettid), getFd(), getIP().c_str(), getPort(), message.c_str());
                // 底下这个回调函数代表TCPServer对于客户端数据的处理回调函数
                processCallBack_(shared_from_this(), message);
            }
            break;
        }
        else if (nread == 0)
        {
            closeCallBack();
            break;
        }
        else
        {
            errorCallBack();
            break;
        }
    }
}

void Connection::writeCallBack()
{
    // 把outputBuffer_中的数据发送出去
    logger.logMessage(DEBUG, __FILE__, __LINE__, "Connection::writeCallBack() called, current thread id=%d", syscall(SYS_gettid));
    int writen = ::send(getFd(), outputBuffer_.getData(), outputBuffer_.getSize(), 0);
    if (writen > 0)
        outputBuffer_.erase(0, writen);
    // 这里还要判断发送缓冲区中是否还有数据，如果没有数据了，则不应该再关注写事件
    if (outputBuffer_.getSize() == 0)
    {
        clientchannel_->unregisterWriteEvent();
        sendCompleteCallBack_(shared_from_this());
    }
}

void Connection::setProcessCallBack(std::function<void(SharedConnectionPointer, std::string &)> processCallBack)
{
    processCallBack_ = processCallBack;
}
void Connection::send(const char *data, size_t size)
{
    if (disconnect_ == true)
    {
        logger.logMessage(DEBUG, __FILE__, __LINE__, "客户端连接已断开，Connection::send直接返回", syscall(SYS_gettid));
        return;
    }
    outputBuffer_.appendWithHead(data, size);
    logger.logMessage(DEBUG, __FILE__, __LINE__, "current thread %d has put the data into outputbuffer", syscall(SYS_gettid));
    clientchannel_->registerWriteEvent();
}
void Connection::setSendCompleteCallBack(std::function<void(SharedConnectionPointer)> sendCompleteCallBack)
{
    sendCompleteCallBack_ = sendCompleteCallBack;
}