#include<string>
typedef struct test
{
    int no;
    std::string name;
} test;
void init_test(int no, std::string str, test *const ptr);
