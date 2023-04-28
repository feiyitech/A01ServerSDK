/*
 * The MIT License (MIT)
 *
 * Copyright © 2023~2050 <qingdao feiyi tech>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
 * Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * @Description: 本文件实现了和飞奕科技的智能主机(当前是A01产品)进行简单交互的功能
 * @Author:  jicanmeng
 * @email:   jicanmeng@163.com
 * @version: v0.1  2023.04.28
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "comm_tcp.h"
#include "vlog.h"
#include "cJSON.h"
#include "protocol_json.h"

typedef struct ST_CLIENT_INFO
{
    int fd;
    uint8_t connected;
    char uuid[36];
    ST_AC_INFO ac_info[32]; // 当前代码中设定最多连接32个空调内机
} ST_CLIENT_INFO;

ST_CLIENT_INFO client_info[MAX_CLIENT_NUM]; // 服务器端最多连接 MAX_CLIENT_NUM 个智能主机, 当前设置为 16，在 comm_tcp.h 文件中定义

static struct DATA_FROM_CLIENT m_client_data;

void get_data_cb(struct DATA_FROM_CLIENT *p_client_data)
{
    int ret = 0;
    uint8_t type_data;
    uint8_t buffer_read_out[BUF_SIZE];

    if (p_client_data->type == DATA_TYPE_CONNECT)
    {
        VLOG("client connect: %s:%d\n", p_client_data->ip_addr, p_client_data->fd);

        int i = 0;
        for (i = 0; i < MAX_CLIENT_NUM; i++)
        {
            if ((client_info[i].fd == p_client_data->fd) && (client_info[i].connected == 1))
            {
                VLOG("error: fd %d already exist.\n", p_client_data->fd);
                break;
            }
        }
        if (i == MAX_CLIENT_NUM)
        {
            for (i = 0; i < MAX_CLIENT_NUM; i++)
            {
                if (client_info[i].connected == 0)
                {
                    client_info[i].connected = 1;
                    client_info[i].fd = p_client_data->fd;
                    VLOG("add fd %d to client_info[%d]\n", p_client_data->fd, i);
                    break;
                }
            }
        }
    }
    else if (p_client_data->type == DATA_TYPE_DISCONNECT)
    {
        VLOG("client disconnect: %s:%d\n", p_client_data->ip_addr, p_client_data->fd);

        int i = 0;
        for (i = 0; i < MAX_CLIENT_NUM; i++)
        {
            if ((client_info[i].fd == p_client_data->fd) && (client_info[i].connected == 1))
            {
                client_info[i].connected = 0;
                memset(client_info[i].uuid, 0x00, sizeof(client_info[i].uuid));
                client_info[i].fd = 0;
                VLOG("remove fd %d from client_info[%d]\n", p_client_data->fd, i);
                break;
            }
        }
    }
    else if (p_client_data->type == DATA_TYPE_GET_DATA)
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

        ret = protocol_json_parse(m_client_data.buf, m_client_data.size, &type_data, buffer_read_out);
        if (ret < 0)
        {
            VLOG("protocol_json_parse failed.\n");
        }
        else
        {
            switch (type_data)
            {
            case TYPE_STATUS_READ:
            {
                ST_AC_STATUS *p_ac_status = (ST_AC_STATUS *)buffer_read_out;
            }
            break;

            case TYPE_GENERAL_READ:
            {
                ST_GENERAL_READ_INFO *p_general_info = (ST_GENERAL_READ_INFO *)buffer_read_out;
                VLOG("general_info:\n"
                     "date: %s\n"
                     "ver:%s\n"
                     "database capacity: %d\n"
                     "database usage: %d\n"
                     "watch: %d\n"
                     "watch time: %d\n"
                     "auth: %d\n", p_general_info->date,
                     p_general_info->version,
                     p_general_info->database_capacity,
                     p_general_info->database_usage,
                     p_general_info->watch,
                     p_general_info->watch_time,
                     p_general_info->auth);
            }
            break;

            case TYPE_HEART_BEAT:
            {
                ST_GENERAL_READ_INFO *p_general_read_info = (ST_GENERAL_READ_INFO *)buffer_read_out;

                for (int i = 0; i < MAX_CLIENT_NUM; i++)
                {
                    if ((client_info[i].connected == 1) && (client_info[i].fd == p_client_data->fd))
                    {
                        if (0 != strcmp(client_info[i].uuid, p_general_read_info->uuid))
                        {
                            snprintf(client_info[i].uuid, sizeof(client_info[i].uuid), "%s", p_general_read_info->uuid);
                            VLOG("add uuid %s to client_info[%d]\n", p_general_read_info->uuid, i);
                        }
                        break;
                    }
                }
            }
            break;

            default:
                break;
            }
        }
    }
    else
    {
        VLOG("invalid type for getting data from client: %d\n", p_client_data->type);
    }
}

int main(int argc, char *argv[])
{
    int ret = 0;
    char send_buf[BUF_SIZE];
    int send_size = 0;

    memset(client_info, 0x00, sizeof(client_info));

    ret = server_tcp_start(5566, get_data_cb);
    if (ret < 0)
    {
        VLOG("server_tcp_start failed.\n");
        return -1;
    }

    while (1)
    {
        usleep(1000000 * 10);

        if ((client_info[0].connected == 1) && (client_info[0].fd != 0) && (0 != strcmp(client_info[0].uuid, "")))
        {
            ret = construct_general_read(client_info[0].uuid, send_buf, &send_size);
            if (ret < 0)
            {
                VLOG("construct_status_read failed.\n");
            }
            else
            {
                ret = server_tcp_write(client_info[0].fd, send_buf, send_size);
                if (ret < 0)
                {
                    VLOG("serve_tcp_write failed.\n");
                }
            }
        }
    }

    return 0;
}
