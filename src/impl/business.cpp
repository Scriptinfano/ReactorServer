#include "business.hpp"
EchoServer::EchoServer(const std::string &ip, in_port_t port)
{
}

EchoServer::~EchoServer()
{
}
void EchoServer::acceptCallBack(int fd, InetAddress clientaddr){

}

void EchoServer::closeConnectionCallBack(Connection *conn){

}

void EchoServer::errorConnectionCallBack(Connection *conn){

}

void EchoServer::processCallBack(Connection *conn, std::string message){

}

void EchoServer::sendComplete(Connection *conn){

}

void EchoServer::epolltimeout(EventLoop *loop){
    
}