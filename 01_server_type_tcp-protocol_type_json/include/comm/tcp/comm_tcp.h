#ifndef __COMM_TCP_H
#define __COMM_TCP_h

#include <stdint.h>

#ifdef  __cplusplus
extern "C"
{
#endif

/* 每次能接收的最大字节数 */
#define BUF_SIZE 4096

/* 客户端最多连接16个 */
#define MAX_CLIENT_NUM  16

#define DATA_TYPE_CONNECT    0x01
#define DATA_TYPE_DISCONNECT 0x02
#define DATA_TYPE_GET_DATA   0x03

struct DATA_FROM_CLIENT {
    char     ip_addr[16];
    int      fd;
    uint16_t size;
    uint8_t  buf[BUF_SIZE];
    uint8_t  type;
};


int server_tcp_start(uint16_t port, void (*funcPtr)(struct DATA_FROM_CLIENT *p_client_data));
int server_tcp_stop();
int server_tcp_write(int fd, char *buffer, int size);

#ifdef  __cplusplus
}
#endif
#endif
