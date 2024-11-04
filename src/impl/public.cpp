#include "public.hpp"
#include <string.h>

bool hasNewlineAtEnd(const char *buffer)
{
    // 计算字符串的长度
    size_t length = strlen(buffer);
    // 检查长度是否大于0并且最后一个字符是否是换行符
    return length > 0 && buffer[length - 1] == '\n';
}