#pragma once
#include "epoll.hpp"
#include "mysocket.hpp"
#include <functional>
class Epoll; // 如果两个头文件互相包含，互相需要对方的数据结构，那么需要在两个文件做对方的前向声明，而且要在头文件的首部加入#pragme once
class Channel
{
private:
    int fd_ = -1;          // Channel和fd是一对一的关系
    Epoll *ep_ = nullptr;  // Channel和Epoll是多对一的关系，一个Channel对应一个Epoll，一个Epoll可以有多个Channel
    bool inepoll_ = false; // 标记Channel是否已添加到epoll的红黑树上，如果没有添加，则调用epoll_ct的时候添加，否则用EPOLL_CTL_MOD
    uint32_t events_ = 0;  // fd_需要监视的事件，listenfd和clientfd需要监视EPOLLIN，clientfd可能还需要监视EPOLLOUT
    uint32_t revents_ = 0; // fd_已经发生的事件
    Socket *sock_ = nullptr;
    std::function<void()> readcallback_; // 遇到可读事件的回调函数

public:
    /*
    @param ep 这条Channel依赖哪一个epoll实例监视
    @param fd 可以是监听套接字也可以是客户端连接套接字的文件描述符
    @oaram sock 本项目封装的套接字对象指针，fd和sock关联
    @param islistenfd 是否是监听套接字关联的Channel
    */
    Channel(Epoll *ep, int fd, Socket *sock);
    ~Channel();

    /*
    @brief 取得fd成员
    */
    int fd();
    /*
    @brief 设置采用边缘触发模式
    */
    void setETMode();
    /*
    @brief 函数内部会调用已绑定epoll实例，将读就绪事件添加到epoll的红黑树中，如果已经有了，则会修改红黑树
    */
    void startMonitoringReadEvent();
    /*
    @brief 将inepoll的值设为true
    */
    void setInEpoll();
    /*
    @brief 设置内部revents，承载epoll_wait返回的事件
    */
    void setRevents(uint32_t revs);
    /*
    @brief 将inepoll的值设为true，表示添加到了树上
    */
    bool getInEpoll();
    /*
    @brief 返回events成员
    */
    uint32_t getEvents();
    /*
    @brief 返回revents成员
    */
    uint16_t getRevents();
    /*
    @brief 事件处理函数，epoll_wait()返回的时候执行它
    */
    void handleEvent();

    /*
    处理新客户端的连接请求
    */
    void handleNewConnection();

    /*
    处理对端发送过来的消息
    */
    void handleNewMessage();
    /*
    设置读事件的回调函数
    */
    void setReadCallBack(std::function<void()> func);
};