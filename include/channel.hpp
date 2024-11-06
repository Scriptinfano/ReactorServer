#pragma once
#include "epoll.hpp"
#include "mysocket.hpp"
#include "eventloop.hpp"
#include <functional>
class Epoll; // 如果两个头文件互相包含，互相需要对方的数据结构，那么需要在两个文件做对方的前向声明，而且要在头文件的首部加入#pragme once
class EventLoop;

/*
对连接抽象化的Channel类
*/
class Channel
{
private:
    int fd_ = -1;          // Channel和fd是一对一的关系
    
    bool inepoll_ = false; // 标记Channel是否已添加到epoll的红黑树上，如果没有添加，则调用epoll_ct的时候添加，否则用EPOLL_CTL_MOD
    uint32_t events_ = 0;  // fd_需要监视的事件，listenfd和clientfd需要监视EPOLLIN，clientfd可能还需要监视EPOLLOUT
    uint32_t revents_ = 0; // fd_已经发生的事件
    Socket *sock_ = nullptr;
    std::function<void()> readcallback_; // 遇到可读事件的回调函数
    EventLoop * loop_=nullptr;
public:
    /*
    @param loop 该Channel属于哪一个事件循环
    @param fd 该Channel需要关联的文件描述符是哪一个
    @param sock 该Channel需要关联的文件描述符对应的封装套接字是哪一个
    */
    Channel(EventLoop *loop, int fd, Socket *sock);
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
    void monitorReadEvent();
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