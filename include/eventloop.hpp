#pragma once
#include "epoll.hpp"
#include "channel.hpp"
#include<functional>
#include <memory>
class Channel;
class Epoll;
/*
事件循环
*/
class EventLoop
{
private:
    std::unique_ptr<Epoll> ep_;//实际负责管理epoll。调用epoll相关底层接口的的底层对象
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