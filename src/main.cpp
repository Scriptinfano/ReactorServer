
#include <iostream>
#include <unistd.h>
#include <sys/shm.h>
using namespace std;
typedef struct test
{
    int no;
    char name[50];
} test;
int main(int argc, char **argv)
{
    if (argc != 3)
        cout << "argc!=3" << endl;
    // 创建共享内存
    int shmid = shmget(0x5005, sizeof(test), 0640 | IPC_CREAT);
    cout << "shmid=" << shmid << endl;
    // 将共享内存连接到当前进程的地址空间
    test *ptr = (test *)shmat(shmid, 0, 0);
    if (ptr == (void *)-1)
    {
        cout << "shmat() failed" << endl;
        return -1;
    }
    //使用共享内存，对共享内存进行读写
    cout<<"原值：no="<<ptr->no<<", name="<<ptr->name<<endl;
    ptr->no = atoi(argv[1]);
    strcpy(ptr->name, argv[2]);//把argv[2]上的字符串拷贝到共享内存上
    cout << "新值：no=" << ptr->no << ", name=" << ptr->name << endl;
    //将共享内存从当前进程中分离
    shmdt(ptr);
    //删除共享内存
    
}