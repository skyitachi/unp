//
// Created by skyitachi on 2017/5/16.
//

#include <unp/unp.h>
#include <limits.h>

int main(int argc, char **argv) {
    int i, maxi, listenfd, connfd, sockfd;
    int nready;
    ssize_t n;
    char buf[MAXLINE];

    struct pollfd client[OPEN_MAX];
    struct sockaddr_in servaddr, cliaddr;

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

    Listen(listenfd, LISTENQ);

    client[0].fd = listenfd;
    client[0].events = POLLIN;

    for(i = 1; i < OPEN_MAX; i++) {
        client[i].fd = -1;
    }

    for(;;) {
        nready = Poll(&client, OPEN_MAX, -1);
        printf("ready client is %d\n", nready);
    }
}

