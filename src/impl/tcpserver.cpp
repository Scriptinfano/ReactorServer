#include "tcpserver.hpp"
#include "mysocket.hpp"
#include "channel.hpp"
#include "connection.hpp"
#include "log.hpp"
#include <unistd.h>
TCPServer::TCPServer(const std::string &ip, const in_port_t port)
{
    loop_ = new EventLoop();
    accepter_ = new Accepter(loop_, ip, port);
    accepter_->setAcceptFunc(std::bind(&TCPServer::handleNewConnection, this, std::placeholders::_1, std::placeholders::_2));
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
void TCPServer::handleNewConnection(int fd, InetAddress clientaddr)
{
    Connection *conn = new Connection(loop_, fd);
    conn->setCloseCallBack(std::bind(&TCPServer::closeConnectionCallBack, this, std::placeholders::_1));
    conn->setErrorCallBack(std::bind(&TCPServer::errorConnectionCallBack, this, std::placeholders::_1));
    logger.logMessage(DEBUG, __FILE__, __LINE__, "accept client(fd=%d,ip=%s,port=%d) ok", fd, clientaddr.ip().c_str(), clientaddr.port());
    connectionMapper_[conn->getFd()] = conn; // 将Connection的地址放入映射器管理起来
}
void TCPServer::closeConnectionCallBack(Connection *conn)
{
    int fd = conn->getFd();
    logger.logMessage(WARNING, __FILE__, __LINE__, "client socket(%d) closed the connection", fd);
    connectionMapper_.erase(conn->getFd());
    delete conn; // Connection的析构函数中会析构ClientSocket，ClientSocket的析构函数
}
void TCPServer::errorConnectionCallBack(Connection *conn)
{
    int fd = conn->getFd();
    logger.logMessage(WARNING, __FILE__, __LINE__, "client socket(%d) occur unknown error", fd);
    connectionMapper_.erase(conn->getFd());
    delete conn;
}
