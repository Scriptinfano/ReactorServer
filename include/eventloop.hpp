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

    /*
    根据Channel提供的信息将相应的事件添加到底层的epoll中
    */
    void updateChannel(Channel *chan);

    void setEpollTimeoutCallBack(std::function<void(EventLoop *)> epollTimeoutCallBack);

    /*
    将和chan相关节点从epoll中删除
    */
    void removeChannel(Channel *chan);
};