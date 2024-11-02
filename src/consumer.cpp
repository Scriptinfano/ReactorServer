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
    while (true)
    {
        mutex.wait();
        // 如果队列为空则进入循环，否则直接处理数据，必须用循环不能用if
        while (queue->isEmpty())
        {
            // 队列为空时，自己要释放对共享内存的占有，也就是要解锁，然后等待生产者的唤醒信号
            mutex.post(); // 解锁
            cond.wait();  // 等待生产者的唤醒信号，这里会阻塞，一直到被唤醒
            mutex.wait(); // 被唤醒之后加锁，进入下一个循环，检查队列是否为空，不为空则可以继续处理数据，操作共享内存
        }

        queue->pop(&elem);
        cout << "no=" << elem.no << ",name=" << elem.name << endl;
        usleep(100);
    }
}