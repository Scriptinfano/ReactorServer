
#include <sys/shm.h>
#include "datastructures.hpp"
#include "semaphore.hpp"
using namespace std;

typedef struct test
{
    int no;
    char name[50];
} test;
int main(int argc, char **argv)
{
    if (argc != 3){
        cerr << "argc!=3" << endl;
        return 1;
    }

    // 创建共享内存
    int shmid = shmget(0x5005, sizeof(test), 0640 | IPC_CREAT);
    cout << "shmid=" << shmid << endl;
    // 将共享内存连接到当前进程的地址空间
    test *ptr = (test *)shmat(shmid, 0, 0); // 第二个参数指示将共享内存连接到当前进程的哪一个地址，通常填写0，让操作系统自行决定，第三个是标志位直接填0
    if (ptr == (void *)-1)
    {
        cout << "shmat() failed" << endl;
        return -1;
    }

    // 使用信号量对共享内存加锁
    Semaphore sem;
    if(sem.init(0x5005)==false){
        cerr << "mutex.init(0x5005) failed" << endl;
        return -1;
    }
    cout << "申请加锁" << endl;
    sem.wait();
    cout << "加锁成功" << endl;

    // 使用共享内存，对共享内存进行读写
    cout<< "原值：no=" << ptr->no << ", name=" << ptr->name << endl;
    ptr->no = atoi(argv[1]);
    strcpy(ptr->name, argv[2]); // 把argv[2]上的字符串拷贝到共享内存上
    cout << "新值：no=" << ptr->no << ", name=" << ptr->name << endl;
    sem.post();
    cout << "已解锁" << endl;
    // 将共享内存从当前进程中分离
    shmdt(ptr);
    // 删除共享内存
    if(shmctl(shmid,IPC_RMID,0)==-1){
        cerr << "shmctl failed" << endl;
        return -1;
    }
}