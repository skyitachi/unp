//
// Created by skyitachi on 2017/5/21.
//
// use epoll implement a tcp server
// doesn't use unp.h because of u_char u_int...
// so nocheck for some stupid error
// #include <unp/unp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#define MAX_EPOLL_EVENTS 1024

#define SA struct sockaddr
#define SERV_PORT 9877
#define LISTENQ 1024

int set_nonblock(int fd) {
    int of = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, of | O_NONBLOCK);
    return of;
}

void add_fd_to_epoll(int efd, int fd, int ev_type, int nonblock) {
    struct epoll_event ep_event;
    ep_event.data.fd = fd;
    ep_event.events = EPOLLIN | ev_type;
    if (nonblock) {
        set_nonblock(fd);
    }
    // Note: epoll control function
    epoll_ctl(efd, EPOLL_CTL_ADD, fd, &ep_event);
}

int main(int argc, char **argv) {
    int listenfd, connfd, epollfd;
    ssize_t n, nread;
    char buf[4096];
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    int32_t optval = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    bind(listenfd, (SA *)&servaddr, sizeof(servaddr));

    listen(listenfd, LISTENQ);

    struct epoll_event epoll_events[MAX_EPOLL_EVENTS];
    // create epoll
    if ((epollfd = epoll_create1(0)) == -1) {
        printf("epoll_create error\n");
        exit(1);
    }
    // add listenfd to epoll
    // edge trigger
    add_fd_to_epoll(epollfd, listenfd, EPOLLET, 1);
    
    for(;;) {
        n = epoll_wait(epollfd, epoll_events, MAX_EPOLL_EVENTS, -1);
        if (n == -1) {
            printf("epoll_wait error\n");
            exit(1);
        } else {
            printf("%d fd has events\n", n);
            for(int i = 0; i < n; i++) {
                if (epoll_events[i].data.fd == listenfd) { // new connection comes
                    printf("new connection comes\n");
                    struct sockaddr_in cliaddr;
                    socklen_t socklen;
                    connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &socklen);
                    printf("accept connection successful\n");
                    add_fd_to_epoll(epollfd, connfd, EPOLLET, 1/* non block*/);
                } else if(epoll_events[i].events & EPOLLIN) {
                    connfd = epoll_events[i].data.fd;
                    // edge trigger should read all data in one loop
                    for(;;) {
                        // block io
                        nread = recv(connfd, buf, 4096, 0);
                        if (nread == 0) {
                            printf("client close the connection\n");
                            break;
                        } else if (nread == -1) {
                            // TODO: in which situation will cause this case
                        }
                    }
                }
            }
        }
    }
}
