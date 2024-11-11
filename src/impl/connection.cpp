#include <unistd.h>
#include <string.h>
#include "connection.hpp"
#include "log.hpp"
Connection::Connection(EventLoop *loop, int fd, InetAddress *clientaddr) : loop_(loop)
{
    // clientsock只能new出来，
    clientsock_ = new Socket(fd);
    clientsock_->setNonBlocking(true); // 在边缘触发模式下的epollsevrer必须将clintsock设为非阻塞模式
    clientsock_->setIp(clientaddr->ip());
    clientsock_->setPort(clientaddr->port());
    clientchannel_ = new Channel(loop_, clientsock_->getFd());
    clientchannel_->setETMode(); // 一定要在start_monitor_read()之前调用设置边缘触发的方法
    clientchannel_->setReadCallBack(std::bind(&Connection::handleNewMessage, this));
    clientchannel_->monitorReadEvent(); // 加入epoll的监视，开始监视这个channel的可读事件
}
Connection::~Connection()
{
    delete clientsock_;
    delete clientchannel_;
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
    closeCallBack_(this);
}

void Connection::errorCallBack()
{
    errorCallBack_(this);
}

void Connection::setCloseCallBack(std::function<void(Connection *)> closeCallBack)
{
    closeCallBack_ = closeCallBack;
}

void Connection::setErrorCallBack(std::function<void(Connection *)> errorCallBack)
{
    errorCallBack_ = errorCallBack;
}

void Connection::handleNewMessage()
{
    // 遇到客户连接的文件描述符就绪
    if (clientchannel_->getRevents() & EPOLLRDHUP)
    {
        closeCallBack();
    }
    else if (clientchannel_->getRevents() & (EPOLLIN | EPOLLPRI))
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
                    logger.logMessage(NORMAL, __FILE__, __LINE__, "recv from client(fd=%d,ip=%s,port=%u):%s", getFd(), getIP().c_str(), getPort(), message.c_str());
                    // 在这里，将经过若干步骤的运算。
                    message = "reply:" + message;
                    len = message.size();                           // 计算回应报文的大小。
                    std::string tmpbuf((char *)&len, 4);            // 把报文头部填充到回应报文中。
                    tmpbuf.append(message);                         // 把报文内容填充到回应报文中。
                    send(getFd(), tmpbuf.data(), tmpbuf.size(), 0); // 把临时缓冲区中的数据直接send()出去。
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
    else if (clientchannel_->getRevents() & EPOLLOUT)
    {
        // 暂时没有代码
    }
    else
    {
        logger.logMessage(WARNING, __FILE__, __LINE__, "client socket(%d) error", getFd());
        close(getFd());
    }
}