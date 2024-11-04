#include "channel.hpp"

Channel::Channel(Epoll *ep, int fd):ep_(ep),fd_(fd){

}
Channel::~Channel(){
    //注意在析构函数中不要销毁ep_，也不能关闭fd_，因为他们不属于Channel类，Channel类只是需要他们，使用他们而已
    
}

/*
@brief 取得fd成员
*/
int Channel::fd(){

}
/*
@brief 设置采用边缘触发模式
*/
void Channel::set_ET(){

}
/*
@brief 让epoll_wait()监视fd_的可读事件
*/
void Channel::enable_read(){

}
/*
@brief 将inepoll的值设为true
*/
void Channel::set_inepoll(){

}
/*
@brief 设置revents成员的值为true
*/
void Channel::set_revents(uint32_t revs){

}
/*
@brief 返回inepoll_成员的值
*/
bool Channel::inepoll(){

}
/*
@brief 返回events成员
*/
uint32_t Channel::events(){

}
/*
@brief 返回revents成员
*/
uint16_t Channel::revents(){
    
}