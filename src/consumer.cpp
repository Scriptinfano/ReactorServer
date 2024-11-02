#include "temp.hpp"
#include "datastructures.hpp"
#include "semaphore.hpp"
#include <sys/shm.h>
#include <unistd.h>
int main()
{
    
    int shmid = shmget(0x5005, sizeof(CircularQueue<test, 5>), 0640 | IPC_CREAT);
    if (shmid == -1)
    {
        cerr << "shmget(0x5005) failed" << endl;
        return -1;
    }
    CircularQueue<test, 5> *queue = (CircularQueue<test, 5> *)shmat(shmid, 0, 0);
    if (queue == (void *)-1)
    {
        cerr << "shmat() failed" << endl;
        return -1;
    }
    queue->init();
    test elem;
    Semaphore mutex, cond;
    mutex.init(0x5001);      // 用于给共享内存加锁
    cond.init(0x5002, 0, 0); // 信号量的值用于表示队列中数据元素的个数
    print_sem(mutex, "[1] mutex");
    print_sem(cond, "[1] cond");
    while (true)
    {
        print_sem(mutex, "[2] mutex");
        print_sem(cond, "[2] cond");
        mutex.wait();
        print_sem(mutex, "[3] mutex");
        print_sem(cond, "[3] cond");
        // 如果队列为空则进入循环，否则直接处理数据，必须用循环不能用if
        while (queue->isEmpty())
        {
            // 队列为空时，自己要释放对共享内存的占有，也就是要解锁，然后等待生产者的唤醒信号
            mutex.post(); // 解锁
            print_sem(mutex, "[4] mutex");
            print_sem(cond, "[4] cond");
            cond.wait(); // 等待生产者的唤醒信号，这里会阻塞，一直到被唤醒
            print_sem(mutex, "[5] mutex");
            print_sem(cond, "[5] cond");
            mutex.wait(); // 被唤醒之后加锁，进入下一个循环，检查队列是否为空，不为空则可以继续处理数据，操作共享内存
            print_sem(mutex, "[6] mutex");
            print_sem(cond, "[6] cond");
        }

        queue->pop(&elem);
        cout << "no=" << elem.no << ",name=" << elem.name << endl;
        
        usleep(100);
        print_sem(mutex, "[7] mutex");
        print_sem(cond, "[7] cond");
        mutex.post(); // 操作完共享内存再次解锁
        print_sem(mutex, "[8] mutex");
        print_sem(cond, "[8] cond");
    }
}
