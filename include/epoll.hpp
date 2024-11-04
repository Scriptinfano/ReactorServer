#pragma once
#include <sys/epoll.h>
#include<vector>
class Epoll
{
private:
    static const int MAXEVENTS = 100;
    int epollfd_ = -1;
    epoll_event events_[MAXEVENTS];

public:
    Epoll();
    ~Epoll();
    void addfd(int fd, uint32_t op);
    std::vector<epoll_event> loop(int timeout = -1);
};