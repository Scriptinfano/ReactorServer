#include "eventloop.hpp"
#include "log.hpp"
#include <sys/syscall.h>
#include <unistd.h>
EventLoop::EventLoop() : ep_(new Epoll)
{
}

EventLoop::~EventLoop()
{
}

void EventLoop::run()
{
    logger.logMessage(DEBUG, __FILE__, __LINE__, "EventLoop::run() called, thread is %d", syscall(SYS_gettid));
    while (true)
    {
        std::vector<Channel *> chans = ep_->loop(10*1000); // 开始等待就绪事件发生
        // 如果chans是空的，那么应该回调TCPServer中的epolltimeout函数
        if (chans.empty())
        {
            epollTimeoutCallBack_(this);
        }
        // 开始遍历epoll_event数组获得就绪的文件描述符信息
        for (auto &ch : chans)
        {
            ch->handleEvent();
        }
    }
}
void EventLoop::updateChannel(Channel *chan)
{
    ep_->updateChannel(chan);
}
void EventLoop::setEpollTimeoutCallBack(std::function<void(EventLoop *)> epollTimeoutCallBack)
{
    epollTimeoutCallBack_ = epollTimeoutCallBack;
}