#include <sys/fcntl.h>
#include <iostream>
#include <sys/socket.h>
#include <netinet/tcp.h>

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

    // 创建监听套接字
    int listenfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (listenfd < 0)
    {
        cerr << "socket() failed: " << std::strerror(errno) << endl;
        return -1;
    }
    /////////////

    // 为监听套接字设置套接字选项选项
    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, static_cast<socklen_t>(sizeof opt));
    setsockopt(listenfd, SOL_SOCKET, TCP_NODELAY, &opt, static_cast<socklen_t>(sizeof opt));
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEPORT, &opt, static_cast<socklen_t>(sizeof opt));
    setsockopt(listenfd, SOL_SOCKET, SO_KEEPALIVE, &opt, static_cast<socklen_t>(sizeof opt));
    //////////////////////////

    // 从参数中读取ip地址，准备在指定的ip地址和端口上调用bind函数
    InetAddress servaddr(argv[1], atoi(argv[2]));
    /////////////////////////////////////////////////////

    // 调用bind函数和listen函数正式开始监听
    if (bind(listenfd, servaddr.addr(), sizeof(sockaddr)) < 0)
    {
        cerr << "bind() failed: " << std::strerror(errno) << endl;
        close(listenfd);
        return -1;
    }
    if (listen(listenfd, 128) < 0)
    {
        cerr << "listen() failed: " << std::strerror(errno) << endl;
        close(listenfd);
        return -1;
    }
    //////////////////////////////////

    // 创建epoll实例，得到它的文件描述符
    int epollfd = epoll_create1(EPOLL_CLOEXEC);
    Channel *servchannel = new Channel(listenfd, true);
    ///////////////////////////////

    // 在epoll模型中添加对epfd的可读事件的监听
    struct epoll_event ev;
    ev.data.ptr = servchannel;
    ev.events = EPOLLIN;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev);
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
                struct sockaddr_in peeraddr;
                socklen_t len = sizeof(peeraddr);
                // accept4函数支持给新接受的套接字设置一个选项
                int clientfd = accept4(listenfd, (struct sockaddr *)&peeraddr, &len, SOCK_NONBLOCK);
                InetAddress clientaddr(peeraddr);
                cout << "accept client(fd=" << clientfd << ", ip=" << clientaddr.ip()<< ", port=" << clientaddr.port() << " ok." << endl;

                Channel *clientchannel = new Channel(clientfd);
                ev.data.ptr = clientchannel;
                ev.events = EPOLLIN | EPOLLET; // 监视客户端的读事件，并采用边缘触发的方式
                epoll_ctl(epollfd, EPOLL_CTL_ADD, clientfd, &ev);
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
                            continue;
                        }
                        else if (nread == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK)))
                        {
                            break;
                        }
                        else if (nread == 0)
                        {
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
