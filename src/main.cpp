#include <iostream>
#include <string.h>
using namespace std;

int main(int argc, char **argv)
{
    char tmp[1024] = {0};
    const char *str = "123";
    memcpy(tmp, str, strlen(str));

    cout << tmp << endl;
}
