
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <cstring>
#include <string>
#include "epoll.hpp"
#include "inetaddress.hpp"
#include "log.hpp"
#include "mysocket.hpp"
#include "channel.hpp"
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
    servsock.listen();
    /////////////

    Epoll ep; // 这里会调用默认的构造函数
    Channel *server_channel = new Channel(&ep, servsock.fd());
    server_channel->start_monitor_read(); // 监视可读事件

    while (true)
    {
        std::vector<Channel *> chans = ep.loop(); // 开始等待就绪事件发生

        // 开始遍历epoll_event数组获得就绪的文件描述符信息
        for (auto &ch : chans)
        {
            // 判断套接字的类型，是监听套接字来新连接了，还是已有的客户连接就绪了
            if (ch->fd() == servsock.fd()) // 还可以直接判断ch是否和server_channel的值相同
            {
                if (ch->revents() & EPOLLHUP)
                {
                    logger.logMessage(FATAL, __FILE__, __LINE__, "可能未调用listen函数使得监听套接字变为被动监听状态");
                    exit(-1);
                }
                // 遇到监听套接字的情况
                InetAddress clientaddr;
                // clientsock只能new出来，放到堆中以避免被调用析构函数关闭fd
                Socket *clientsock = new Socket(servsock.accept(clientaddr));
                clientsock->setNonBlocking(true);//在边缘触发模式下的epollsevrer必须将clintsock设为非阻塞模式
                Channel *clien_channel = new Channel(&ep, clientsock->fd());
                clien_channel->set_ET();//一定要在start_monitor_read()之前调用设置边缘触发的方法
                clien_channel->start_monitor_read();//加入epoll的监视，开始监视这个channel的可读事件
            }
            else
            {
                // 遇到客户连接的文件描述符就绪
                if (ch->revents() & EPOLLRDHUP)
                {
                    logger.logMessage(WARNING, __FILE__, __LINE__, "client socket(%d) closed the connection", ch->fd());
                    close(ch->fd());
                }
                else if (ch->revents() & (EPOLLIN | EPOLLPRI))
                {
                    char buffer[1024] = {0};
                    while (true)
                    {
                        ssize_t nread = read(ch->fd(), buffer,sizeof(buffer));
                        if (nread > 0)
                        {
                            cout << "recv(eventfd=" << ch->fd() << "): " << buffer << endl;
                            //读取完成之后原封不动的发送回去
                            send(ch->fd(), buffer, sizeof(buffer), 0);
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
                            logger.logMessage(WARNING, __FILE__, __LINE__, "client socket(%d) closed the connection", ch->fd());
                            close(ch->fd());
                            break;
                        }
                    }
                }
                else if (ch->revents() & EPOLLOUT)
                {
                    // 暂时没有代码
                }
                else
                {
                    logger.logMessage(WARNING, __FILE__, __LINE__, "client socket(%d) error", ch->fd());
                    close(ch->fd());
                }
            }
        }
    }
}
