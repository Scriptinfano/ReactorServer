#include "tcpserver.hpp"
#include "mysocket.hpp"
#include "channel.hpp"
#include "connection.hpp"
#include "log.hpp"
#include <sys/syscall.h>
#include <unistd.h>
TCPServer::TCPServer(const std::string &ip, const in_port_t port, int threadnum)
{
    threadnum_ = threadnum;
    loop_ = std::make_shared<EventLoop>();
    loop_->setEpollTimeoutCallBack(std::bind(&TCPServer::epollTimeoutCallBack, this, std::placeholders::_1));
    //TODO 这里Accepter的构造函数的第一个参数可能需要修改，而且这里将unique_ptr传入其他函数的做法不太妥当，可能需要改为shared_ptr
    accepter_ =std::make_unique<Accepter>(loop_, ip, port);
    accepter_->setAcceptCallBack(std::bind(&TCPServer::acceptCallBack, this, std::placeholders::_1, std::placeholders::_2));
    threadpool_ = std::make_unique<ThreadPool>(threadnum_, "io_thread");
    for (int i = 0; i < threadnum_; i++)
    {
        subloops_.emplace_back(std::make_unique<EventLoop>());
        subloops_[i]->setEpollTimeoutCallBack(std::bind(&TCPServer::epollTimeoutCallBack, this, std::placeholders::_1));
        threadpool_->addTask(std::bind(&EventLoop::run, subloops_[i].get()));//对智能指针调用get()接口可以转化为普通指针
    }
}
TCPServer::~TCPServer()
{
}
void TCPServer::start()
{
    loop_->run();
}
void TCPServer::acceptCallBack(int fd, InetAddress clientaddr)
{
    // 在这里做手脚，将线程池里线程中的loop_想办法传入下面的Connection，就能实现主线程处理传入链接，线程池里的线程处理与客户交流的连接
    SharedConnectionPointer conn=std::make_shared<Connection>(subloops_[fd % threadnum_], fd, &clientaddr);
    // 这里将clientaddr的值传入Connection内部
    conn->setCloseCallBack(std::bind(&TCPServer::closeCallBack, this, std::placeholders::_1));
    conn->setErrorCallBack(std::bind(&TCPServer::errorCallBack, this, std::placeholders::_1));
    conn->setProcessCallBack(std::bind(&TCPServer::processCallBack, this, std::placeholders::_1, std::placeholders::_2));
    conn->setSendCompleteCallBack(std::bind(&TCPServer::sendCompleteCallBack, this, std::placeholders::_1));
    logger.logMessage(NORMAL, __FILE__, __LINE__, "accept client(fd=%d,ip=%s,port=%d) ok", fd, clientaddr.ip().c_str(), clientaddr.port());
    connectionMapper_[conn->getFd()] = conn; // 将Connection的地址放入映射器管理起来
    if (acceptCallBack_)
        acceptCallBack_(conn);
}
void TCPServer::closeCallBack(SharedConnectionPointer conn)
{
    if (closeCallBack_)
        closeCallBack_(conn);
    int fd = conn->getFd();
    logger.logMessage(WARNING, __FILE__, __LINE__, "client socket(%d) closed the connection", fd);
    connectionMapper_.erase(conn->getFd());
}
void TCPServer::errorCallBack(SharedConnectionPointer conn)
{
    if (errorCallBack_)
        errorCallBack_(conn);
    int fd = conn->getFd();
    logger.logMessage(WARNING, __FILE__, __LINE__, "client socket(%d) occur unknown error", fd);
    connectionMapper_.erase(conn->getFd());
}
void TCPServer::processCallBack(SharedConnectionPointer conn, std::string &message)
{
    // 其实在这里也不应该直接处理业务，而是应该再创建一个业务处理类，让业务处理类区处理业务数据，这样的话，结构更加清晰
    // 根据业务需求也可以有其他代码
    if (processCallBack_)
        processCallBack_(conn, message);
}
void TCPServer::sendCompleteCallBack(SharedConnectionPointer conn)
{
    logger.logMessage(NORMAL, __FILE__, __LINE__, "send complete");
    // 根据业务需求也可以有其他代码
    if (sendCompleteCallBack_)
        sendCompleteCallBack_(conn);
}
void TCPServer::epollTimeoutCallBack(EventLoop *loop)
{
    logger.logMessage(NORMAL, __FILE__, __LINE__, "thread %d start to handle epolltimeout situation", syscall(SYS_gettid));
    if (epollTimeoutCallBack_)
        epollTimeoutCallBack_(loop);
}

void TCPServer::setAcceptCallBack(std::function<void(SharedConnectionPointer)> acceptCallBack)
{
    acceptCallBack_ = acceptCallBack;
}
void TCPServer::setCloseCallBack(std::function<void(SharedConnectionPointer)> closeCallBack)
{
    closeCallBack_ = closeCallBack;
}
void TCPServer::setErrorCallBack(std::function<void(SharedConnectionPointer)> errorCallBack)
{
    errorCallBack_ = errorCallBack;
}
void TCPServer::setProcessCallBack(std::function<void(SharedConnectionPointer, std::string &)> processCallBack)
{
    processCallBack_ = processCallBack;
}
void TCPServer::setSendCompleteCallBack(std::function<void(SharedConnectionPointer)> sendCompleteCallBack)
{
    sendCompleteCallBack_ = sendCompleteCallBack;
}
void TCPServer::setEpollTimeoutCallBack(std::function<void(EventLoop *)> epollTimeoutCallBack)
{
    epollTimeoutCallBack_ = epollTimeoutCallBack;
}