#include <sys/fcntl.h>
#include <iostream>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/epoll.h>
#include <cstring>
#include <string>
#include "errno.h"

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

void set_non_blocking(int fd)
{
    fcntl(fd, F_SETFD, fcntl(fd, F_GETFL) | O_NONBLOCK);
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        cout << "usage: epollserver ip port" << endl;
        cout << "example, epollserver 192.168.150.128 8080" << endl;
        return -1;
    }

    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0)
    {
        cerr << "socket() failed: " << std::strerror(errno) << endl;
        return -1;
    }

    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, static_cast<socklen_t>(sizeof opt));
    setsockopt(listenfd, SOL_SOCKET, TCP_NODELAY, &opt, static_cast<socklen_t>(sizeof opt));
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEPORT, &opt, static_cast<socklen_t>(sizeof opt));
    setsockopt(listenfd, SOL_SOCKET, SO_KEEPALIVE, &opt, static_cast<socklen_t>(sizeof opt));

    set_non_blocking(listenfd);

    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    int ret = inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

    if (ret == 0)
    {
        cout << "不是有效的地址格式，重新调用命令输入" << endl;
        return -1;
    }
    else if (ret == -1)
    {
        cerr << "inet_pton() failed: " << std::strerror(errno) << endl;
        return -1;
    }

    servaddr.sin_port = htons(atoi(argv[2]));
    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
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

    int epollfd = epoll_create1(EPOLL_CLOEXEC);
    Channel *servchannel = new Channel(listenfd, true);

    struct epoll_event ev;
    ev.data.ptr = servchannel;
    ev.events = EPOLLIN;

    epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev);

    struct epoll_event evs[10];

    while (true)
    {
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

        for (int ii = 0; ii < infds; ii++)
        {
            Channel *ch = static_cast<Channel *>(evs[ii].data.ptr);

            if (ch->islisten() == true)
            {
                struct sockaddr_in clientaddr;
                socklen_t len = sizeof(clientaddr);
                int clientfd = accept(listenfd, (struct sockaddr *)&clientaddr, &len);
                set_non_blocking(clientfd);

                cout << "accept client(fd=" << clientfd << ", ip=" << inet_ntoa(clientaddr.sin_addr)
                     << ", port=" << ntohs(clientaddr.sin_port) << ") ok." << endl;

                Channel *clientchannel = new Channel(clientfd);
                ev.data.ptr = clientchannel;
                ev.events = EPOLLIN | EPOLLET;
                epoll_ctl(epollfd, EPOLL_CTL_ADD, clientfd, &ev);
            }
            else
            {
                if (evs[ii].events & EPOLLRDHUP)
                {
                    cout << "1client(eventfd=" << ch->fd() << ") disconnected." << endl;
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
                            cout << "2client(eventfd=" << ch->fd() << ") disconnected." << endl;
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
                    cout << "3client(eventfd=" << ch->fd() << ") error." << endl;
                    close(ch->fd());
                }
            }
        }
    }
}
