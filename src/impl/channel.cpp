#include "channel.hpp"

Channel::Channel(Epoll *ep, int fd) : ep_(ep), fd_(fd)
{
}
Channel::~Channel()
{
    // 注意在析构函数中不要销毁ep_，也不能关闭fd_，因为他们不属于Channel类，Channel类只是需要他们，使用他们而已
}

int Channel::fd()
{
    return fd_;
}

void Channel::set_ET()
{
    events_ = events_ | EPOLLET;
}

void Channel::start_monitor_read()
{
    events_ = events_ | EPOLLIN;
    ep_->updateChannel(this);
}

void Channel::set_inepoll()
{
    inepoll_ = true;
}

void Channel::set_revents(uint32_t revs)
{
    revents_ = revs;
}

bool Channel::inepoll()
{
    return inepoll_;
}

uint32_t Channel::events()
{
    return events_;
}

uint16_t Channel::revents()
{
    return revents_;
}