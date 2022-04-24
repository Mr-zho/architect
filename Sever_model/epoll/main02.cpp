#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/epoll.h>
#include <iostream>
#include <stdlib.h>


// int g_epfd = 0;

// 结构体-> 保存io的状态
struct sockitem
{
    int sockfd;
    // 回调函数
    int (*callback)(int fd, int events, void * arg);
    char recvbuffer[1024];  // 
    char sendbuffer[1024];
};

// mainloop  eventloop
struct reactor
{
    int epfd;
    struct epoll_event events[512];
};

// 全局变量
struct reactor *eventloop = NULL;

// 函数前置声明
int recv_cb(int fd, int events, void * arg);

// 写事件
int send_cb(int fd, int events, void * arg)
{
    struct sockitem * si = (struct sockitem *)arg;
    send(fd, "sendto\n", strlen("hello\n"),0);

    // 再设置回去
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;

    si->sockfd = fd;
    si->callback = recv_cb;
    
    ev.data.ptr = si;
    epoll_ctl(eventloop->epfd, EPOLL_CTL_MOD, fd, &ev);

    return 0;
}

// 读事件
int recv_cb(int fd, int events, void * arg)
{
    struct sockitem *si = (struct sockitem *)arg;
    struct epoll_event ev;

    char buffer[1024] = {0};
    int ret = recv(fd, buffer, sizeof(buffer), 0);
    if(ret < 0)
    {
        if(errno == EAGAIN || errno == EWOULDBLOCK)
        {

        }
        else
        {

        }
        
        
        
        ev.events = EPOLLIN;
        ev.data.fd = fd;
        epoll_ctl(eventloop->epfd, EPOLL_CTL_DEL, fd, &ev);

        // 客户端断开连接
        close(fd);

        // 释放内存
        free(si);
    }
    else if(ret == 0)
    {   
        ev.events = EPOLLIN;
        ev.data.fd = fd;
        epoll_ctl(eventloop->epfd, EPOLL_CTL_DEL, fd, &ev);

        // 客户端断开连接
        close(fd);

        // 释放内存
        free(si);
    }
    else
    {   
        printf ("Recv:%s ,%dBytes\n", buffer, ret);

        // 写给客户端
        #if 0
        ret = send();
        #else
        struct epoll_event ev;
        ev.events = EPOLLOUT | EPOLLET;

        si->sockfd = fd;
        si->callback = send_cb;
        
        ev.data.ptr = si;
        epoll_ctl(eventloop->epfd, EPOLL_CTL_MOD, fd, &ev);
        
        #endif
    }

    return 0;
}

// 回调函数
int accept_cb(int fd, int events, void * arg)
{
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(struct sockaddr_in));
    socklen_t client_len = sizeof(client_addr);

    int client_fd = accept(fd, (struct sockaddr *)&client_addr, &client_len); 
    if(client_fd < 0)
    {
        return -1;
    }

    char str[INET_ADDRSTRLEN] = {0};
    printf("recv from %s at post:%d\n",
            inet_ntop(AF_INET,&client_addr.sin_addr,str, INET_ADDRSTRLEN),
            ntohs(client_addr.sin_port));
    
    
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    // ev.data.fd = client_fd;

    struct sockitem * si = (struct sockitem *)malloc(sizeof(struct sockitem));
    si->sockfd = client_fd;
    si->callback = recv_cb;

    ev.data.ptr = si;
    epoll_ctl(eventloop->epfd, EPOLL_CTL_ADD, client_fd, &ev);

    return client_fd;
}

