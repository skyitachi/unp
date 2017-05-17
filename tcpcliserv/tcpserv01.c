//
// Created by skyitachi on 2017/5/3.
//

#include <unp/unp.h>

void str_echo(int sockfd) {
    ssize_t n;
    char buf[MAXLINE];

again:
    while((n = read(sockfd, buf, MAXLINE)) > 0) {
        Writen(sockfd, buf, n);
    }

    if (n < 0 && errno == EINTR)
        goto again;
    else if (n < 0) {
        err_sys("str_echo: read error");
    }
}

int main(int argc, char **argv) {
    int listenfd, connfd;
    pid_t childpid;
    socklen_t childlen;
    struct sockaddr_in childaddr, servaddr;

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT/* 9877 */);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    Bind(listenfd, (SA *)&servaddr, sizeof(servaddr));
    Listen(listenfd, LISTENQ);

    for(;;) {
        childlen = sizeof(childaddr);
        connfd = Accept(listenfd, (SA *)&childaddr, &childlen);
        if ((childpid = Fork()) == 0) {
            Close(listenfd); // should close
            str_echo(connfd);
//            printf("leave the child\n");
            Close(connfd);
            exit(0); // auto close connfd
        }
        Close(connfd);
    }
}

