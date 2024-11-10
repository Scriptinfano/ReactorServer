#include "channel.hpp"

Channel::Channel(EventLoop *loop, int fd) : loop_(loop), fd_(fd)
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

void Channel::setETMode()
{
    events_ = events_ | EPOLLET;
}

void Channel::monitorReadEvent()
{
    events_ = events_ | EPOLLIN;

    loop_->getEpoll()->updateChannel(this);
}

void Channel::setInEpoll()
{
    inepoll_ = true;
}

void Channel::setRevents(uint32_t revs)
{
    revents_ = revs;
}

bool Channel::getInEpoll()
{
    return inepoll_;
}

uint32_t Channel::getEvents()
{
    return events_;
}

uint16_t Channel::getRevents()
{
    return revents_;
}
void Channel::handleEvent()
{
    readcallback_();
}

void Channel::setReadCallBack(std::function<void()> func)
{
    readcallback_ = func;
}