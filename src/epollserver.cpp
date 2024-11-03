
#include <iostream>


#include "mysocket.hpp"
#include <unistd.h>
#include <string.h>
#include <sys/epoll.h>
#include <cstring>
#include <string>
#include "errno.h"
#include "inetaddress.hpp"
using namespace std;

class Channel
{
private:
    int fd_;
    bool islisten_ = false; // true-监听的fd，false-客户端连上来的fd。
    // ......更多的成员变量。
public:
    Channel(int fd, bool islisten = false) : fd_(fd), islisten_(islisten) {}
    int fd() { return fd_; }
    bool islisten() { return islisten_; }
    // ......更多的成员函数。
};

int main(int argc, char **argv)
{
    // 检查参数的合法性
    if (argc != 3)
    {
        cout << "usage: epollserver ip port" << endl;
        cout << "example, epollserver 192.168.150.128 8080" << endl;
        return -1;
    } 
    ///////////////

    // 创建监听套接字，设置套接字选项，然后绑定监听地址
    Socket servsock(createNonBlockingSocket());
    InetAddress servaddr(argv[1], atoi(argv[2]));
    servsock.setKeepAlive(true);
    servsock.setReuseAddr(true);
    servsock.setReusePort(true);
    servsock.setTcpNoDelay(true);
    servsock.bind(servaddr);
    /////////////


    // 创建epoll实例，得到它的文件描述符
    int epollfd = epoll_create1(EPOLL_CLOEXEC);
    Channel *servchannel = new Channel(servsock.fd(), true);
    ///////////////////////////////

    // 在epoll模型中添加对epfd的可读事件的监听
    struct epoll_event ev;
    ev.data.ptr = servchannel;
    ev.events = EPOLLIN;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, servsock.fd(), &ev);
    ////////////////////////////////////

    // 创建epoll_wait所需要的epoll_event数组，存放就绪的epoll_event
    struct epoll_event evs[10];

    while (true)
    {
        // 循环中调用epoll_wait，最后一个参数-1代表持续阻塞，直到返回就绪的文件描述符数量
        int infds = epoll_wait(epollfd, evs, 10, -1);

        if (infds < 0)
        {
            cerr << "epoll_wait() failed: " << std::strerror(errno) << endl;
            break;
        }

        if (infds == 0)
        {
            cout << "epoll_wait() timeout" << endl;
            continue;
        }

        // 开始遍历epoll_event数组获得就绪的文件描述符信息
        for (int ii = 0; ii < infds; ii++)
        {
            Channel *ch = static_cast<Channel *>(evs[ii].data.ptr);

            if (ch->islisten() == true)
            {
                // 遇到监听套接字的情况
                InetAddress clientaddr;
                //clientsock只能new出来，放到堆中以避免被调用析构函数关闭fd
                Socket *clientsock = new Socket(servsock.accept(clientaddr));
                /////////////////////
                Channel *clientchannel = new Channel(clientsock->fd());
                ev.data.ptr = clientchannel;
                ev.events = EPOLLIN | EPOLLET; // 监视客户端的读事件，并采用边缘触发的方式
                epoll_ctl(epollfd, EPOLL_CTL_ADD, clientsock->fd(), &ev);
            }
            else
            {
                // 遇到客户连接的文件描述符就绪

                // 当对端关闭连接时，EPOLLRDHUP 事件会被触发。此时，应用程序可以相应地关闭本地的连接或采取其他必要的操作
                if (evs[ii].events & EPOLLRDHUP)
                {
                    cout << "client(eventfd=" << ch->fd() << ") disconnected." << endl;
                    close(ch->fd());
                }
                else if (evs[ii].events & (EPOLLIN | EPOLLPRI))
                {
                    string buffer;
                    buffer.resize(1024);
                    while (true)
                    {
                        ssize_t nread = read(ch->fd(), &buffer[0], buffer.size() - 1);
                        if (nread > 0)
                        {
                            buffer[nread] = '\0'; // Null-terminate the string
                            cout << "recv(eventfd=" << ch->fd() << "): " << buffer << endl;
                            send(ch->fd(), buffer.c_str(), buffer.size(), 0);
                        }
                        else if (nread == -1 && errno == EINTR)
                        {
                            // 读取数据的时候被信号中断，继续读取。
                            continue;
                        }
                        else if (nread == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK)))
                        {
                            // 全部的数据已读取完毕
                            break;
                        }
                        else if (nread == 0)
                        {
                            // 客户端连接已断开
                            cout << "client(eventfd=" << ch->fd() << ") disconnected." << endl;
                            close(ch->fd());
                            break;
                        }
                    }
                }
                else if (evs[ii].events & EPOLLOUT)
                {
                    // 暂时没有代码
                }
                else
                {
                    cout << "client(eventfd=" << ch->fd() << ") error." << endl;
                    close(ch->fd());
                }
            }
        }
    }
}
