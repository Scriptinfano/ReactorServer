#include "mysocket.hpp"
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <iostream>

#include "log.hpp"
int createNonBlockingSocket()
{
    int listenfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (listenfd < 0)
    {
        logger.logMessage(FATAL, __FILE__, __LINE__, "socket() failed");
        exit(-1);
    }
    return listenfd;
}
Socket::Socket(int fd) : fd_(fd)
{
}

Socket::~Socket()
{
    close(fd_);
}

int Socket::fd() const
{
    return fd_;
}

void Socket::setReuseAddr(bool on)
{
    int optval = on ? 1 : 0;
    setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
}
void Socket::setReusePort(bool on)
{
    int optval = on ? 1 : 0;
    setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
}
void Socket::setTcpNoDelay(bool on)
{
    int optval = on ? 1 : 0;
    setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
}
void Socket::setKeepAlive(bool on)
{
    int optval = on ? 1 : 0;
    setsockopt(fd_, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
}
void Socket::bind(const InetAddress &servaddr)
{
    if (::bind(fd_, servaddr.addr(), sizeof(sockaddr)) < 0)
    {
        logger.logMessage(FATAL, __FILE__, __LINE__, logger.createErrorMessage("bind() failed").c_str());
        exit(-1);
    }
}
void Socket::listen(int backlog)
{
    if (::listen(fd_, backlog) < 0)
    {
        logger.logMessage(FATAL, __FILE__, __LINE__, logger.createErrorMessage("listen() failed").c_str());
        exit(-1);
    }
}
int Socket::accept(InetAddress &clientaddr)
{
    struct sockaddr_in peeraddr;
    socklen_t len = sizeof(peeraddr);
    // accept4函数支持给新接受的套接字设置一个选项
    int clientfd = accept4(fd_, (struct sockaddr *)&peeraddr, &len, SOCK_NONBLOCK);
    clientaddr.setaddr(peeraddr);
    logger.logMessage(DEBUG, __FILE__, __LINE__, "accept client(fd=%d,ip=%s,port=%d) ok", clientfd, clientaddr.ip(), clientaddr.port());
    return clientfd;
}
