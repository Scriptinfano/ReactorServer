#pragma once
#include "eventloop.hpp"
#include "mysocket.hpp"
#include "channel.hpp"
#include <functional>
class Accepter
{

private:
    EventLoop *loop_;  // 指明这个Accepter属于哪一个事件循环，因为多线程服务器中最顶层的TCPServer可能有多个事件循环
    Socket *servsock_; // Accepter接受连接得由具体的套接字负责
    Channel *acceptchannel_;
    std::function<void(int, InetAddress &)> accetpFunc_; // 具体调用相关accept函数的回调
public:
    /*
    Accepter一旦初始化，内部的servsock就根据设置初始化完成并开始监听
    */
    Accepter(EventLoop *loop, const std::string &ip, const in_port_t port);
    ~Accepter();
    void handleNewConnection();
    void setAcceptFunc(std::function<void(int, InetAddress &)> func);
};