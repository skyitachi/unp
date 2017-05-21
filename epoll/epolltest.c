//
// Created by skyitachi on 2017/5/21.
//
// use epoll implement a tcp server
#include <unp/unp.h>

#define MAX_EPOLL_EVENTS 1024

int main(int argc, char **argv) {
    int listenfd, connfd;
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);
    int32_t optval = 1;
    Setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    Bind(listenfd, (SA *)&servaddr, sizeof(servaddr));

    Listen(listenfd, LISTENQ);

    struct epoll_event epoll_events[MAX_EPOLL_EVENTS];
}