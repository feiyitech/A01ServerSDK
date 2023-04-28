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
 * @Description: 本文件是 comm_tcp.c 对应的头文件
 * @Author:  jicanmeng
 * @email:   jicanmeng@163.com
 */

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
