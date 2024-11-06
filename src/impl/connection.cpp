#include "connection.hpp"
Connection::Connection(EventLoop *loop,int fd):loop_(loop){
    // clientsock只能new出来，
    clientsock_ = new Socket(fd);
    clientsock_->setNonBlocking(true); // 在边缘触发模式下的epollsevrer必须将clintsock设为非阻塞模式
    clientchannel_ = new Channel(loop_, clientsock_->fd(), clientsock_);
    clientchannel_->setETMode(); // 一定要在start_monitor_read()之前调用设置边缘触发的方法
    clientchannel_->setReadCallBack(std::bind(&Channel::handleNewMessage, clientchannel_));
    clientchannel_->monitorReadEvent(); // 加入epoll的监视，开始监视这个channel的可读事件
}
Connection::~Connection(){
    delete clientsock_;
    delete clientchannel_;
}