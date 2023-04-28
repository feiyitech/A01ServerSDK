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
 * @Description: 本文件实现了对飞奕科技的智能主机(当前是A01产品)的json数据解析的功能
 * @Author:  jicanmeng
 * @email:   jicanmeng@163.com
 * @version: v0.1  2023.04.28 可以解析智能数据上发的心跳数据和主机信息的数据
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "vlog.h"
#include "cJSON.h"
#include "protocol_json.h"

/* @brief 解析智能主机上发的心跳数据
 *
 * @param json_payload   :json数据体内容
 * @param p_general_info :解析后将数据填充到这个指针指向的结构体中

 * @retval 0  :解析成功
 * @retval -1 :解析失败
 * 智能主机返回的数据，举例说明如下。其中 body 是本函数要解析的内容。
{
    "sn": 1,
    "cmd": "online",
    "uuid": "fa0000014000012402201180001015134",
    "body": {
    }
}
 */
int parse_online(cJSON *json_payload, ST_GENERAL_READ_INFO *p_general_info)
{
    /* 经过测试, 心跳数据包中并没有包含auth这一项, 需要和同事确认一下 */
    // cJSON *json_01_rtc = cJSON_GetObjectItem(json_payload, "auth");
    // if((!json_01_rtc) || (json_01_rtc->type != cJSON_String))
    // {
    //     VLOG("get auth failed\n");
    //     return -1;
    // }

    return 0;
}

/* @brief 解析智能主机对 general_read 命令的回复数据
 *
 * @param json_payload   :json数据体内容
 * @param p_general_info :解析后将数据填充到这个指针指向的结构体中

 * @retval 0  :解析成功
 * @retval -1 :解析失败
 * 智能主机返回的数据，举例说明如下。其中 body 是本函数要解析的内容。
{
    "sn": 1,
    "cmd": "general_read",
    "uuid": "fa0000014000012402201180001015134",
    "body": {
        "rtc": {
            "date": "2021-02-03 18:00:00" // 智能主机时间
        },
        "ver": "19.202.0.0.27", // 智能主机版本号
        "dbt": 2500, // 智能主机数据库总容量大小(M)
        "dbu": 56, // 智能主机数据库已使用量(百分比)
        "auth": "0", // 智能主机授权状态，0：未授权；1：已授权
        "watch": 1, // 是否启动长时间运行监控，1：启动；0：未启动
        "watcht": 1000 // 超过多长时间(分钟)持续开机监控生效
    }
}
 */
int parse_general_read(cJSON *json_payload, ST_GENERAL_READ_INFO *p_general_info)
{
    cJSON *json_01_rtc = cJSON_GetObjectItem(json_payload, "rtc");
    if((!json_01_rtc) || (json_01_rtc->type != cJSON_Object))
    {
        VLOG("get rtc failed\n");
        return -1;
    }

    cJSON *json_01_rtc_02_date = cJSON_GetObjectItem(json_01_rtc, "date");
    if((!json_01_rtc_02_date) || (json_01_rtc_02_date->type != cJSON_String))
    {
        VLOG("get date failed\n");
        return -1;
    }
    snprintf(p_general_info->date, sizeof(p_general_info->date),
        "%s", json_01_rtc_02_date->valuestring);

    cJSON *json_01_ver = cJSON_GetObjectItem(json_payload, "ver");
    if((!json_01_ver) || (json_01_ver->type != cJSON_String))
    {
        VLOG("get ver failed\n");
        return -1;
    }
    snprintf(p_general_info->version, sizeof(p_general_info->version),
        "%s", json_01_ver->valuestring);

    cJSON *json_01_dbt = cJSON_GetObjectItem(json_payload, "dbt");
    if((!json_01_dbt) || (json_01_dbt->type != cJSON_Number))
    {
        VLOG("get dbt failed\n");
        return -1;
    }
    p_general_info->database_capacity = json_01_dbt->valueint;

    cJSON *json_01_dbu = cJSON_GetObjectItem(json_payload, "dbu");
    if((!json_01_dbu) || (json_01_dbu->type != cJSON_Number))
    {
        VLOG("get dbu failed\n");
        return -1;
    }
    p_general_info->database_usage = json_01_dbu->valueint;

    /* 这里和协议不一致, 需要和同事确认一下. 协议文档中auth是字符串型 */
    cJSON *json_01_auth = cJSON_GetObjectItem(json_payload, "auth");
    if((!json_01_auth) || (json_01_auth->type != cJSON_Number))
    {
        VLOG("get auth failed\n");
        return -1;
    }
    p_general_info->auth = json_01_auth->valueint;

    cJSON *json_01_watch = cJSON_GetObjectItem(json_payload, "watch");
    if((!json_01_watch) || (json_01_watch->type != cJSON_Number))
    {
        VLOG("get watch failed\n");
        return -1;
    }
    p_general_info->watch = json_01_watch->valueint;

    cJSON *json_01_watch_time = cJSON_GetObjectItem(json_payload, "watcht");
    if((!json_01_watch_time) || (json_01_watch_time->type != cJSON_Number))
    {
        VLOG("get watch_time failed\n");
        return -1;
    }
    p_general_info->watch_time = json_01_watch_time->valueint;

    return 0;
}