int main(int argc, const char *argv[])
{   
    if (argc < 2)
    {
        return -1;
    }
    char *endptr = NULL;
    int port = strtoul(argv[1], &endptr, 10);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr));


    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(struct sockaddr)) < 0)
    {
        return -2;
    }

    if (listen(sockfd, 5) < 0)
    {
        return -3;
    }

    eventloop = (struct reactor*)malloc(sizeof(struct reactor));

    // 接下来就是epoll的操作了
    eventloop->epfd = epoll_create(1);
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = sockfd;

    struct sockitem *si = (struct sockitem *)malloc(sizeof(sizeof(sockitem)));
    si->sockfd = sockfd;
    si->callback = accept_cb;
    ev.data.ptr = si;

    
    epoll_ctl(eventloop->epfd, EPOLL_CTL_ADD, sockfd, &ev);

    // struct epoll_event events[1024] = {0};
    while(true)
    {
        int nReadys = epoll_wait(eventloop->epfd, eventloop->events, sizeof(eventloop->events), -1);
        if (nReadys < 0)
        {
            break;
        }

        for (int i = 0; i < nReadys; i++)
        {
// 第一版本
#if 0
            if (events[i].data.fd == sockfd)
            {
                struct sockaddr_in client_addr;
                memset(&client_addr, 0, sizeof(struct sockaddr_in));
                socklen_t client_len = sizeof(client_addr);
                int clientfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_len);
                if (clientfd < 0)
                {
                    continue;
                }

                char visitip[INET_ADDRSTRLEN] = {0};
                printf("ip:%s,port:%d\n",
                        inet_ntop(AF_INET, &(client_addr.sin_addr),
                                visitip, INET_ADDRSTRLEN), ntohs(client_addr.sin_port));

                ev.events = EPOLLIN | EPOLLET;  // EPOLLET:边沿触发
                ev.data.fd = clientfd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, clientfd, &ev);
            }
            else
            {

                // 取出通信句柄
                int client_fd = events[i].data.fd;
                char buffer[1024] = {0};

                int ret = recv(client_fd, buffer, sizeof(buffer), 0);
                if (ret < 0)
                {
                    if (errno == EAGAIN || errno == EWOULDBLOCK)
                    {
                        printf("normal\n");
                        continue;
                    }
                    else
                    {
                        close(client_fd);
                        ev.events = EPOLLIN;
                        ev.data.fd = client_fd;
                        epoll_ctl(epfd, EPOLL_CTL_DEL, client_fd, &ev);
                    }
                }
                else if (ret == 0)
                {
                    // 客户端关闭通信
                    close(client_fd);

                    ev.events = EPOLLIN;
                    ev.data.fd = client_fd;
                    epoll_ctl(epfd, EPOLL_CTL_DEL, client_fd, &ev);
                }
                else
                {
                    // 数据
                    printf ("Recv:%s,%d Bytes\n", buffer, ret);
                }

                if (events[i].events & EPOLLIN) //此操作确认是读事件
                {
                    // 取出通信句柄
                    int client_fd = events[i].data.fd;
                    char buffer[1024] = {0};

                    int ret = recv(client_fd, buffer, sizeof(buffer), 0);
                    if (ret < 0)
                    {
                        if (errno == EAGAIN || errno == EWOULDBLOCK)
                        {
                            printf("normal\n");
                            continue;
                        }
                        else
                        {
                            close(client_fd);
                            ev.events = EPOLLIN;
                            ev.data.fd = client_fd;
                            epoll_ctl(epfd, EPOLL_CTL_DEL, client_fd, &ev);
                        }
                    }
                    else if (ret == 0)
                    {
                        // 客户端关闭通信
                        close(client_fd);

                        ev.events = EPOLLIN;
                        ev.data.fd = client_fd;
                        epoll_ctl(epfd, EPOLL_CTL_DEL, client_fd, &ev);
                    }
                    else
                    {
                        // 数据
                        printf ("Recv:%s,%d Bytes\n", buffer, ret);
                    }
                }

                if (events[i].events & EPOLLOUT)
                {
                    int clientfd = events[i].data.fd;

                    int ret = send(clientfd, );

                    ev.events |= EPOLLOUT;

                    epoll_ctl();
                }
#endif

// 第二版本
#if 0
            if(events[i].events & EPOLLIN)  // 有读事件
            {
                if (events[i].data.fd == sockfd)
                {
                struct sockaddr_in client_addr;
                memset(&client_addr, 0, sizeof(struct sockaddr_in));
                socklen_t client_len = sizeof(client_addr);
                int clientfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_len);
                if (clientfd < 0)
                {
                    continue;
                }

                char visitip[INET_ADDRSTRLEN] = {0};
                printf("ip:%s,port:%d\n",
                        inet_ntop(AF_INET, &(client_addr.sin_addr),
                                visitip, INET_ADDRSTRLEN), ntohs(client_addr.sin_port));

                ev.events = EPOLLIN | EPOLLET;  // EPOLLET:边沿触发
                ev.data.fd = clientfd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, clientfd, &ev);
            }
            else
            {
                // 取出通信句柄
                int client_fd = events[i].data.fd;
                char buffer[1024] = {0};

                int ret = recv(client_fd, buffer, sizeof(buffer), 0);
                if (ret < 0)
                {
                    if (errno == EAGAIN || errno == EWOULDBLOCK)
                    {
                        printf("normal\n");
                        continue;
                    }
                    else
                    {
                        close(client_fd);
                        ev.events = EPOLLIN;
                        ev.data.fd = client_fd;
                        epoll_ctl(epfd, EPOLL_CTL_DEL, client_fd, &ev);
                    }
                }
                else if (ret == 0)
                {
                    // 客户端关闭通信
                    close(client_fd);

                    ev.events = EPOLLIN;
                    ev.data.fd = client_fd;
                    epoll_ctl(epfd, EPOLL_CTL_DEL, client_fd, &ev);
                }
                else
                {
                    // 数据
                    printf ("Recv:%s,%d Bytes\n", buffer, ret);
                }
            }

            if(events[i].events & EPOLLOUT) // 有写事件
            {
                // 这边是有写事件
            }
#endif

// 第三版本
#if 1   
            if(eventloop->events[i].events & EPOLLIN)  // 有读事件
            { 
                struct sockitem * si = (struct sockitem *)eventloop->events[i].data.ptr;
                // 不同做区分sockfd和clientfd
                si->callback(si->sockfd, eventloop->events[i].events, si);
            }

            if(eventloop->events[i].events & EPOLLOUT) // 有写事件
            {
                // 有写事件
                struct sockitem * si = (struct sockitem *)eventloop->events[i].data.ptr;
                si->callback(si->sockfd, eventloop->events[i].events, si);
            }
#endif
        }

    }

    return 0;
}