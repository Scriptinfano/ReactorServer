#include "tcpserver.hpp"
#include "mysocket.hpp"
#include "channel.hpp"
TCPServer::TCPServer(const std::string &ip, const in_port_t port)
{
    loop_ = new EventLoop();
    accepter_ = new Accepter(loop_, ip, port);
}
TCPServer::~TCPServer()
{
    delete loop_;
    delete accepter_;
}
void TCPServer::start()
{
    loop_->run();
}