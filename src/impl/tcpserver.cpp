#include "tcpserver.hpp"
#include "mysocket.hpp"
#include "channel.hpp"
#include "connection.hpp"
#include "log.hpp"
#include <unistd.h>
TCPServer::TCPServer(const std::string &ip, const in_port_t port)
{
    loop_ = new EventLoop();
    loop_->setEpollTimeoutCallBack(std::bind(&TCPServer::epolltimeout, this, std::placeholders::_1));
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
    conn->setCloseCallBack(std::bind(&TCPServer::closeConnectionCallBack, this, std::placeholders::_1));
    conn->setErrorCallBack(std::bind(&TCPServer::errorConnectionCallBack, this, std::placeholders::_1));
    conn->setProcessCallBack(std::bind(&TCPServer::processCallBack, this, std::placeholders::_1, std::placeholders::_2));
    conn->setSendCompleteCallBack(std::bind(&TCPServer::sendComplete, this, std::placeholders::_1));
    logger.logMessage(NORMAL, __FILE__, __LINE__, "accept client(fd=%d,ip=%s,port=%d) ok", fd, clientaddr.ip().c_str(), clientaddr.port());
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
void TCPServer::processCallBack(Connection *conn, std::string message)
{
    //其实在这里也不应该直接处理业务，而是应该再创建一个业务处理类，让业务处理类区处理业务数据，这样的话，结构更加清晰
    message = "reply:" + message;
    int len = message.size();                  // 计算回应报文的大小。
    std::string tmpbuf((char *)&len, 4);       // 把报文头部填充到回应报文中。
    tmpbuf.append(message);                    // 把报文内容填充到回应报文中。
    conn->send(tmpbuf.c_str(), tmpbuf.size()); // Connection内部的send函数会把数据放到内部的outputBuffer_中，等下一次可写事件发生时会被发送出去
}
void TCPServer::sendComplete(Connection *conn)
{
    logger.logMessage(NORMAL, __FILE__, __LINE__, "send complete");
    // 根据业务需求也可以有其他代码
}
void TCPServer::epolltimeout(EventLoop *loop)
{
    logger.logMessage(NORMAL, __FILE__, __LINE__, "start to handle epolltimeout situation");
    
}