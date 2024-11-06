
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
#include "eventloop.hpp"
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

    EventLoop loop;   
    Channel *server_channel = new Channel(loop.getEpoll(), servsock.fd(), &servsock);
    server_channel->setReadCallBack(std::bind(&Channel::handleNewConnection, server_channel));
    server_channel->monitorReadEvent(); // 监视可读事件
    loop.run();
}
