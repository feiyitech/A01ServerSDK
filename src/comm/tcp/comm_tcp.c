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
 * @Description: 本文件实现了对飞奕科技的智能主机(当前是A01产品)进行tcp通信的功能
 * @Author:  jicanmeng
 * @email:   jicanmeng@163.com
 * @version: v0.1  2023.04.28 可以读取到智能主机上发的数据, 也可以下发数据到智能主机
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include <pthread.h>
#include <errno.h>
#include "utils/vlog.h"
#include "comm/tcp/comm_tcp.h"


static void (*pfn_get_data)(struct DATA_FROM_CLIENT *p_client_data);

static pthread_t server_comm_tcp_thread_id = 0;
static int flag_server_stop    = 0;

static void *server_comm_tcp_thread(void *arg)
{
    int serv_sock = *(int *)arg;
    int clnt_sock;
    struct sockaddr_in clnt_adr;
    socklen_t adr_sz;

    struct timeval timeout;
    fd_set reads, cpy_reads;
    int fd_max, fd_num;

    int str_len = 0;
    int ret     = 0;
    struct DATA_FROM_CLIENT data_from_client;
    int client_number_connected = 0;

    /* ip地址形式为 192.168.133.123，最长占15个字节, 加上结尾符0x00，所以定义ip_addr大小为16个字节 */
    struct { int connected; int fd; char ip_addr[16]; } client_info[MAX_CLIENT_NUM];
    memset(client_info, 0x00, sizeof(client_info));

    FD_ZERO(&reads);
    FD_SET(serv_sock, &reads);
    fd_max = serv_sock;

    while (1)
    {
        cpy_reads = reads;
        timeout.tv_sec = 3;
        timeout.tv_usec = 0;

        if(flag_server_stop == 1)
        {
            for(int i = 0; i < MAX_CLIENT_NUM; i++)
            if(client_info[i].connected)
            {
                close(client_info[i].fd);
            }
            close(serv_sock);
            break;
        }

        ret = select(fd_max + 1, &cpy_reads, 0, 0, &timeout);
        if(ret < 0)
        {
            VLOG("select failed: %s\n", strerror(errno));
            break;
        }
        else if(ret == 0)
        {
            // VLOG("select timed out...\n");
            continue;
        }

        for (fd_num = 0; fd_num < fd_max + 1; fd_num++)
        {
            if (FD_ISSET(fd_num, &cpy_reads))
            {
                if (fd_num == serv_sock)                // connection request!
                {
                    adr_sz = sizeof(clnt_adr);
                    clnt_sock =
                        accept(serv_sock, (struct sockaddr *)&clnt_adr, &adr_sz);

                    // VLOG("connected client %d: %s:%d\n", clnt_sock,
                    //     inet_ntoa(clnt_adr.sin_addr), ntohs(clnt_adr.sin_port));
                    if(client_number_connected >= MAX_CLIENT_NUM)
                    {
                        VLOG("client_number_connected is already %d, disconnect.\n", client_number_connected);
                        close(clnt_sock);
                        continue;
                    }

                    FD_SET(clnt_sock, &reads);
                    client_number_connected++;
                    if (fd_max < clnt_sock)
                        fd_max = clnt_sock;

                    for(int i = 0; i < MAX_CLIENT_NUM; i++)
                    {
                        if(client_info[i].connected == 0)
                        {
                            client_info[i].fd = clnt_sock;
                            snprintf(client_info[i].ip_addr, sizeof(client_info[i].ip_addr),
                                "%s", inet_ntoa(clnt_adr.sin_addr));

                            data_from_client.type    = DATA_TYPE_CONNECT;
                            data_from_client.fd      = clnt_sock;
                            snprintf(data_from_client.ip_addr, sizeof(data_from_client.ip_addr),
                                "%s", inet_ntoa(clnt_adr.sin_addr));
                            if(pfn_get_data)
                                pfn_get_data(&data_from_client);

                            client_info[i].connected = 1;
                            break;
                        }
                    }
                }
                else                                    // read message!
                {
                    str_len = read(fd_num, data_from_client.buf, BUF_SIZE);
                    if (str_len == 0)                   // close request!
                    {
                        FD_CLR(fd_num, &reads);
                        close(fd_num);
                        client_number_connected--;
                        // VLOG("closed client: %d \n", fd_num);

                        for(int i = 0; i < MAX_CLIENT_NUM; i++)
                        {
                            if(client_info[i].fd == fd_num)
                            {
                                data_from_client.type    = DATA_TYPE_DISCONNECT;
                                data_from_client.fd      = fd_num;
                                snprintf(data_from_client.ip_addr, sizeof(data_from_client.ip_addr),
                                    "%s", client_info[i].ip_addr);
                                if(pfn_get_data)
                                    pfn_get_data(&data_from_client);

                                client_info[i].connected = 0;
                                break;
                            }
                        }
                    }
                    else
                    {
                        //data coming from client
                        data_from_client.type    = DATA_TYPE_GET_DATA;
                        data_from_client.size    = str_len;
                        data_from_client.fd      = fd_num;
                        memset(data_from_client.ip_addr, 0x00, sizeof(data_from_client.ip_addr));
                        for(int i = 0; i < MAX_CLIENT_NUM; i++)
                        {
                            if((client_info[i].fd == fd_num) && (client_info[i].connected == 1))
                            {
                                snprintf(data_from_client.ip_addr, sizeof(data_from_client.ip_addr),
                                    "%s", client_info[i].ip_addr);
                                break;
                            }
                        }

                        if(pfn_get_data)
                            pfn_get_data(&data_from_client);
                    }
                }
            }
        }
    }
}

int server_tcp_write(int fd, char *buffer, int size)
{
    int ret           = 0;
    int written_bytes = 0;

    while(written_bytes < size)
    {
        ret = write(fd, buffer, size - written_bytes);
        if(ret < 0)
        {
            VLOG("write failed: %s\n", strerror(errno));
            return -1;
        }
        written_bytes += ret;
    }

    return 0;
}

int server_tcp_start(uint16_t port, void (*funcPtr)(struct DATA_FROM_CLIENT *p_client_data))
{
    int serv_sock = 0;
    struct sockaddr_in serv_adr;
    int option;
    int str_len;
    int ret = 0;

    pfn_get_data            = funcPtr;
    flag_server_stop        = 0;

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (ret < 0)
    {
        VLOG("create socket failed: %s\n", strerror(errno));
        return -1;
    }

    option = 1;
    ret = setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
    if (ret < 0)
    {
        VLOG("setsockopt failed: %s\n", strerror(errno));
        return -1;
    }

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(port);

    ret = bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr));
    if (ret < 0)
    {
        VLOG("bind failed: %s\n", strerror(errno));
        return -1;
    }

    ret = listen(serv_sock, 5);
    if (ret < 0)
    {
        VLOG("listen failed: %s\n", strerror(errno));
        return -1;
    }

    ret = pthread_create (&server_comm_tcp_thread_id, NULL, &server_comm_tcp_thread, &serv_sock);
    if(ret) {
        printf("create server_comm_tcp thread failed: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

int server_tcp_stop()
{
    flag_server_stop = 1;
    pthread_join(server_comm_tcp_thread_id, NULL);
    return 0;
}

