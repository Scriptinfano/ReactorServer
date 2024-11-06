#pragma once
#include "epoll.hpp"
class EventLoop{
    private:
        Epoll *ep_;

    public:
        EventLoop();
        
        ~EventLoop();
        /*
        运行事件循环
        */
        void run();
        Epoll *getEpoll();
};