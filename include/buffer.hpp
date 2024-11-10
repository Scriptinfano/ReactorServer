#pragma once
#include<string>
class Buffer{
private:
    std::string buf_;//用于存放数据

public:
    Buffer();
    ~Buffer();
    /*
    将size大小的数据追加到buffer中
    */
    void append(const char *data, size_t size);
    /*
    获取缓冲区目前存放的数据的大小
    */
    size_t getSize();
    /*
    获取缓冲区的数据的首地址
    */
    const char *getData();
    /*
    清空缓冲区
    */
    void clear();
};