#include "connection.hpp"
#include "log.hpp"
#include <unistd.h>
Connection::Connection(EventLoop *loop, int fd) : loop_(loop)
{
    // clientsock只能new出来，
    clientsock_ = new Socket(fd);
    clientsock_->setNonBlocking(true); // 在边缘触发模式下的epollsevrer必须将clintsock设为非阻塞模式
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
        char buffer[1024] = {0};
        while (true)
        {
            
            ssize_t nread = read(getFd(), buffer, sizeof(buffer)); // 先读取报文的头部获取后面要读取多少字节的数据
            if (nread > 0)
            {
                inputBuffer_.append(buffer, nread);
                logger.logMessage(DEBUG, __FILE__, __LINE__, "The read data has been placed into the buffer");
            }
            else if (nread == -1 && errno == EINTR)
            {
                // 读取数据的时候被信号中断，继续读取。
                logger.logMessage(NORMAL, __FILE__, __LINE__, "读取数据的时候被信号中断，继续读取。");
                continue;
            }
            else if (nread == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK)))
            {
                // 全部的数据已读取完毕
                logger.logMessage(NORMAL, __FILE__, __LINE__, "recv from client(fd=%d,ip=%s,port=%hu):%s",getFd(),clientsock_->getIP().c_str(),clientsock_->getPort(),inputBuffer_.getData());
                // 一般服务器会在这里对接收的数据做一些运算，然后再返回给客户端
                outputBuffer_ = inputBuffer_;
                inputBuffer_.clear();
                const char *temp = "（此消息已被服务端处理）";
                outputBuffer_.append(temp,sizeof(temp));
                send(getFd(), outputBuffer_.getData(), outputBuffer_.getSize(), 0);
                //outputBuffer_.clear();
                break;
            }
            else if (nread == 0)
            {
                // 连接在这里断开的时候可以通过回调函数通知上层的TCPServer类，先回调到Connection的成员函数上，然后Connection再回调到TCPServer上
                closeCallBack();
                break;
            }
            else
            {
                // 出现其他错误的情况可以通过回调函数通知上层的TCPServer类，先回调到Connection的成员函数上，然后Connection再回调到TCPServer上
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