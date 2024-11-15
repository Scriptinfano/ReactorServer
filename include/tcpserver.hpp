#pragma once
#include "eventloop.hpp"
#include "accepter.hpp"
#include "connection.hpp"
#include <map>
#include <vector>
#include "threadpool.hpp"
/*
顶层封装，整个业务服务器的接口提供类，如果要增加其他业务，需要在业务类中包含该类对象，然后调用set...CallBack()函数设置回调函数到自己的类成员函数
*/
class TCPServer
{
private:
    EventLoop *loop_;                              // 一个TCPServer可以有多个事件循环，现在是单线程，暂时只用一个事件循环
    std::vector<EventLoop *> subloops_;            // 从事件循环集合，增加这个成员的目的是要构建多线程下的Reactor模型
    Accepter *accepter_;                           // 一个TCPServer只有一个Accepter类
    ThreadPool *threadpool_;                       // 线程池
    std::map<int, Connection *> connectionMapper_; // 一个TCPServer可以有多个Connection，所以用一个Map容器将Connection管理起来
    int threadnum_;                                // 线程池中应该有多少个线程
    std::function<void(Connection *)> acceptCallBack_;
    std::function<void(Connection *)> closeCallBack_;
    std::function<void(Connection *)> errorCallBack_;
    std::function<void(Connection *, std::string &)> processCallBack_;
    std::function<void(Connection *)> sendCompleteCallBack_;
    std::function<void(EventLoop *)> epollTimeoutCallBack_;

public:
    TCPServer(const std::string &ip, const in_port_t port, int threadnum);
    ~TCPServer();
    /*
    开始运行事件循环，也就是开始运行这个服务器
    */
    void start();
    /*

    */
    void acceptCallBack(int fd, InetAddress clientaddr);
    /*

    */
    void closeCallBack(Connection *conn);
    /*

    */
    void errorCallBack(Connection *conn);
    /*
    该函数代表整个服务器对于客户端发来的数据的一个处理
    @param conn 处理哪一个连接发来的数据
    @param message 原始数据
    */
    void processCallBack(Connection *conn, std::string &message);

    /*
    当Connection将数据都加到输入缓冲区中之后，回调这个函数，相当于通知TCPServer
    */
    void sendCompleteCallBack(Connection *conn);

    /*
    在EventLoop中如果发生超时的情况，需要回调这个函数
    */
    void epollTimeoutCallBack(EventLoop *loop);

    void setAcceptCallBack(std::function<void(Connection *)> acceptCallBack);
    void setCloseCallBack(std::function<void(Connection *)> closeCallBack);
    void setErrorCallBack(std::function<void(Connection *)> errorCallBack);
    void setProcessCallBack(std::function<void(Connection *, std::string &)> processCallBack);
    void setSendCompleteCallBack(std::function<void(Connection *)> sendCompleteCallBack);
    void setEpollTimeoutCallBack(std::function<void(EventLoop *)> epollTimeoutCallBack);
};