#pragma once
#include "inetaddress.hpp"
class Socket
{
private:
    const int fd_;

public:
    /*

    */
    Socket(int fd);
    /*
    @brief 在析构函数中要关闭打开的文件描述符
    */
    ~Socket();
    /*
    返回文件描述符
    */
    int fd() const;
    /*
    设置SO_REUSEADDR选项
    */
    void setReuseAddr(bool on);
    /*
    设置SO_REUSEPORT选项
    */
    void setReusePort(bool on);
    /*
    设置TCP_NODELAY选项
    */
    void setTcpNoDelay(bool on);
    /*
    设置KEEPALIVE选项
    */
    void setKeepAlive(bool on);

    /*
    将当前套接字设为非阻塞
    */
    void setNonBlocking(bool on);

    /*
    @berif 将参数中所代表的地址绑定到当前套接字
    */
    void bind(const InetAddress &servaddr);
    /*
    将套接字转换为被动监听模式，参数指定连接队列的大小
    */
    void listen(int backlog = 128);
    /*
    @param clientaddr 将接收的连接的对端地址信息保存在参数中
    @return 返回已连接到的客户连接文件描述符
    */
    int accept(InetAddress &clientaddr);
};
int createNonBlockingSocket();