//
// Created by skyitachi on 2017/5/30.
//
#include <unp/unp.h>

void str_cli(FILE *fp, int sockfd) {
    int maxfdpl, val, stdineof;
    ssize_t n, nwritten;
    fd_set rset, wset;
    char to[MAXLINE], fr[MAXLINE];
    char *toiptr, *tooptr, *friptr, *froptr;
    val = Fcntl(sockfd, F_GETFL, 0);
    Fcntl(sockfd, F_SETFL, val | O_NONBLOCK);

    val = Fcntl(STDIN_FILENO, F_GETFL, 0);
    Fcntl(STDIN_FILENO, F_SETFL, val | O_NONBLOCK);

    val = Fcntl(STDOUT_FILENO, F_GETFL, 0);
    Fcntl(STDOUT_FILENO, F_SETFL, val | O_NONBLOCK);
    toiptr = tooptr = to;
    friptr = froptr = fr;
    stdineof = 0;

    maxfdpl = max(max(STDIN_FILENO, STDOUT_FILENO), sockfd) + 1;

    for (;;) {
        FD_ZERO(&rset);
        FD_ZERO(&wset);
        if (stdineof == 0 && toiptr < &to[MAXLINE]) { // 标准输入到socket的缓冲区未满，仍然可以读取标准输入
            FD_SET(STDIN_FILENO, &rset);
        }
        if (friptr < &fr[MAXLINE]) { // socket的到标准输出的缓冲区未满，仍然可以从socket读取
            FD_SET(sockfd, &rset);
        }

        if (froptr != friptr) {
            FD_SET(STDOUT_FILENO, &wset);
        }

        Select(maxfdpl, &rset, &wset, NULL, NULL);

        if (FD_ISSET(STDIN_FILENO, &rset)) {
            if ((n = read(STDIN_FILENO, toiptr, &to[MAXLINE] - toiptr)) < 0) {
                if (errno != EWOULDBLOCK)
                    err_sys("read error on stdin");
            } else if (n == 0) {
                fprintf(stderr, "EOF on stdin\n");
                stdineof = 1;
                if (tooptr == toiptr) { // 没有数据可发送
                    Shutdown(sockfd, SHUT_WR);
                }
            } else {
                toiptr += n;
                FD_SET(sockfd, &wset); // 标准输入的缓冲区有可写入到socket的数据，所以打开可写标志
            }
        }

        if (FD_ISSET(sockfd, &rset)) {
            if ((n = read(sockfd, friptr, &fr[MAXLINE] - friptr)) < 0) {
                if (errno != EWOULDBLOCK)
                    err_sys("read error on sockfd");
            } else if (n == 0) {
                fprintf(stderr, "EOF on socket\n");
                if (stdineof) return;
                else
                    err_quit("str_cli: server terminated prematurely");
            } else {
                fprintf(stderr, "read %zd data bytes from socket\n", n);
                friptr += n;
                FD_ISSET(STDOUT_FILENO, &wset);
            }
        }

        if (FD_ISSET(STDOUT_FILENO, &wset) && (n = friptr - froptr) > 0) {
            if ((nwritten = write(STDOUT_FILENO, froptr, n)) < 0) {
                if (errno != EWOULDBLOCK) {
                    err_sys("write error to stdout");
                }
            } else {
                fprintf(stderr, "write %zd bytes to stdout\n", nwritten);
                froptr += nwritten;
                if (froptr == friptr) {
                    froptr = friptr = fr;
                }
            }
        }

        if (FD_ISSET(sockfd, &wset) && (n = toiptr - tooptr) > 0) {
            if ((nwritten = write(sockfd, tooptr, n)) < 0) {
                if (errno != EWOULDBLOCK) {
                    err_sys("write error to sockfd");
                }
            } else {
                fprintf(stderr, "write %zd bytes to sockfd\n", nwritten);
                tooptr += nwritten;
                if (tooptr == toiptr) {
                    tooptr = toiptr = to;
                    if (stdineof) {
                        Shutdown(sockfd, SHUT_WR);
                    }
                }
            }
        }

    }
}

int main(int argc, char **argv) {
    int sockfd;
    struct sockaddr_in servaddr;

    sockfd = Socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

    Connect(sockfd, (SA *)&servaddr, sizeof(servaddr));
    str_cli(stdin, sockfd);
    exit(0);
}

