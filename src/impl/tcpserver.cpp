#include "tcpserver.hpp"
#include "mysocket.hpp"
#include "channel.hpp"
#include "connection.hpp"
#include "log.hpp"
#include <unistd.h>
TCPServer::TCPServer(const std::string &ip, const in_port_t port)
{
    loop_ = new EventLoop();
    loop_->setEpollTimeoutCallBack(std::bind(&TCPServer::epollTimeoutCallBack, this, std::placeholders::_1));
    accepter_ = new Accepter(loop_, ip, port);
    accepter_->setAcceptCallBack(std::bind(&TCPServer::acceptCallBack, this, std::placeholders::_1, std::placeholders::_2));
}
TCPServer::~TCPServer()
{
    delete loop_;
    delete accepter_;
    // 释放全部的Connection对象
    for (auto &kv : connectionMapper_)
    {
        delete kv.second; // 取出键值对的值
    }
}
void TCPServer::start()
{
    loop_->run();
}
void TCPServer::acceptCallBack(int fd, InetAddress clientaddr)
{
    Connection *conn = new Connection(loop_, fd, &clientaddr);
    // 这里将clientaddr的值传入Connection内部
    conn->setCloseCallBack(std::bind(&TCPServer::closeCallBack, this, std::placeholders::_1));
    conn->setErrorCallBack(std::bind(&TCPServer::errorCallBack, this, std::placeholders::_1));
    conn->setProcessCallBack(std::bind(&TCPServer::processCallBack, this, std::placeholders::_1, std::placeholders::_2));
    conn->setSendCompleteCallBack(std::bind(&TCPServer::sendCompleteCallBack, this, std::placeholders::_1));
    logger.logMessage(NORMAL, __FILE__, __LINE__, "accept client(fd=%d,ip=%s,port=%d) ok", fd, clientaddr.ip().c_str(), clientaddr.port());
    connectionMapper_[conn->getFd()] = conn; // 将Connection的地址放入映射器管理起来
    if (acceptCallBack_)
        acceptCallBack_(conn);
}
void TCPServer::closeCallBack(Connection *conn)
{
    if (closeCallBack_)
        closeCallBack_(conn);
    int fd = conn->getFd();
    logger.logMessage(WARNING, __FILE__, __LINE__, "client socket(%d) closed the connection", fd);
    connectionMapper_.erase(conn->getFd());
    delete conn; // Connection的析构函数中会析构ClientSocket，ClientSocket的析构函数
}
void TCPServer::errorCallBack(Connection *conn)
{
    if (errorCallBack_)
        errorCallBack_(conn);
    int fd = conn->getFd();
    logger.logMessage(WARNING, __FILE__, __LINE__, "client socket(%d) occur unknown error", fd);
    connectionMapper_.erase(conn->getFd());
    delete conn;
}
void TCPServer::processCallBack(Connection *conn, std::string& message)
{
    // 其实在这里也不应该直接处理业务，而是应该再创建一个业务处理类，让业务处理类区处理业务数据，这样的话，结构更加清晰
    // 根据业务需求也可以有其他代码
    if (processCallBack_)
        processCallBack_(conn, message);
}
void TCPServer::sendCompleteCallBack(Connection *conn)
{
    logger.logMessage(NORMAL, __FILE__, __LINE__, "send complete");
    // 根据业务需求也可以有其他代码
    if (sendCompleteCallBack_)
        sendCompleteCallBack_(conn);
}
void TCPServer::epollTimeoutCallBack(EventLoop *loop)
{
    logger.logMessage(NORMAL, __FILE__, __LINE__, "start to handle epolltimeout situation");
    if (epollTimeoutCallBack_)
        epollTimeoutCallBack_(loop);
}

void TCPServer::setAcceptCallBack(std::function<void(Connection *)> acceptCallBack)
{
    acceptCallBack_ = acceptCallBack;
}
void TCPServer::setCloseCallBack(std::function<void(Connection *)> closeCallBack)
{
    closeCallBack_ = closeCallBack;
}
void TCPServer::setErrorCallBack(std::function<void(Connection *)> errorCallBack)
{
    errorCallBack_ = errorCallBack;
}
void TCPServer::setProcessCallBack(std::function<void(Connection *, std::string&)> processCallBack)
{
    processCallBack_ = processCallBack;
}
void TCPServer::setSendCompleteCallBack(std::function<void(Connection *)> sendCompleteCallBack)
{
    sendCompleteCallBack_ = sendCompleteCallBack;
}
void TCPServer::setEpollTimeoutCallBack(std::function<void(EventLoop *)> epollTimeoutCallBack)
{
    epollTimeoutCallBack_ = epollTimeoutCallBack;
}