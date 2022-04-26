/*
    2022-04-24
    本段代码是手写线程池与性能分析
*/
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>

#define LL_ADD(item, list)                  \
do {                                        \
    item->prev = NULL;                      \
    item->next = list;                      \
    if (list != NULL) list->prev = item;    \
    list = item;                            \
}while(0)

#define LL_DEL(item, list)                  \
do {                                        \
    if (item->prev != NULL) item->prev->next = item->next;  \
    if (item->next != NULL) item->next->prev = item->prev;  \
    if (list == item) list = item->next;                    \
    item->prev = item->next = NULL                          \
}while(0)

// 前置声明
typedef struct NMANAGER nThreadPool;

// 任务队列
struct NJOB
{
    void (*func)(void *arg);
    void * user_data;

    struct NJOB * prev;     // 节点的前指针
    struct NJOB * next;     // 节点的后指针
};

struct NWORKER
{
    pthread_t threadid;
    int terminal;       // 判断是否终止
    struct NMANAGER * pool;

    struct NWORKER * prev;
    struct NWORKER * next;
};

typedef struct NMANAGER
{
    pthread_mutex_t mtx;
    pthread_cond_t cond;
    struct NWORKER * workers;
    int waiting;
    
    struct NJOB * jobs;
} nThreadPool;

void * thread_callback(void * arg)
{
    struct NWORKER * worker = (struct NWORKER *)arg;

    while(1)
    {
        pthread_mutex_lock(&(worker->pool->mtx));
        while(worker->pool->jobs == NULL)
        {
            if (worker->terminal == 1)
            {
                break;
            }
            pthread_cond_wait(&(worker->pool->cond), &w(orker->pool->mtx));
        }

        // 退出
        if (worker->terminal == 1)
        {
            break;
        }
        
        // 取出任务
        struct NJOB * job = worker->pool->jobs;
        if (job != NULL)
        {
            LL_DEL(job, worker->pool->jobs);
        }
        // 任务数减一
        worker->pool->waiting--;
        pthread_mutex_unlock(worker->pool->mtx);

        // 回调函数
        job->func(job);
    }

    // 释放空间
    free(worker);
    return NULL;
}

// api
/*
    创建线程池
*/
int nThreadPoolCreate(nThreadPool * pool, int numworkers)
{
    // 1.检测参数
    if (numworkers < 0)
    {
        numworkers = 1; 
    }
    pool = (nThreadPool *)malloc(sizeof(nThreadPool));
    if (pool == NULL)
    {
        return -1;
    }
    
    // 初始化线程池
    pthread_mutex_t blank_mutex = PTHREAD_MUTEX_INITIALIZER;
    memcpy(pool->mtx, blank_mutex, sizeof(pthread_mutex_t));

    pthread_cond_t blank_cond = PTHREAD_COND_INITIALIZER;
    memcpy(pool->cond, blank_cond, sizeof(pthread_cond_t));

    // 初始化时任务为0
    pool->waiting = 0;
    int i = 0;
    for (i = 0; i < numworkers; i++)
    {
        struct NWORKER * worker = (struct NWORKER*)malloc(struct NWORKER);
        if (worker == NULL)
        {
            perror("malloc error");
            return -1;
        }
        memset(worker, 0, sizeof(struct NWORKER));

        worker->pool = pool;

        // 创建线程
        int ret = pthread_create(&worker->threadid, NULL, thread_callback, worker);
        if (ret)
        {
            perror("pthread_create");
            free(worker);
            return -1;
        }

        LL_ADD(worker, pool->worker);
    }

}

/*
    销毁线程池
*/
int nThreadPoolDestroy(nThreadPool * pool);

/*
    抛任务给线程池
*/
int nThreadPoolPost(nThreadPool * pool, struct  NJOB * job)
{
    pthread_mutex_lock(&(pool->mtx))
    LL_ADD(job, pool->jobs);
    // 任务数加1
    pool->waiting++;
    pthread_cond_signal(&pool->cond);
    pthread_mutex_unlock(&(pool->jobs));
}





int main()
{
    
    return 0;
}