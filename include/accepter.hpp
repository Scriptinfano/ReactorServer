#pragma once
#include "eventloop.hpp"
#include "mysocket.hpp"
#include "channel.hpp"
class Accepter{

private:
    EventLoop *loop_;//一个Accepter对应一个事件循环，在构造函数中传入
    Socket *servsock_;
    Channel *acceptchannel_;

public:
    Accepter(EventLoop *loop, const std::string &ip, const in_port_t port);
    ~Accepter();  
};