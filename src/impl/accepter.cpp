#include "accepter.hpp"
Accepter::Accepter(EventLoop *loop, const std::string &ip, const in_port_t port) : loop_(loop)
{
    servsock_ = new Socket(createNonBlockingSocket()); // 创建一个非阻塞的封装之后的用来监听指定地址和端口的套接字
    InetAddress servaddr(ip, port);                    // 创建封装之后的地址对象，构造函数传入ip地址和端口号
    // 设置几个套接字选项
    servsock_->setKeepAlive(true);
    servsock_->setReuseAddr(true);
    servsock_->setReusePort(true);
    servsock_->setTcpNoDelay(true);
    servsock_->bind(servaddr); // 绑定地址
    servsock_->listen();       // 开始监听
    /////////////

    acceptchannel_ = new Channel(loop_, servsock_->fd(), servsock_);                           // 这是管理服务端监听套接字的抽象连接
    acceptchannel_->setReadCallBack(std::bind(&Channel::handleNewConnection, acceptchannel_)); // 设置该Channel在读事件就绪之后应该调用的回调函数
    acceptchannel_->monitorReadEvent();                                                        // 将该Channel设为应该监视可读事件，也就是客户端来连接的事件
}
Accepter::~Accepter()
{
    delete servsock_;
    delete acceptchannel_;
    //loop_是外部传入的，不要释放
}