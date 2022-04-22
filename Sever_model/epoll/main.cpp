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

    // 接下来就是epoll的操作了
    int epfd = epoll_create(1);
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = sockfd;

    // 
    epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);

    struct epoll_event events[1024] = {0};
    while(true)
    {
        int nReadys = epoll_wait(epfd, events, sizeof(events), -1);
        if (nReadys < 0)
        {
            break;
        }

        for (int i = 0; i < nReadys; i++)
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
        }

    }



    return 0;
}