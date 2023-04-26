#include <stdio.h>
#include <unistd.h>
#include "comm_tcp.h"
#include "../include/vlog.h"

#define BUF_SIZE 4096

int main(int argc, char *argv[])
{
    int ret = 0;
    struct DATA_FROM_CLIENT client_data;

    ret = server_tcp_start(5566);
    if(ret < 0)
    {
        VLOG("server_tcp_start failed.\n");
        return -1;
    }

    while(1)
    {
        server_tcp_get_data(&client_data);
        VLOG("client data: %s:%d: size %d: %02x %02x %02x %02x %02x %02x %02x %02x\n",
            client_data.ip_addr, client_data.fd, client_data.size,
            client_data.buf[0],
            client_data.buf[1],
            client_data.buf[2],
            client_data.buf[3],
            client_data.buf[4],
            client_data.buf[5],
            client_data.buf[6],
            client_data.buf[7]);
    }

    return 0;
}

