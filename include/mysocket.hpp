#include "inetaddress.hpp"
class Socket
{
private:
    const int fd_;

public:
    /*
    
    */
    Socket(int fd);
    /*
    @brief 在析构函数中要关闭打开的文件描述符
    */
    ~Socket();
    /*
    返回文件描述符
    */
    int fd() const;
    /*
    设置SO_REUSEADDR选项
    */
    void setReuseAddr(bool on);
    void setReusePort(bool on);
    void setTcpNoDelay(bool on);
    void setKeepAlive(bool on);
    void bind(const InetAddress& servaddr);
    void listen(int backlog = 128);
    /*
    @param clientaddr 将接收的连接的对端地址信息保存在参数中
    */
    int accept(InetAddress &clientaddr);
};
int createNonBlockingSocket();