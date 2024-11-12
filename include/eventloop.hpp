#pragma once
#include "epoll.hpp"
#include "channel.hpp"
#include<functional>
class Channel;
class Epoll;
/*
事件循环
*/
class EventLoop
{
private:
    Epoll *ep_;
    std::function<void(EventLoop *)> epollTimeoutCallBack_;

public:
    EventLoop();

    ~EventLoop();
    /*
    运行事件循环
    */
    void run();

    void updateChannel(Channel *chan);

    void setEpollTimeoutCallBack(std::function<void(EventLoop *)> epollTimeoutCallBack);

};