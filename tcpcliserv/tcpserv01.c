//
// Created by skyitachi on 2017/5/3.
//
// TODO should deal with SIGCHLD and SIGPIPE
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

// read two numbers and write sum to the socket
void str_echo2(int sockfd) {
    long arg1, arg2;
    ssize_t  n;
    char line[MAXLINE];
    for(;;) {
        if ((n = Readline(sockfd, line, MAXLINE)) == 0) {
            return;
        }
        if (sscanf(line, "%ld%ld", &arg1, &arg2) == 2) {
            snprintf(line, sizeof(line), "%ld\n", arg1 + arg2);
        } else {
            snprintf(line, sizeof(line), "input error\n");
        }
        n = strlen(line);
        Writen(sockfd, line, n);
    }
}

int main(int argc, char **argv) {
    int listenfd, connfd;
    pid_t childpid;
    socklen_t childlen;
    struct sockaddr_in childaddr, servaddr;

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);
    int optval = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
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
            str_echo2(connfd);
//            str_echo(connfd);
//            printf("leave the child\n");
            Close(connfd);
            exit(0); // auto close connfd
        }
        Close(connfd);
    }
}

