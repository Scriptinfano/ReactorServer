#pragma once
#include "eventloop.hpp"
#include "accepter.hpp"
#include "connection.hpp"
#include <map>
/*
顶层封装，整个服务器的抽象
*/
class TCPServer
{
private:
    EventLoop *loop_;                              // 一个TCPServer可以有多个事件循环，现在是单线程，暂时只用一个事件循环
    Accepter *accepter_;                           // 一个TCPServer只有一个Accepter类
    std::map<int, Connection *> connectionMapper_; // 一个TCPServer可以有多个Connection，所以用一个Map容器将Connection管理起来

public:
    TCPServer(const std::string &ip, const in_port_t port);
    ~TCPServer();
    /*
    开始运行事件循环，也就是开始运行这个服务器
    */
    void start();
    void handleNewConnection(int fd, InetAddress clientaddr);
    void closeConnectionCallBack(Connection *conn);
    void errorConnectionCallBack(Connection *conn);
};