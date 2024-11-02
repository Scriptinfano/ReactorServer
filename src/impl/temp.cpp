#include "temp.hpp"
#include <string>
void init_test(int no, char *str, test *const ptr)
{
    ptr->no = no;
    strncpy(ptr->name, str, sizeof(ptr->name));
}