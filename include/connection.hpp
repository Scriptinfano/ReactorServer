#pragma once
#include "eventloop.hpp"
#include "mysocket.hpp"
#include "channel.hpp"
class Connection
{
private:
    EventLoop *loop_;
    Socket *clientsock_;     // 客户端连接的套接字，构造函数中new出来
    Channel *clientchannel_; // 客户级别的Channel

public:
    /*
    @param loop 这个Connection属于哪一个事件循环
    @param fd 客户端连接的文件描述符
    */
    Connection(EventLoop *loop, int fd);
    ~Connection();
    int getFd() const;
    std::string getIP() const;
    in_port_t getPort() const;
};