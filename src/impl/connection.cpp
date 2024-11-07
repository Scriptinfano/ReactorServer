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
    clientchannel_->setReadCallBack(std::bind(&Channel::handleNewMessage, clientchannel_));
    clientchannel_->setCloseCallBack(std::bind(&Connection::closeCallBack, this));
    clientchannel_->setErrorCallBack(std::bind(&Connection::errorCallBack, this));
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
    int fd = getFd();
    logger.logMessage(WARNING, __FILE__, __LINE__, "client socket(%d) closed the connection", fd);
    // TCP连接在这里断开的时候可以通过回调函数通知上层的TCPServer类，先回调到Connection的成员函数上，然后Connection再回调到TCPServer上
    close(fd);
}

void Connection::errorCallBack()
{
    logger.logMessage(ERROR, __FILE__, __LINE__, "client socket(%d) occur unknown error", getFd());
}