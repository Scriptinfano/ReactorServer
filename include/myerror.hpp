#include<iostream>
#include<cstring>
#include<cerrno>
using namespace std;
void print_error(string errmsg)
{
    cerr << errmsg << strerror(errno) << endl;
}