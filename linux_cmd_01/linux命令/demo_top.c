#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#define TOP_HP 1

void * thread_func(void * arg)
{
    while(1)
    {
        printf("thread_self:%ld\n",pthread_self());
        sleep(5);
    }
}


/*
 目的:
 top -Hp pid 查看具体进程下的线程
*/
int main()
{
#if TOP_HP
    pthread_t tid;
    pthread_create(&tid, NULL, thread_func, NULL);
#endif

    while(1)
    {
        printf("self:%ld\n",pthread_self());
        sleep(3);
    }
    return 0;
}