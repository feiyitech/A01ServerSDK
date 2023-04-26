#include <stdio.h>
#include <unistd.h>
#include "comm_tcp.h"
#include "../include/vlog.h"

#define BUF_SIZE 4096

int main(int argc, char *argv[])
{
    int ret = 0;

    ret = start_server_tcp(5566);
    if(ret < 0)
    {
        VLOG("start_server_tcp failed.\n");
        return -1;
    }

    while(1)
    {
        usleep(1000000);
    }

    return 0;
}

