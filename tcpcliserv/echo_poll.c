//
// Created by skyitachi on 2017/5/16.
//

#include <unp/unp.h>
#include <limits.h>

#ifndef OPEN_MAX
#define OPEN_MAX 1024
#endif

int main(int argc, char **argv) {
    int i, maxi, listenfd, connfd, sockfd;
    int nready;
    int clilen;
    ssize_t n;
    char buf[MAXLINE];

    struct pollfd client[OPEN_MAX];
    struct sockaddr_in servaddr, cliaddr;

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);
    int optval = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

    Listen(listenfd, LISTENQ);

    client[0].fd = listenfd;
    client[0].events = POLLIN;
    
    maxi = -1;
    for(i = 1; i < OPEN_MAX; i++) {
        client[i].fd = -1;
    }

    for(;;) {
        nready = Poll(client, OPEN_MAX, -1);
        // check new connection comes
        printf("ready client is %d\n", nready);
        if(client[0].revents & POLLIN) {
            clilen = sizeof(cliaddr);
            connfd = Accept(listenfd, (SA *)&cliaddr, &clilen);
            
            for(i = 1; i < OPEN_MAX; i++) {
                if (client[i].fd < 0) {
                    client[i].fd = connfd;
                    client[i].events = POLLIN;
                    break;
                }
            }
            if(i == OPEN_MAX) {
                err_quit("too many clients\n");
            }
            if(i > maxi) maxi = i;
            if(--nready <= 0) continue;
        }
        
        for(i = 1; i <= maxi; i++) {
            if((sockfd = client[i].fd) < 0) continue;
            if (client[i].revents & POLLIN) {
                printf("in the pollin\n");
                n = Read(sockfd, buf, MAXLINE);
                if (errno == ECONNRESET) {
                    printf("in the connection reset\n");
                    Close(sockfd);
                    client[i].fd = -1;
                } else if (n == 0) {
                    printf("client shutdown positively\n");
                    client[i].fd = -1;
                    Close(sockfd);
                    if (i == maxi) {
                        maxi = maxi - 1;
                    }
                    continue;
                }
                printf("client data is %s\n, client len is %d\n", buf, n);
                Writen(sockfd, buf, n);
                if (--nready <= 0) break;
            }
        }
    }
}

