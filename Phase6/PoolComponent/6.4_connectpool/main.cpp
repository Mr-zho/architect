/*
    2022-04-24
    本段代码是手写线程池与性能分析
*/


#include <stdio.h>


void push_tasks(void *arg)
{
#if 0
    recv();

    parse();

    send();
#endif
}

void eventloop()
{
    while (1)
    {
        int nready = event_wait();

        for (int i = 0; i < nready; i++)
        {
#if 0
            recv();

            parse();        // 这边不要运行耗时的操作

            send();
#elif 0
            task = fd;      // 针对fd操作事件比较长的
            push_tasks(task);

            // 具体不足的地方是：
            // 1.会出现多个线程公用一个fd的情况
            // 

#else
            recv(fd, buffer, length, 0);
            push_tasks(buffer);         // 适合业务耗时的操作
#endif
        }
    }
}

int main()
{
    
    return 0;
}