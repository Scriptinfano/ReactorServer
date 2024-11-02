#include "temp.hpp"
#include "datastructures.hpp"
#include "semaphore.hpp"
#include <sys/shm.h>
int main(){
    int shmid = shmget(0x5005, sizeof(CircularQueue<test, 5>), 0640 | IPC_CREAT);
    if(shmid==-1){
        cerr << "shmget(0x5005)failed" << endl;
        return -1;
    }
    CircularQueue<test, 5> *queue = (CircularQueue<test, 5> *)shmat(shmid, 0, 0);//将共享内存区域附加到当前进程的地址空间，并将其转换为指向CircularQueue<test,5>类型的指针
    if(queue==(void *)-1){
        cerr << "shmat() failed" << endl;
        return -1;
    }
    queue->init();
    test elem1,elem2,elem3;
    Semaphore mutex, cond;
    mutex.init(0x5001);
    cond.init(0x5002, 0, 0);//第二个参数是信号量的初值，这里表示在生产消费
    mutex.wait();//加锁
    //在共享内存中生产三个数据
    init_test(3, "mick", &elem1);
    init_test(7,"nick",&elem2);
    init_test(8, "alice", &elem3);
    queue->push(elem1);
    queue->push(elem2);
    queue->push(elem3);
    mutex.post();//解锁
    cond.post(3);//表示生产了3个数据
    shmdt(queue);//将共享内存从当前进程中分离
}