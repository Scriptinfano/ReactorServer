#pragma once
#include <sys/epoll.h>
#include <vector>
#include "channel.hpp"
class Channel; // 如果两个头文件互相包含，互相需要对方的数据结构，那么需要在两个文件做对方的前向声明，而且要在头文件的首部加入#pragme once
/*
对epoll相关底层API进行封装
*/
class Epoll
{
private:
    static const int MAXEVENTS = 100;
    int epollfd_ = -1;
    epoll_event events_[MAXEVENTS];

public:
    Epoll();
    ~Epoll();
    /*
    将fd和它需要监视的事件添加到红黑树上
    */
    void addfd(int fd, uint32_t op);
    /*
    将Channel添加或更新到红黑树上，Channel中有fd，也有需要监视的事件
    */
    void updateChannel(Channel *ch);
    std::vector<Channel *> loop(int timeout = -1);
};