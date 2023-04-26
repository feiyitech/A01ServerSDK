#ifndef __COMM_TCP_H
#define __COMM_TCP_h

#include <stdint.h>

#define BUF_SIZE 4096

struct DATA_FROM_CLIENT {
    char     ip_addr[16];
    int      fd;
    uint16_t size;
    uint8_t  buf[BUF_SIZE];
};

int server_tcp_start(uint16_t port, int(*funcPtr)(struct DATA_FROM_CLIENT *p_client_data));
int server_tcp_stop();

#endif
