#pragma once
#include "epoll.hpp"
#include "channel.hpp"
class Channel;
class Epoll;
/*
事件循环
*/
class EventLoop
{
private:
    Epoll *ep_;

public:
    EventLoop();

    ~EventLoop();
    /*
    运行事件循环
    */
    void run();

    void updateChannel(Channel *chan);
};