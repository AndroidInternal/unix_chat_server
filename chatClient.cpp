// =====================================================================================
// 
//       Filename:  chatClient.cpp
// 
//    Description:  
// 
//        Version:  1.0
//        Created:  02/21/2016 11:20:17 AM
//       Revision:  none
//       Compiler:  g++
// 
//         Author:  李锐, root@lirui.in
//        License:  Copyright © 2016 李锐. All Rights Reserved.
// 
// =====================================================================================
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include "./protocol.h"
#include <stdlib.h>

#define SERV_PORT 9877
#define MAXLINE 4096

ssize_t writen(int fd, const char *vptr, size_t n)
{
    size_t nleft;
    ssize_t nwritten;
    const char *ptr;

    ptr = vptr;
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
        ptr += nwritten;
    }
    return n;
}

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
        else if(nread == 0)
            break;
        nleft -= nread;
        ptr += nread;
    }
    return n - nleft;
}

void str_cli(int sockfd)
{
    char nickname[24], content[1024];
    int uid;
    while(scanf("%d %s %s", &uid, nickname, content) != EOF)
    {
        Message message;
        message.uid = uid;
        strcpy(message.nickname, nickname);
        strcpy(message.content, content);
        write(sockfd, &message, sizeof(message));
        //TODO receving and sending...
        ssize_t n;
        if ( (n = readn(sockfd, &message, sizeof(message))) == 0)
        {
            printf("connection closed by client\n");
            return ;
        }
    }
}

int main(int argc, char **argv)
{
    int sockfd;
    struct sockaddr_in servaddr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        printf("socket create error\n");
        return 0;
    }
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
    connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    str_cli(sockfd);
    return 0;
}
