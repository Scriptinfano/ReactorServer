#pragma once
#include "tcpserver.hpp"
// 这是一个定义业务类的头文件，想要修改整个服务器的业务处理逻辑需要在这里创建业务类，然后在相关回调函数中调用业务类即可
class EchoServer
{
private:
    TCPServer tcpserver_;

public:
    EchoServer(const std::string &ip, in_port_t port);
    ~EchoServer();
    /*
    在Accepter初步accept之后，接下来的处理工作
    */
    void start();

    void acceptCallBack(Connection *conn);

    void closeCallBack(Connection *conn);

    void errorCallBack(Connection *conn);
    /*
    该函数代表整个服务器对于客户端发来的数据的一个处理
    @param conn 处理哪一个连接发来的数据
    @param message 原始数据
    */
    void processCallBack(Connection *conn, std::string message);

    /*
    当Connection将数据都加到输入缓冲区中之后，回调这个函数，相当于通知TCPServer
    */
    void sendCompleteCallBack(Connection *conn);

    /*
    在EventLoop中如果发生超时的情况，需要回调这个函数
    */
    void epollTimeoutCallBack(EventLoop *loop);
};
