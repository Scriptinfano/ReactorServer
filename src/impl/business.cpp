#include "business.hpp"
EchoServer::EchoServer(const std::string &ip, in_port_t port) : tcpserver_(ip, port)
{
    tcpserver_.setAcceptCallBack(std::bind(&EchoServer::acceptCallBack, this, std::placeholders::_1));
    tcpserver_.setCloseCallBack(std::bind(&EchoServer::closeCallBack, this, std::placeholders::_1));
    tcpserver_.setEpollTimeoutCallBack(std::bind(&EchoServer::epollTimeoutCallBack, this, std::placeholders::_1));
    tcpserver_.setErrorCallBack(std::bind(&EchoServer::errorCallBack, this, std::placeholders::_1));
    tcpserver_.setProcessCallBack(std::bind(&EchoServer::processCallBack, this, std::placeholders::_1, std::placeholders::_2));
    tcpserver_.setSendCompleteCallBack(std::bind(&EchoServer::sendCompleteCallBack, this, std::placeholders::_1));
}

EchoServer::~EchoServer()
{
}

void EchoServer::start()
{
    tcpserver_.start();
}
void EchoServer::acceptCallBack(Connection *conn)
{
}

void EchoServer::closeCallBack(Connection *conn)
{
}

void EchoServer::errorCallBack(Connection *conn)
{
}

void EchoServer::processCallBack(Connection *conn, std::string message)
{
    message = "reply:" + message;
    int len = message.size();                  // 计算回应报文的大小。
    std::string tmpbuf((char *)&len, 4);       // 把报文头部填充到回应报文中。
    tmpbuf.append(message);                    // 把报文内容填充到回应报文中。
    conn->send(tmpbuf.c_str(), tmpbuf.size()); // Connection内部的send函数会把数据放到内部的outputBuffer_中，等下一次可写事件发生时会被发送出去
}

void EchoServer::sendCompleteCallBack(Connection *conn)
{
}

void EchoServer::epollTimeoutCallBack(EventLoop *loop)
{
}