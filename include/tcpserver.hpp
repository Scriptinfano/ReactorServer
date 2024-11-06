#pragma once
#include "eventloop.hpp"
/*
顶层封装，整个服务器的抽象
*/
class TCPServer
{
private:
    EventLoop *loop_; // 一个TCPServer可以有多个事件循环，现在是单线程，暂时只用一个事件循环
public:
    TCPServer(const std::string &ip, const in_port_t port);
    ~TCPServer();
    /*
    开始运行事件循环，也就是开始运行这个服务器
    */
    void start();
};