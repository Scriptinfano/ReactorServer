
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <cstring>
#include <string>
#include "epoll.hpp"
#include "inetaddress.hpp"
#include "log.hpp"

#include "mysocket.hpp"

using namespace std;

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
    // servsock.listen();
    /////////////

    Epoll epoll; // 这里会调用默认的构造函数
    epoll.addfd(servsock.fd(), EPOLLIN);
    std::vector<epoll_event> evs;

    while (true)
    {
        evs = epoll.loop(); // 开始等待就绪事件发生

        // 开始遍历epoll_event数组获得就绪的文件描述符信息
        for (auto &ev : evs)
        {
            if (ev.data.fd == servsock.fd())
            {
                if (ev.events == EPOLLHUP)
                {
                    logger.logMessage(FATAL, __FILE__, __LINE__, "可能未调用listen函数使得监听套接字变为被动监听状态");
                    exit(-1);
                }
                // 遇到监听套接字的情况
                InetAddress clientaddr;
                // clientsock只能new出来，放到堆中以避免被调用析构函数关闭fd
                Socket *clientsock = new Socket(servsock.accept(clientaddr));
                epoll.addfd(clientsock->fd(), EPOLLIN | EPOLLET); // 客户端连上的fd采用边缘触发法
            }
            else
            {
                // 遇到客户连接的文件描述符就绪

                // 当对端关闭连接时，EPOLLRDHUP 事件会被触发。此时，应用程序可以相应地关闭本地的连接或采取其他必要的操作
                if (ev.events & EPOLLRDHUP)
                {
                    cout << "client(eventfd=" << ev.data.fd << ") disconnected." << endl;
                    close(ev.data.fd);
                }
                else if (ev.events & (EPOLLIN | EPOLLPRI))
                {
                    string buffer;
                    buffer.resize(1024);
                    while (true)
                    {
                        ssize_t nread = read(ev.data.fd, &buffer[0], buffer.size() - 1);
                        if (nread > 0)
                        {
                            buffer[nread] = '\0'; // Null-terminate the string
                            cout << "recv(eventfd=" << ev.data.fd << "): " << buffer << endl;
                            send(ev.data.fd, buffer.c_str(), buffer.size(), 0);
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
                            cout << "client(eventfd=" << ev.data.fd << ") disconnected." << endl;
                            close(ev.data.fd);
                            break;
                        }
                    }
                }
                else if (ev.events & EPOLLOUT)
                {
                    // 暂时没有代码
                }
                else
                {
                    cout << "client(eventfd=" << ev.data.fd << ") error." << endl;
                    close(ev.data.fd);
                }
            }
        }
    }
}
