//
// Created by skyitachi on 2017/5/3.
//

#include <unp/unp.h>

struct args {
    long arg1;
    long arg2;
};

struct result {
    long sum;
};

void str_cli(FILE *fp, int sockfd) {
    char sendline[MAXLINE], recvline[MAXLINE];

    while(Fgets(sendline, MAXLINE, fp) != NULL) {
        Writen(sockfd, sendline, strlen(sendline));
        sleep(1);
        Writen(sockfd, sendline + 1, strlen(sendline) - 1); // demonstrate SIGPIPE

        if (Readline(sockfd, recvline, MAXLINE) == 0) { // wait for newline
            err_quit("str_cli: server terminated prematurely");
        }
        Fputs(recvline, stdout);
    }
}

void str_cli_select(FILE *fp, int sockfd) {
    char sendline[MAXLINE], recvline[MAXLINE];

    int fpd = fileno(fp);
    int maxfd = max(sockfd, fpd) + 1;

    fd_set rset;

    FD_ZERO(&rset);
    FD_SET(sockfd, &rset);
    FD_SET(fpd, &rset);

    for(;;) {
        Select(maxfd, &rset, NULL, NULL, NULL);
        if (FD_ISSET(sockfd, &rset)) {
            if (Readline(sockfd, recvline, MAXLINE) == 0) {
                err_quit("str_cli: server terminated prematurely");
            }
            Fputs(recvline, stdout);
        }

        if (FD_ISSET(fpd, &rset)) {
            if (Fgets(sendline, MAXLINE, fp) == NULL) // read eof
                return;
            Writen(sockfd, sendline, strlen(sendline));
        }
    }
}

// read from the buffer
void str_cli_select_2(FILE *fp, int sockfd) {
    char sendline[MAXLINE], recvline[MAXLINE];

    int fpd = fileno(fp);
    int maxfd = max(sockfd, fpd) + 1;

    fd_set rset;

    FD_ZERO(&rset);

    int eof = 0;
    ssize_t  n;
    for(;;) {
        if (eof == 0) {
            FD_SET(fpd, &rset);
        }
        FD_SET(sockfd, &rset);
        Select(maxfd, &rset, NULL, NULL, NULL);
        if (FD_ISSET(sockfd, &rset)) {
            printf("sockfd is ready\n");
            if ((n = Read(sockfd, recvline, MAXLINE)) == 0) { // eof
                if (eof == 1) {
                    return;
                }
            }
//            recvline[n] = '\0'; // 手动添加
            Write(fileno(stdout), recvline, n);
//            Fputs(recvline, stdout);
        }

        if (FD_ISSET(fpd, &rset)) {
            printf("\nfp read already\n");
            if ((n = Read(fpd, sendline, MAXLINE)) == 0) { // read stdin eof
                printf("in the eof\n");
                eof = 1;
                shutdown(sockfd, SHUT_WR); // 停止写
                FD_CLR(fpd, &rset);
                continue;
            }
//            sendline[n] = '\0';
            printf("\nthe content is: %s\n", sendline);
            Writen(sockfd, sendline, strlen(sendline));
        }
    }
}

void str_cli_sum(FILE *fp, int sockfd) {
    char sendline[MAXLINE];
    struct args args;
    struct result result;

    while(Fgets(sendline, MAXLINE, fp) != NULL) {
        if (sscanf(sendline, "%ld%ld", &args.arg1, &args.arg2) != 2) {
            printf("invaild input\n");
            continue;
        }
        Writen(sockfd, &args, sizeof(args));

        if (Readn(sockfd, &result, sizeof(result)) == 0) {
            err_quit("str_cli: server terminated prematurely");
        }
        printf("%ld\n", result.sum);
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
//    str_cli(stdin, sockfd);
//    str_cli_select(stdin, sockfd);
//    str_cli_select_2(stdin, sockfd);
    str_cli_sum(stdin, sockfd);
    exit(0);
}

