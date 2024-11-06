#include "tcpserver.hpp"
#include "mysocket.hpp"
#include "channel.hpp"
TCPServer::TCPServer(const std::string &ip, const in_port_t port){
    
    Socket* servsock=new Socket(createNonBlockingSocket());//创建一个非阻塞的封装之后的用来监听指定地址和端口的套接字
    InetAddress servaddr(ip,port);//创建封装之后的地址对象，构造函数传入ip地址和端口号
    //设置几个套接字选项
    servsock->setKeepAlive(true);
    servsock->setReuseAddr(true);
    servsock->setReusePort(true);
    servsock->setTcpNoDelay(true);
    servsock->bind(servaddr);//绑定地址
    servsock->listen();//开始监听
    /////////////

    loop_ = new EventLoop();//创建事件循环对象，一个事件循环对象有多个Channel代表抽象的连接
    Channel *server_channel = new Channel(loop_, servsock->fd(), servsock);//这是管理服务端监听套接字的抽象连接
    server_channel->setReadCallBack(std::bind(&Channel::handleNewConnection, server_channel));//设置该Channel在读事件就绪之后应该调用的回调函数
    server_channel->monitorReadEvent(); //将该Channel设为应该监视可读事件，也就是客户端来连接的事件
}
TCPServer::~TCPServer(){

}
void TCPServer::start(){
    loop_->run();
}