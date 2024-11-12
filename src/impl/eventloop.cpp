#include "eventloop.hpp"

EventLoop::EventLoop() : ep_(new Epoll)
{
}

EventLoop::~EventLoop()
{
}

void EventLoop::run()
{
    while (true)
    {
        std::vector<Channel *> chans = ep_->loop(); // 开始等待就绪事件发生

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