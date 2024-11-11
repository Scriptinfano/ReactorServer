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
    std::function<void(Connection *,std::string)> processCallBack_;//处理客户端发来的数据的回调函数
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
    /*
    设定当连接关闭的时候该执行的回调函数
    */
    void setCloseCallBack(std::function<void(Connection *)> closeCallBack);
    /*
    设定当错误发生时该执行的回调函数
    */
    void setErrorCallBack(std::function<void(Connection *)> errorCallBack);
    /*
    设定该如何处理客户端的数据
    */
    void setProcessCallBack(std::function<void(Connection *,std::string)> processCallBack);

    /*
    此函数是真正调用read函数的
    */
    void handleNewMessage();
};