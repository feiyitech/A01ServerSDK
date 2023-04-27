#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "comm_tcp.h"
#include "../include/vlog.h"
#include "cJSON.h"

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

        uint8_t *p_data = m_client_data.buf;
        if((p_data[0] != 0xAA) || (p_data[1] != 0x55))
        {
            VLOG("error header: 0x%02x 0x%02x\n", p_data[0], p_data[1]);
            return;
        }

        int payload_len = (p_data[2] << 24) + (p_data[3] << 16) + (p_data[4] << 8) + p_data[5];

        if(payload_len + 6 != m_client_data.size)
        {
            VLOG("error length: %d %d\n", m_client_data.size, payload_len);
            return;
        }

        cJSON *json_root;
        cJSON *json_01_sn, *json_01_cmd, *json_01_uuid;
        cJSON *json_01_01_namespace, *json_01_02_name, *json_01_03_messageId, *json_01_04_payloadVersion;
        int ret = 0;

        json_root = cJSON_Parse(p_data + 6);
        if (!json_root)
        {
            VLOG("json format error\n");
            return;
        }

        /* 注意, cJSON_print 返回的字符串指针指向的内存是通过动态内存分配的, 使用完成后应该释放内存. */
        char *p_data_print = cJSON_Print(json_root);
        if(p_data_print)
        {
            VLOG("json data:\n%s\n", p_data_print);
            free(p_data_print);
        }

        json_01_sn = cJSON_GetObjectItem(json_root, "sn");
        if((!json_01_sn) || (json_01_sn->type != cJSON_Number))
        {
            VLOG("get sn failed\n");
            cJSON_Delete(json_root);
            return;
        }

        cJSON_Delete(json_root);
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

