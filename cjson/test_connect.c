#include "hsocket.h"
#include "htime.h"

int test_connect(int port) {
    const char ip[16] = "127.0.0.1";

    unsigned int start_time = gettick();
    int ret = ConnectNonblock(ip, port);
    unsigned int end_time = gettick();
    printf("ConnectNonblock[%s:%d] retval=%d cost=%ums\n", ip, port, ret, end_time - start_time);

    start_time = gettick();
    ret = ConnectTimeout(ip, port, 100);
    end_time = gettick();
    printf("ConnectTimeout[%s:%d] retval=%d cost=%ums\n", ip, port, ret, end_time - start_time);

    start_time = gettick();
    ret = Connect(ip, port, 0);
    end_time = gettick();
    printf("ConnectBlock[%s:%d] retval=%d cost=%ums\n", ip, port, ret, end_time - start_time);

    return 0;
}
