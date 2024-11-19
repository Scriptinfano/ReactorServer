#include "business.hpp"
#include "log.hpp"
#include <sys/syscall.h>
#include <unistd.h>
EchoServer::EchoServer(const std::string &ip, in_port_t port, int threadnum, int workthreadnum)
{
    tcpserver_ = std::make_unique<TCPServer>(ip, port, threadnum);
    threadpool_ = std::make_unique<ThreadPool>(workthreadnum, "worker_thread");
    tcpserver_->setAcceptCallBack(std::bind(&EchoServer::acceptCallBack, this, std::placeholders::_1));
    tcpserver_->setCloseCallBack(std::bind(&EchoServer::closeCallBack, this, std::placeholders::_1));
    tcpserver_->setEpollTimeoutCallBack(std::bind(&EchoServer::epollTimeoutCallBack, this, std::placeholders::_1));
    tcpserver_->setErrorCallBack(std::bind(&EchoServer::errorCallBack, this, std::placeholders::_1));
    tcpserver_->setProcessCallBack(std::bind(&EchoServer::processCallBack, this, std::placeholders::_1, std::placeholders::_2));
    tcpserver_->setSendCompleteCallBack(std::bind(&EchoServer::sendCompleteCallBack, this, std::placeholders::_1));
}

EchoServer::~EchoServer()
{
}

void EchoServer::start()
{
    tcpserver_->start();
}
void EchoServer::acceptCallBack(SharedConnectionPointer conn)
{
}

void EchoServer::closeCallBack(SharedConnectionPointer conn)
{
}

void EchoServer::errorCallBack(SharedConnectionPointer conn)
{
}

void EchoServer::processCallBack(SharedConnectionPointer conn, std::string message)
{
    //这个判断是为了适应如果没有工作线程，那就必须由从线程自己来负责处理数据并发送数据的工作
    if(threadpool_->getThreadSize()==0){
        logger.logMessage(DEBUG, __FILE__, __LINE__, "there is no worker thread, the sub thread will be responsible for handling and sending data itself.");
        wokerThreadBehavior(conn, message);
    }else{
        logger.logMessage(DEBUG, __FILE__, __LINE__, "EchoServer::processCallBack() called, sub thread id=%d", syscall(SYS_gettid));
        threadpool_->addTask(std::bind(&EchoServer::wokerThreadBehavior, this, conn, message));
        logger.logMessage(DEBUG, __FILE__, __LINE__, "EchoServer threadpool addTask to task queue, sub thread id=%d", syscall(SYS_gettid));
    }

}

void EchoServer::sendCompleteCallBack(SharedConnectionPointer conn)
{
}

void EchoServer::epollTimeoutCallBack(EventLoop *loop)
{
}
void EchoServer::wokerThreadBehavior(SharedConnectionPointer conn, std::string message)
{
    logger.logMessage(DEBUG, __FILE__, __LINE__, "EchoServer::workerThreadBehavior() called, worker thread id=%d", syscall(SYS_gettid));
    message = "reply:" + message;
    conn->send(message.c_str(), message.size()); // Connection内部的send函数会把数据放到内部的outputBuffer_中，等下一次可写事件发生时会被发送出去
}