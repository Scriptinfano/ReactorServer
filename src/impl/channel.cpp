#include <unistd.h>

#include "channel.hpp"
#include "log.hpp"
#include "mysocket.hpp"
Channel::Channel(Epoll *ep, int fd, Socket *sock, bool islistenfd) : ep_(ep), fd_(fd), sock_(sock), islistenfd_(islistenfd)
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
void Channel::handleEvent()
{
    // 判断套接字的类型，是监听套接字来新连接了，还是已有的客户连接就绪了
    if (islistenfd_) // 还可以直接判断ch是否和server_channel的值相同
    {
        if (revents_ & EPOLLHUP)
        {
            logger.logMessage(FATAL, __FILE__, __LINE__, "可能未调用listen函数使得监听套接字变为被动监听状态");
            exit(-1);
        }
        // 遇到监听套接字的情况
        InetAddress clientaddr;
        // clientsock只能new出来，放到堆中以避免被调用析构函数关闭fd
        Socket *clientsock = new Socket(sock_->accept(clientaddr));
        clientsock->setNonBlocking(true); // 在边缘触发模式下的epollsevrer必须将clintsock设为非阻塞模式
        Channel *clien_channel = new Channel(ep_, clientsock->fd(), clientsock);
        clien_channel->set_ET();             // 一定要在start_monitor_read()之前调用设置边缘触发的方法
        clien_channel->start_monitor_read(); // 加入epoll的监视，开始监视这个channel的可读事件
    }
    else
    {
        // 遇到客户连接的文件描述符就绪
        if (revents_ & EPOLLRDHUP)
        {
            logger.logMessage(WARNING, __FILE__, __LINE__, "client socket(%d) closed the connection", fd_);
            close(fd_);
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
                    // 读取完成之后原封不动的发送回去
                    send(fd_, buffer, sizeof(buffer), 0);
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
                    // 客户端连接已断开
                    logger.logMessage(WARNING, __FILE__, __LINE__, "client socket(%d) closed the connection", fd_);
                    close(fd_);
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
}