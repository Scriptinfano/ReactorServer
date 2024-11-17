#pragma once
#include "eventloop.hpp"
#include "mysocket.hpp"
#include "channel.hpp"
#include <functional>
#include <memory>
class Accepter
{

private:
    EventLoop *loop_;  // 指明这个Accepter属于哪一个事件循环，因为多线程服务器中最顶层的TCPServer可能有多个事件循环
    std::unique_ptr<Socket> servsock_; // 负责监听传入连接的服务端套接字
    std::unique_ptr<Channel> acceptchannel_;//管理epoll的channel
    std::function<void(int, InetAddress &)> acceptCallBack_; // 回调到TCPServer中，在上层类中构建Connection接管连接
public:
    /*
    Accepter一旦初始化，内部的servsock就根据设置初始化完成并开始监听
    */
    Accepter(EventLoop *loop, const std::string &ip, const in_port_t port);
    ~Accepter();
    void handleNewConnection();
    void setAcceptCallBack(std::function<void(int, InetAddress &)> acceptCallBack);
};