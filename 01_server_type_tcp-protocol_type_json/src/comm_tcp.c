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
#include "../include/vlog.h"
#include "comm_tcp.h"


void (*pfn_get_data)(struct DATA_FROM_CLIENT *p_client_data);

#define MAX_CLIENT_NUM  16

static int serv_sock = 0;

static void *server_comm_tcp_thread(void *arg)
{
    int clnt_sock;
    struct sockaddr_in clnt_adr;
    socklen_t adr_sz;

    struct timeval timeout;
    fd_set reads, cpy_reads;
    int fd_max, fd_num;

    int str_len = 0;
    int ret     = 0;
    struct DATA_FROM_CLIENT data_from_client;

    /* ip地址形式为 192.168.133.123，最长占15个字节, 加上结尾符0x00，所以定义ip_addr大小为16个字节 */
    struct { int used; int fd; char ip_addr[16]; } client_info[MAX_CLIENT_NUM];

    FD_ZERO(&reads);
    FD_SET(serv_sock, &reads);
    fd_max = serv_sock;

    while (1)
    {
        cpy_reads = reads;
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        ret = select(fd_max + 1, &cpy_reads, 0, 0, &timeout);
        if(ret < 0)
        {
            VLOG("select failed: %s\n", strerror(errno));
            break;
        }
        else if(ret == 0)
        {
            VLOG("select timed out...\n");
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
                    FD_SET(clnt_sock, &reads);
                    if (fd_max < clnt_sock)
                        fd_max = clnt_sock;
                    // VLOG("connected client %d: %s:%d\n", clnt_sock,
                    //     inet_ntoa(clnt_adr.sin_addr), ntohs(clnt_adr.sin_port));

                    for(int i = 0; i < MAX_CLIENT_NUM; i++)
                    {
                        if(client_info[i].used == 0)
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

                            client_info[i].used = 1;
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

                                client_info[i].used = 0;
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
                            if((client_info[i].fd == fd_num) && (client_info[i].used == 1))
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

int server_tcp_start(uint16_t port, void (*funcPtr)(struct DATA_FROM_CLIENT *p_client_data))
{
    struct sockaddr_in serv_adr;
    int option;
    int str_len;
    int ret = 0;
    static pthread_t server_comm_tcp_thread_id = 0;

    pfn_get_data = funcPtr;

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

    ret = pthread_create (&server_comm_tcp_thread_id, NULL, &server_comm_tcp_thread, NULL);
    if(ret) {
        printf("create server_comm_tcp thread failed: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

int server_tcp_stop()
{
    close(serv_sock);
    return 0;
}