/* @brief 解析智能主机上发的数据，根据数据中不同的命令调用不同的子函数进行解析
 *
 * @param p_data_in   :智能主机上发的数据起始地址
 * @param size_in     :智能主机上发的数据大小
 * @param p_type_out  :解析数据后，将数据类型放到 p_type_out 指向的变量中
 *                                数据类型有：TYPE_HEART_BEAT
 *                                           TYPE_PROPERTY_READ
 *                                           TYPE_STATUS_READ
 *                                           TYPE_GENERAL_READ
 * @param p_data_out  :解析数据后，将数据放到 p_data_out指向的内存中
 *
 * @retval 0  :解析成功
 * @retval -1 :解析失败
 *
 * 注意: 此函数的参数中, p_data_in 和 size_in 是输入参数，p_type_out 和 p_data_out 是输出参数. 调用此函数
 *       后, 要根据 *p_type_out 类型, 通过不同的结构体指针来读取 p_data_out 中的内容. 可参考示例程序
 *       example/main.c.
 */
int protocol_json_parse(uint8_t *p_data_in, uint16_t size_in, uint8_t *p_type_out, void *p_data_out)
{
    cJSON *json_root;
    cJSON *json_01_sn, *json_01_cmd, *json_01_uuid, *json_01_body;
    int ret = 0;

    /* 1. 检查数据头部是否有效：0xAA 0x55 和四个字节的json数据长度 */
    if((p_data_in[0] != 0xAA) || (p_data_in[1] != 0x55))
    {
        VLOG("error header: 0x%02x 0x%02x\n", p_data_in[0], p_data_in[1]);
        return -1;
    }

    int payload_len = (p_data_in[2] << 24) + (p_data_in[3] << 16) + (p_data_in[4] << 8) + p_data_in[5];

    if(payload_len + 6 != size_in)
    {
        VLOG("error length: %d %d\n", size_in, payload_len);
        return -1;
    }

    json_root = cJSON_Parse(p_data_in + 6);
    if (!json_root)
    {
        VLOG("json format error\n");
        return -1;
    }

    /* 注意, cJSON_print 返回的字符串指针指向的内存是通过动态内存分配的, 使用完成后应该释放内存. */
    char *p_data_print = cJSON_Print(json_root);
    if(p_data_print)
    {
        VLOG("json data:\n%s\n", p_data_print);
        cJSON_free(p_data_print);
    }

    /* 2. 检查必要字段：sn, uuid, cmd, body */
    json_01_sn = cJSON_GetObjectItem(json_root, "sn");
    if((!json_01_sn) || (json_01_sn->type != cJSON_Number))
    {
        VLOG("get sn failed\n");
        cJSON_Delete(json_root);
        return -1;
    }

    json_01_uuid = cJSON_GetObjectItem(json_root, "uuid");
    if((!json_01_uuid) || (json_01_uuid->type != cJSON_String))
    {
        VLOG("get uuid failed\n");
        cJSON_Delete(json_root);
        return -1;
    }

    json_01_cmd = cJSON_GetObjectItem(json_root, "cmd");
    if((!json_01_cmd) || (json_01_cmd->type != cJSON_String))
    {
        VLOG("get cmd failed\n");
        cJSON_Delete(json_root);
        return -1;
    }

    json_01_body = cJSON_GetObjectItem(json_root, "body");
    if((!json_01_body) || (json_01_body->type != cJSON_Object))
    {
        VLOG("get body failed\n");
        cJSON_Delete(json_root);
        return -1;
    }

    /* 3. 依据不同的 cmd 调用不同的函数进行解析 */
    if(0 == strcmp(json_01_cmd->valuestring, "general_read"))
    {
        *p_type_out = TYPE_GENERAL_READ;
        ST_GENERAL_READ_INFO *p_general_info = (ST_GENERAL_READ_INFO *)p_data_out;

        ret = parse_general_read(json_01_body, p_general_info);
        if(ret < 0)
        {
            VLOG("parse_general_read failed.\n");
            cJSON_Delete(json_root);
            return -1;
        }
    }
    else if(0 == strcmp(json_01_cmd->valuestring, "status_read"))
    {
        *p_type_out = TYPE_STATUS_READ;
        ST_AC_STATUS *p_ac_status = (ST_AC_STATUS *)p_data_out;
        // snprintf(p_ac_status->uuid, sizeof(p_ac_status->uuid), "%s", json_01_uuid->valuestring);
    }
    else if(0 == strcmp(json_01_cmd->valuestring, "online"))
    {
        *p_type_out = TYPE_HEART_BEAT;
        ST_GENERAL_READ_INFO *p_general_read_info = (ST_GENERAL_READ_INFO *)p_data_out;

        ret = parse_online(json_01_body, p_general_read_info);
        if(ret < 0)
        {
            VLOG("parse_online failed.\n");
            cJSON_Delete(json_root);
            return -1;
        }
        snprintf(p_general_read_info->uuid, sizeof(p_general_read_info->uuid), "%s", json_01_uuid->valuestring);
    }
    else
    {
        VLOG("current unsupported cmd: %s\n", json_01_cmd->valuestring);
        cJSON_Delete(json_root);
        return -1;
    }

    cJSON_Delete(json_root);
    return 0;
}

