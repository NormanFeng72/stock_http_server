#include "htime.h"
#include <stdio.h>
#include <stdlib.h>

#include "hatomic.h"
#include "hthread.h"
#include <string.h>
#include <assert.h>

#include "md5.h"
#include "sha1.h"
#include "hsocket.h"

hatomic_flag_t flag = HATOMIC_FLAG_INIT;
hatomic_t cnt = HATOMIC_VAR_INIT(0);

HTHREAD_ROUTINE(test_hatomic_flag) {
    if (!hatomic_flag_test_and_set(&flag)) {
        // printf("tid=%ld flag 0=>1\n", hv_gettid());
    }
    else {
        // printf("tid=%ld flag=1\n", hv_gettid());
    }
    return 0;
}

HTHREAD_ROUTINE(test_hatomic) {
    for (int i = 0; i < 10; ++i) {
        long n = hatomic_inc(&cnt);
        // printf("tid=%ld cnt=%ld\n", hv_gettid(), n);
        hv_delay(1);
    }
    return 0;
}

int test_hatomic_main() {
    for (int i = 0; i < 10; ++i) {
        hthread_create(test_hatomic_flag, NULL);
    }

    for (int i = 0; i < 10; ++i) {
        hthread_create(test_hatomic, NULL);
    }

    hv_delay(1000);
    return 0;
}

int test_date() {
    datetime_t dt = datetime_now();
    char buf1[DATETIME_FMT_BUFLEN];
    datetime_fmt(&dt, buf1);
    puts(buf1);

    time_t ts = datetime_mktime(&dt);
    char buf2[GMTIME_FMT_BUFLEN];
    gmtime_fmt(ts, buf2);
    puts(buf2);

    return 0;
}

int test_md5(char* data) {
    char md5[33];
    unsigned char ch = '1';
    hv_md5_hex(&ch, 1, md5, sizeof(md5));
    assert(strcmp(md5, "c4ca4238a0b923820dcc509a6f75849b") == 0);

    const char* flags = strdup("-s");
    if (flags[0] == '-' && flags[1] == 's') {
        hv_md5_hex((unsigned char*)data, strlen(data), md5, sizeof(md5));
    }
    else {
        printf("Unrecognized flags '%s'\n", flags);
        return -40;
    }

    puts(md5);

    return 0;
}

int test_sha1(char* data) {
    unsigned char ch = '1';
    char sha1[41];
    hv_sha1_hex(&ch, 1, sha1, sizeof(sha1));
    assert(strcmp(sha1, "356a192b7913b04c54574d18c28d46e6395428ab") == 0);

    const char* flags = "-s";
    if (flags[0] == '-' && flags[1] == 's') {
        hv_sha1_hex((unsigned char*)data, strlen(data), sha1, sizeof(sha1));
    }
    else {
        printf("Unrecognized flags '%s'\n", flags);
        return -40;
    }

    puts(sha1);

    return 0;
}

int test_socketpair() {
    int sockfds[2];
    if (Socketpair(AF_INET, SOCK_STREAM, 0, sockfds) != 0) {
        printf("socketpair failed!\n");
        return -1;
    }
    printf("Socketpair %d<=>%d\n", sockfds[0], sockfds[1]);

    char sendbuf[] = "hello,world!";
    char recvbuf[1460];
    int nsend = send(sockfds[0], sendbuf, strlen(sendbuf), 0);
    printf("sockfd:%d send %d bytes: %s\n", sockfds[0], nsend, sendbuf);
    memset(recvbuf, 0, sizeof(recvbuf));
    int nrecv = recv(sockfds[1], recvbuf, sizeof(recvbuf), 0);
    printf("sockfd:%d recv %d bytes: %s\n", sockfds[1], nrecv, recvbuf);

    closesocket(sockfds[0]);
    closesocket(sockfds[1]);
    return 0;
}
