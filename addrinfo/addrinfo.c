//
// Created by skyitachi on 2017/5/20.
//

#include <unp/unp.h>

int main(int argc, char** argv) {
    if (argc < 2) {
        err_quit("Usage: ./addrinfo.bin hostname \n");
    }
    struct addrinfo hint, *result;
    bzero(&hint, sizeof(struct addrinfo));
    hint.ai_flags = AI_CANONNAME;
    hint.ai_family = AF_INET;

    int ret = getaddrinfo(argv[1], argv[2], &hint, &result);
    if (ret != 0) {
        printf("error is: %s\n", gai_strerror(ret));
    }
    do {
        char dst[INET_ADDRSTRLEN];
        struct sockaddr_in *sin = (struct sockaddr_in *) result->ai_addr;
        inet_ntop(AF_INET, &sin->sin_addr, dst, result->ai_addrlen);
        printf("CNAME is %s\n", result->ai_canonname);
        printf("IPv4 address is %s\n", dst);
        result = result->ai_next;
    } while(result != NULL);

    freeaddrinfo(result);
}
