#include <unistd.h>

#include "channel.hpp"
#include "log.hpp"
#include "mysocket.hpp"

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

void Channel::handleNewMessage()
{
    // 遇到客户连接的文件描述符就绪
    if (revents_ & EPOLLRDHUP)
    {
        closecallback_();
    }
    else if (revents_ & (EPOLLIN | EPOLLPRI))
    {
        char buffer[1024] = {0};
        while (true)
        {
            ssize_t nread = read(fd_, buffer, sizeof(buffer));
            if (nread > 0)
            {
                logger.logMessage(NORMAL, __FILE__, __LINE__, "recv from client(fd:%d):%s", fd_, buffer);

                // send(fd_, buffer, sizeof(buffer), 0);// 读取完成之后原封不动的发送回去
            }
            else if (nread == -1 && errno == EINTR)
            {
                // 读取数据的时候被信号中断，继续读取。
                logger.logMessage(NORMAL, __FILE__, __LINE__, "读取数据的时候被信号中断，继续读取。");
                continue;
            }
            else if (nread == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK)))
            {
                // 全部的数据已读取完毕
                logger.logMessage(NORMAL, __FILE__, __LINE__, "全部数据读取完成");
                break;
            }
            else if (nread == 0)
            {
                // 连接在这里断开的时候可以通过回调函数通知上层的TCPServer类，先回调到Connection的成员函数上，然后Connection再回调到TCPServer上
                closecallback_();
                break;
            }
            else
            {
                // 出现其他错误的情况可以通过回调函数通知上层的TCPServer类，先回调到Connection的成员函数上，然后Connection再回调到TCPServer上
                errorcallback_();
                break;
            }
        }
    }
    else if (revents_ & EPOLLOUT)
    {
        // 暂时没有代码
    }
    else
    {
        logger.logMessage(WARNING, __FILE__, __LINE__, "client socket(%d) error", fd_);
        close(fd_);
    }
}

void Channel::setReadCallBack(std::function<void()> func)
{
    readcallback_ = func;
}

void Channel::setCloseCallBack(std::function<void()> func)
{
    closecallback_ = func;
}
void Channel::setErrorCallBack(std::function<void()> func)
{
    errorcallback_ = func;
}