#include <iostream>
#include <cerrno>
#include "myerror.hpp"
void print_error(std::string errmsg)
{
    std::cerr << errmsg << strerror(errno) << std::endl;
}