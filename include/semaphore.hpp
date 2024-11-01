#include <sys/sem.h>
#include "myerror.hpp"

class Semaphore
{
private:
    /*
    union semun是一种常见的union类型，用于semctl函数中配置或或获取信号量的信息
    */
    union semun
    {
        int val;//信号量的值，用于SETVAL操作，设置信号量的单个整数值
        struct semid_ds *buf; // 描述信号量的属性和状态，用于IPC_STAT或IPC_SET操作，分别用于读取或设置信号量状态信息
        unsigned short *array;//指向一个数组，操作信号量集，用于GETALL和SETALL操作，获取或设置信号量的值
    };
    int semid;
    /*
    如果把sem_flg设置为SEM_UNDO，操作系统将跟踪进程对信号量的修改情况，在全部修改过信号量的进程终止后，操作系统将把信号量恢复为初始值。
    如果信号量用于互斥锁，设置为SEM_UNDO
    如果信号量用于生产消费者模型，设为0
    */
    short semflg;
    Semaphore(const Semaphore &) = delete;
    Semaphore &operator=(const Semaphore &) = delete;

public:
    Semaphore() : semid(-1) {}
    /*
    信号量的初始化分为三个步骤：
    1. 获取信号量，如果成功则函数返回
    2. 如果失败则创建信号量
    3. 设置信号量的初始值
    @param key key_t类型的键值由ftok函数生成
    @param value 信号量的初始值
    @param the_semflg 控制标志，用于指定权限和行为；IPC_CREAT：若指定的 key 尚未存在，则创建一个新的信号量集；若已存在，则获取它；IPC_EXCL：与 IPC_CREAT 一起使用，若指定的 key 已存在则返回错误，否则创建一个新的信号量集;SEM_UNDO：当进程结束时，自动撤销对信号量的操作，避免资源泄露。
    */
    bool init(key_t key, unsigned short value = 1, short the_semflg = SEM_UNDO)
    {
        if (semid != -1)
            return false;
        semflg = the_semflg;
        /*
        semget各个参数解释
        key用来唯一标识信号量集
        1: 请求的信号量数量
        0666: 权限位
        IPC_CREAT: 如果信号量不存在则创建一个新的信号量集
        IPC_EXCL: 如果信号量集已经存在，则调用失败
        IPC_CREAT|IPC_EXCL 的使用会使的如果信号量存在时的errno被设为EEXIST
        */
        if ((semid = semget(key, 1, 0666 | IPC_CREAT | IPC_EXCL)) == -1 && errno == EEXIST)
        {
            // 信号量已存在的情况下再次尝试获取信号量
            if ((semid = semget(key, 1, 0666)) == -1)
            {
                //再次尝试获取信号量之后失败
                print_error("init函数中调用semget获取已有信号量时失败");
                return false;
            }
            //再次尝试获取信号量之后成功
        }else{
            print_error("init函数中发生了其他错误");
            return false;
        }
        union semun sem_union;
        sem_union.val = value;
        if(semctl(semid,0,SETVAL,sem_union)<0)
            print_error("init函数中调用semctl()时失败");
        return true;
    }
    /*
    信号量的P操作，将信号量的值-value，如果信号量的值是0则阻塞等待，直到信号量的值大于0
    */
    bool wait(short value = -1){
        if(semid==-1)
            return false;
        
        /*
        sembuf是用于描述信号量操作的结构体，配合semop函数用于执行信号量的增减操作或等待操作
        */
        struct sembuf sem_b;
        sem_b.sem_num = 0;//信号量编号，0代表信号量集中的第一个信号量
        sem_b.sem_op = value; // 操作数，用于指定信号量的增减值
        sem_b.sem_flg = semflg;
        if(semop(semid,&sem_b,1)==-1){
            print_error("在wait函数中调用semop函数时出粗");
            return false;
        }
        return true;
    }
    /*
    信号量的V操作，将信号量的值+value
    */
    bool post(short value = 1){
        if(semid==-1)
            return false;
        struct sembuf sem_b;
        sem_b.sem_num=0;
        sem_b.sem_op = value;
        sem_b.sem_flg = semflg;
        if(semop(semid,&sem_b,1)==-1){
            print_error("在函数post中调用semop函数时出错");
            return false;
        }
        return true;
    }
    /*
    获取信号量的值，成功则返回信号量的值，失败返回-1
    */
    int getvalue(){
        return semctl(semid, 0, GETVAL);
    }
    bool destroy(){
        if(semid==-1)
            return false;
        if(semctl(semid,0,IPC_RMID)==-1){
            print_error("在destroy函数中调用semctl销毁信号量失败");
            return false;
        }
        return true;
    }
};