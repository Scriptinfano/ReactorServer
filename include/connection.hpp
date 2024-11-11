#pragma once
#include "eventloop.hpp"
#include "mysocket.hpp"
#include "channel.hpp"
#include "buffer.hpp"
class Connection
{
private:
    EventLoop *loop_;
    Socket *clientsock_;     // 客户端连接的套接字，构造函数中new出来
    Channel *clientchannel_; // 客户级别的Channel
    std::function<void(Connection *)> closeCallBack_;
    std::function<void(Connection *)> errorCallBack_;
    Buffer inputBuffer_;  // 接收缓冲区
    Buffer outputBuffer_; // 发送缓冲区

public:
    /*
    @param loop 这个Connection属于哪一个事件循环
    @param fd 客户端连接的文件描述符
    */
    Connection(EventLoop *loop, int fd, InetAddress *clientaddr);
    ~Connection();
    int getFd() const;
    std::string getIP() const;
    in_port_t getPort() const;
    /*
    TCP连接关闭断开之后的回调函数，供Channel回调
    */
    void closeCallBack();
    /*
    TCP连接错误的回调函数，供Channel回调
    */
    void errorCallBack();

    void setCloseCallBack(std::function<void(Connection *)> closeCallBack);

    void setErrorCallBack(std::function<void(Connection *)> errorCallBack);
    void handleNewMessage();
};