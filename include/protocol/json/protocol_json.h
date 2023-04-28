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
 * @Description: 本文件是 protocol_set.c 和 protocol_get.c 对应的头文件
 * @Author:  jicanmeng
 * @email:   jicanmeng@163.com
 */

#ifndef __PROTOCOL_JSON_H
#define __PROTOCOL_JSON_H

#include <stdint.h>
#include <utils/cJSON.h>

#ifdef  __cplusplus
extern "C"
{
#endif

#define TYPE_STATUS_READ           0x01
#define TYPE_PROPERTY_READ         0x02
#define TYPE_HEART_BEAT            0x03
#define TYPE_GENERAL_READ          0x04

typedef struct ST_AC_INFO {
    uint8_t type;           // 0x01:氟机   0x02:水机
    uint8_t ou_addr;        // 外机地址
    uint8_t iu_addr;        // 内机地址
    uint8_t on_off;         // 开关
    uint8_t work_mode;      // 运行模式
    uint8_t temp_set;       // 设置温度
    uint8_t temp_air;       // 室内温度
    uint8_t fan_speed;      // 风速
    uint8_t fan_direction;  // 风向
} ST_AC_INFO;


typedef struct ST_AC_STATUS {
    char uuid[36];
    ST_AC_INFO ac_info[32];
} ST_AC_STATUS;

typedef struct ST_GENERAL_READ_INFO {
    char uuid[36];
    char date[32];
    char version[32];
    uint32_t database_capacity;
    uint8_t database_usage;
    uint8_t auth;
    uint8_t watch;
    uint8_t watch_time;
} ST_GENERAL_READ_INFO;

/* 给智能主机发送命令 */
int construct_general_read(char *p_uuid, char *p_data_out, int *p_len);
int construct_status_read(char *p_uuid, char *p_data_out, int *p_len);

/* 从智能主机读取数据 */
int parse_online(cJSON *json_payload, ST_GENERAL_READ_INFO *p_general_info);
int parse_general_read(cJSON *json_payload, ST_GENERAL_READ_INFO *p_general_info);
int protocol_json_parse(uint8_t *p_data_in, uint16_t size_in, uint8_t *p_type_out, void *p_data_out);

#ifdef  __cplusplus
}
#endif
#endif
