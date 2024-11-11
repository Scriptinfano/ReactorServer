#include "buffer.hpp"
#include<log.hpp>
Buffer::Buffer()
{
}
Buffer::~Buffer()
{
}
void Buffer::append(const char *data, size_t size)
{
    buf_.append(data, size); // append(const char* s, size_t n)：将指针 s 指向的字符数组中的前 n 个字符附加到当前字符串末尾

}
size_t Buffer::getSize()
{
    return buf_.size();
}
const char *Buffer::getData()
{
    return buf_.data();
}
void Buffer::clear()
{
    return buf_.clear();
}

std::string Buffer::getString(){
    return buf_;
}

void Buffer::erase(size_t pos, size_t n){
    buf_.erase(pos, n);
}