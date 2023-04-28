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
 * @Description: 本文件实现了给飞奕科技的智能主机(当前是A01产品)封装要发送的json数据的功能
 * @Author:  jicanmeng
 * @email:   jicanmeng@163.com
 * @version: v0.1  2023.04.28 实现了封装读取主机信息的命令和读取所有空调信息的命令
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "vlog.h"
#include "cJSON.h"
#include "protocol_json.h"

int construct_status_read(char *p_uuid, char *p_data_out, int *p_len)
{
    cJSON *json_root;
    cJSON *json_01_sn, *json_01_cmd, *json_01_uuid, *json_01_body;
    cJSON *json_01_body_02_cmd;
    char *pData = NULL;
    int ret = 0;

    json_root = cJSON_CreateObject();
    if (!json_root)
    {
        VLOG("json create object failed\n");
        return -1;
    }

    cJSON_AddNumberToObject(json_root, "sn", 1);

    json_01_cmd = cJSON_CreateString("status_read");
    if(!json_01_cmd)
    {
        VLOG("json create cmd failed\n");
        cJSON_Delete(json_root);
        return -1;
    }
    cJSON_AddItemToObject(json_root, "cmd", json_01_cmd);

    json_01_uuid = cJSON_CreateString(p_uuid);
    if(!json_01_uuid)
    {
        VLOG("json create uuid failed\n");
        cJSON_Delete(json_root);
        return -1;
    }
    cJSON_AddItemToObject(json_root, "uuid", json_01_uuid);

    json_01_body = cJSON_CreateObject();
    if (!json_01_body)
    {
        VLOG("json create body failed\n");
        cJSON_Delete(json_root);
        return -1;
    }
    cJSON_AddItemToObject(json_root, "body", json_01_body);

    json_01_body_02_cmd = cJSON_CreateString("all");
    if(!json_01_body_02_cmd)
    {
        VLOG("json create body-cmd failed\n");
        cJSON_Delete(json_root);
        return -1;
    }
    cJSON_AddItemToObject(json_01_body, "cmd", json_01_body_02_cmd);

    pData = cJSON_PrintUnformatted(json_root);
    if(pData)
    {
        // VLOG("json data:\n%s. memorycopy-length: %d\n", pData, (int)strlen(pData));

        uint32_t length = strlen(pData);
        p_data_out[0] = 0xAA;
        p_data_out[1] = 0x55;
        p_data_out[2] = length >> 24;
        p_data_out[3] = length >> 16;
        p_data_out[4] = length >> 8;
        p_data_out[5] = length;
        memcpy(p_data_out + 6, pData, length);

        *p_len = length + 6;

        cJSON_free(pData);
    }
    else
    {
        VLOG("cjson print function failed\n");
        cJSON_Delete(json_root);
        return -1;
    }

    cJSON_Delete(json_root);
    return 0;
}

int construct_general_read(char *p_uuid, char *p_data_out, int *p_len)
{
    cJSON *json_root;
    cJSON *json_01_sn, *json_01_cmd, *json_01_uuid, *json_01_body;
    cJSON *json_01_body_02_cmd;
    char *pData = NULL;
    int ret = 0;

    json_root = cJSON_CreateObject();
    if (!json_root)
    {
        VLOG("json create object failed\n");
        return -1;
    }

    cJSON_AddNumberToObject(json_root, "sn", 1);

    json_01_cmd = cJSON_CreateString("general_read");
    if(!json_01_cmd)
    {
        VLOG("json create cmd failed\n");
        cJSON_Delete(json_root);
        return -1;
    }
    cJSON_AddItemToObject(json_root, "cmd", json_01_cmd);

    json_01_uuid = cJSON_CreateString(p_uuid);
    if(!json_01_uuid)
    {
        VLOG("json create uuid failed\n");
        cJSON_Delete(json_root);
        return -1;
    }
    cJSON_AddItemToObject(json_root, "uuid", json_01_uuid);

    json_01_body = cJSON_CreateObject();
    if (!json_01_body)
    {
        VLOG("json create body failed\n");
        cJSON_Delete(json_root);
        return -1;
    }
    cJSON_AddItemToObject(json_root, "body", json_01_body);

    pData = cJSON_PrintUnformatted(json_root);
    if(pData)
    {
        // VLOG("json data:\n%s. memorycopy-length: %d\n", pData, (int)strlen(pData));

        uint32_t length = strlen(pData);
        p_data_out[0] = 0xAA;
        p_data_out[1] = 0x55;
        p_data_out[2] = length >> 24;
        p_data_out[3] = length >> 16;
        p_data_out[4] = length >> 8;
        p_data_out[5] = length;
        memcpy(p_data_out + 6, pData, length);

        *p_len = length + 6;

        cJSON_free(pData);
    }
    else
    {
        VLOG("cjson print function failed\n");
        cJSON_Delete(json_root);
        return -1;
    }

    cJSON_Delete(json_root);
    return 0;
}

