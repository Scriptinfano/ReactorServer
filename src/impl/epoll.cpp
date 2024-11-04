#include "epoll.hpp"
#include "log.hpp"
#include "channel.hpp"
#include <stdexcept>
Epoll::Epoll()
{
    epollfd_ = epoll_create1(EPOLL_CLOEXEC);
    if (epollfd_ == -1)
    {
        throw std::runtime_error("epoll_create1() failed");
    }
}
Epoll::~Epoll()
{
}

void Epoll::updateChannel(Channel *ch)
{
    epoll_event ev;
    ev.data.ptr = ch;
    ev.events = ch->events();
    if (ch->inepoll())
    {
        // 如果已经在树上则更新
        if (epoll_ctl(epollfd_, EPOLL_CTL_MOD, ch->fd(), &ev) == -1)
        {
            logger.logMessage(FATAL, __FILE__, __LINE__, logger.createErrorMessage("epoll_ctl() failed").c_str());
            exit(-1);
        }
    }
    else
    {
        // 如果不在树上则添加
        if (epoll_ctl(epollfd_, EPOLL_CTL_ADD, ch->fd(), &ev) == -1)
        {
            logger.logMessage(FATAL, __FILE__, __LINE__, logger.createErrorMessage("epoll_ctl() failed").c_str());
            exit(-1);
        }
        ch->set_inepoll();
    }
}

void Epoll::addfd(int fd, uint32_t op)
{
    epoll_event ev;
    ev.data.fd = fd;
    ev.events = op;
    if (epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &ev) == -1)
    {
        logger.logMessage(FATAL, __FILE__, __LINE__, logger.createErrorMessage("epoll_ctl() failed").c_str());
        exit(-1);
    }
}
std::vector<Channel *> Epoll::loop(int timeout)
{
    std::vector<Channel *> channels; // 存放epoll_wait返回的事件
    /*
    std::fill()函数将events数组中的每个元素初始化为0，begin()函数返回指向events数组的第一个元素的迭代器
    end(events)函数返回指向events数组的最后一个元素后一个位置的迭代器
    第三个参数epoll_event{}是用来填充的值，它是epoll_event类型的默认构造值
    */
    std::fill(std::begin(channels), std::end(channels), nullptr);
    int rfdnum = epoll_wait(epollfd_, events_, MAXEVENTS, timeout);
    if (rfdnum < 0)
    {
        // 调用失败的情况
        logger.logMessage(FATAL, __FILE__, __LINE__, "epoll_wait() failed");
        exit(-1);
    }
    if (rfdnum == 0)
    {
        // 超时的情况
        logger.logMessage(DEBUG, __FILE__, __LINE__, "epoll_wait() timeout");
        return channels;
    }
    for (int i = 0; i < rfdnum; i++)
    {
        Channel *ch = (Channel *)events_[i].data.ptr;
        ch->set_revents(events_[i].events);
        channels.push_back(ch);
    }
    return channels;
}