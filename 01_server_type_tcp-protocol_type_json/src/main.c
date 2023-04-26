#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "comm_tcp.h"
#include "../include/vlog.h"

struct DATA_FROM_CLIENT m_client_data;

void get_data_cb(struct DATA_FROM_CLIENT *p_client_data)
{
    if(p_client_data->type == DATA_TYPE_CONNECT)
    {
        VLOG("client connect: %s:%d\n", p_client_data->ip_addr, p_client_data->fd);
    }
    else if(p_client_data->type == DATA_TYPE_DISCONNECT)
    {
        VLOG("client disconnect: %s:%d\n", p_client_data->ip_addr, p_client_data->fd);
    }
    else if(p_client_data->type == DATA_TYPE_GET_DATA)
    {
        memcpy(&m_client_data, p_client_data, sizeof(m_client_data));
        VLOG("client data: %s:%d: size %d: %02x %02x %02x %02x %02x %02x %02x %02x\n",
            m_client_data.ip_addr, m_client_data.fd, m_client_data.size,
            m_client_data.buf[0],
            m_client_data.buf[1],
            m_client_data.buf[2],
            m_client_data.buf[3],
            m_client_data.buf[4],
            m_client_data.buf[5],
            m_client_data.buf[6],
            m_client_data.buf[7]);
    }
    else
    {
        VLOG("invalid type for getting data from client: %d\n", p_client_data->type);
    }
}

int main(int argc, char *argv[])
{
    int ret = 0;

    ret = server_tcp_start(5566, get_data_cb);
    if(ret < 0)
    {
        VLOG("server_tcp_start failed.\n");
        return -1;
    }

    while(1)
    {
        usleep(1000000);
    }

    return 0;
}

