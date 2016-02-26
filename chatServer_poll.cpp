// =====================================================================================
// 
//       Filename:  chatServer_poll.cpp
// 
//    Description:  
// 
//        Version:  1.0
//        Created:  02/19/2016 07:35:07 AM
//       Revision:  none
//       Compiler:  g++
// 
//         Author:  李锐, root@lirui.in
//        License:  Copyright © 2016 李锐. All Rights Reserved.
// 
// =====================================================================================
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <poll.h>
#include "protocol.h"
#include <limits.h>

#define SERV_PORT 9877
#define LISTENQ 1024
#define MAXLINE 4096
#define OPEN_MAX 1024
#define INFTIM -1

//TODO manage the map, such as the uid is huge and when someone offline
int uid_socket_map[1024];
//TODO manage the client socket map
struct pollfd client_pollfd_map[OPEN_MAX];

ssize_t readn(int fd, void *vptr, size_t n)
{
    size_t nleft;
    ssize_t nread;
    char *ptr;

    ptr = static_cast<char *>(vptr);
    nleft = n;
    while (nleft > 0)
    {
        if ( (nread = read(fd, ptr, nleft)) < 0)
        {
            if(errno == EINTR)
                nread = 0;
            else
                return -1;
        }
        else if (nread == 0)
        {
            break;
        }
        nleft -= nread;
        ptr += nread;
    }
    return n - nleft;
}

ssize_t writen(int fd, void *vptr, size_t n)
{
    size_t nleft;
    ssize_t nwritten;
    const char *ptr;

    ptr = static_cast<char *>(vptr);
    nleft = n;
    while(nleft > 0)
    {
        if((nwritten = write(fd, ptr, nleft)) <= 0)
        {
            if(nwritten < 0 && errno == EINTR)
                nwritten = 0;
            else
                return -1;
        }
        nleft -= nwritten;
        ptr   += nwritten;
    }
    return n;
}

void process_one_client(int sockfd)
{
    ssize_t n;
    Message message;
    for (; ; )
    {
        if ( (n = readn(sockfd, &message, sizeof(message))) == 0)
        {
            //connection closed by client
            printf("connection closed by client\n");
            return ;
        }
        printf("uid: %d\n", message.uid);
        printf("nickname: %s\n", message.nickname);
        printf("content: %s\n", message.content);
        printf("t_uid: %d\n", message.t_uid);
        int uid = message.uid, t_uid = message.t_uid;
        uid_socket_map[uid] = sockfd;
        if(uid < 0)
        {
            printf("uid error\n");
            return ;
        }
        //start forward
        if(t_uid < 0)
        {
            for(int i = 0; i < 1024; i++)
            {
                if(uid_socket_map[i] != 0 && uid_socket_map[i] != sockfd)
                {
                    writen(uid_socket_map[i], &message, sizeof(message));
                }
            }
        }
        else
        {
            if(uid_socket_map[t_uid] != 0 && uid_socket_map[t_uid] != sockfd)
            {
                writen(uid_socket_map[t_uid], &message, sizeof(message));
            }
        }
    }
}

int main(int argc, char **argv)
{
    int listenfd, connfd;
    pid_t childpid;
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr;
    int maxi, nready;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    
    if(listenfd < 0)
    {
        printf("socket create error\n");
        return 0;
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);
    
    bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    
    char *ptr;
    int backlog = LISTENQ;
    if ( (ptr = getenv("LISTENQ")) != NULL)
    {
        backlog = atoi(ptr);
        //printf("%d\n", backlog);
    }
    if (listen(listenfd, backlog) < 0)
    {
        printf("listen error\n");
    }
    
    maxi = 0;
    client_pollfd_map[0].fd = listenfd;
    client_pollfd_map[0].events = POLLRDNORM;
    for(int i = 1; i < OPEN_MAX; i++)
        client_pollfd_map[i].fd = -1;

    for( ; ; )
    {
        nready = poll(client_pollfd_map, maxi + 1, INFTIM);
        if(nready < 0)
        {
            printf("poll error\n");
            return 0;
        }
        if (client_pollfd_map[0].revents & POLLRDNORM)
        {
            clilen = sizeof(cliaddr);
            connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);
            printf("accept one socket\n");
            int i;
            for(i = 1; i < OPEN_MAX; i++)
            {
                if(client_pollfd_map[i].fd < 0)
                {
                    client_pollfd_map[i].fd = connfd;
                    break;
                }
            }
            if(i == OPEN_MAX)
            {
                printf("too many clients\n");
                return 0;
            }
            client_pollfd_map[i].events = POLLRDNORM;
            if(i > maxi)
                maxi = i;
            if(--nready <= 0)
                continue;
        }

        for(int i = 1; i <= maxi; i++)
        {
            int sockfd;
            if ( (sockfd = client_pollfd_map[i].fd) < 0)
                continue;
            if (client_pollfd_map[i].revents & (POLLRDNORM | POLLERR))
            {
                process_one_client(client_pollfd_map[i].fd);
                //TODO process when connection is reset by client
                if(--nready <= 0)
                    break;
            }
        }
    }

    /*for( ; ; )
    {
        clilen = sizeof(cliaddr);
        connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);
        printf("accept one socket\n");
        if ( (childpid = fork()) == 0)
        {
            close(listenfd);
            process_one_client(connfd);
            return 0;
        }
    }*/
    return 0;
}
