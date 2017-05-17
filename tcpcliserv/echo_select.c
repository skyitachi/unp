//
// Created by skyitachi on 2017/5/15.
//

#include <unp/unp.h>

#define MAX_CLIENT 1

int main(int argc, char **argv) {
    int i, listenfd, connfd, sockfd;
    int maxfd; // 当前最大的fd
    int maxi; // 当期client中最大可读的index
    int nready; // number of data ready clients
    int client[MAX_CLIENT];
    ssize_t n;
    fd_set rset, allset;
    char buf[MAXLINE];
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr;

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);
    int optval = 1;
    Setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    Bind(listenfd, (SA *)&servaddr, sizeof(servaddr));

    Listen(listenfd, LISTENQ);
    maxfd = listenfd;
    maxi = -1;

    for(i = 0; i < MAX_CLIENT; i++) client[i] = -1;
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);  // 将listenfd push到allset中

    for(;;) {
        rset = allset;
        nready = Select(maxfd + 1, &rset, NULL, NULL, NULL);

        if (FD_ISSET(listenfd, &rset)) {
            clilen = sizeof(cliaddr);
            connfd = Accept(listenfd, (SA *)&cliaddr, &clilen);
            for (i = 0; i < MAX_CLIENT; i++) {
                if (client[i] < 0) {
                    client[i] = connfd;
                    break;
                }
            }

            if (i == MAX_CLIENT) {
                for(i = 0; i < MAX_CLIENT; i++) {
                    if (client[i] != -1) {
                        Close(client[i]);
                    }
                }
                err_quit("too many client");
            }

            FD_SET(connfd, &allset); // 将已经建立连接的fd放到FD_SET中
            if (connfd > maxfd) {
                maxfd = connfd;
            }
            if (i > maxi) {
                maxi = i;
            }
            if (--nready <= 0) {
                continue;
            }
        }

        for(i = 0; i <= maxi; i++) {
            if ((sockfd = client[i]) < 0) continue;
            if (FD_ISSET(sockfd, &rset)) {
                if ((n = Read(sockfd, buf, MAXLINE)) == 0) {
                    Close(sockfd);
                    FD_CLR(sockfd, &allset);
                    client[i] = -1;
                } else Write(sockfd, buf, n);

                if (--nready <= 0) break;
            }
        }

    }
}