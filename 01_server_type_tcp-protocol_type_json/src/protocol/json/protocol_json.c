#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "vlog.h"
#include "cJSON.h"
#include "protocol_json.h"

int parse_online(cJSON *json_payload, ST_HOST_INFO *p_general_info)
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
    snprintf(p_general_info->host_info.date, sizeof(p_general_info->host_info.date),
        "%s", json_01_rtc_02_date->valuestring);

    cJSON *json_01_ver = cJSON_GetObjectItem(json_payload, "ver");
    if((!json_01_ver) || (json_01_ver->type != cJSON_String))
    {
        VLOG("get ver failed\n");
        return -1;
    }
    snprintf(p_general_info->host_info.version, sizeof(p_general_info->host_info.version),
        "%s", json_01_ver->valuestring);

    cJSON *json_01_dbt = cJSON_GetObjectItem(json_payload, "dbt");
    if((!json_01_dbt) || (json_01_dbt->type != cJSON_Number))
    {
        VLOG("get dbt failed\n");
        return -1;
    }
    p_general_info->host_info.database_capacity = json_01_dbt->valueint;

    cJSON *json_01_dbu = cJSON_GetObjectItem(json_payload, "dbu");
    if((!json_01_dbu) || (json_01_dbu->type != cJSON_Number))
    {
        VLOG("get dbu failed\n");
        return -1;
    }
    p_general_info->host_info.database_usage = json_01_dbu->valueint;

    /* 这里和协议不一致, 需要和同事确认一下. 协议文档中auth是字符串型 */
    cJSON *json_01_auth = cJSON_GetObjectItem(json_payload, "auth");
    if((!json_01_auth) || (json_01_auth->type != cJSON_Number))
    {
        VLOG("get auth failed\n");
        return -1;
    }
    p_general_info->host_info.auth = json_01_auth->valueint;

    cJSON *json_01_watch = cJSON_GetObjectItem(json_payload, "watch");
    if((!json_01_watch) || (json_01_watch->type != cJSON_Number))
    {
        VLOG("get watch failed\n");
        return -1;
    }
    p_general_info->host_info.watch = json_01_watch->valueint;

    cJSON *json_01_watch_time = cJSON_GetObjectItem(json_payload, "watcht");
    if((!json_01_watch_time) || (json_01_watch_time->type != cJSON_Number))
    {
        VLOG("get watch_time failed\n");
        return -1;
    }
    p_general_info->host_info.watch_time = json_01_watch_time->valueint;

    return 0;
}

int protocol_json_parse(uint8_t *p_data_in, uint16_t size_in, uint8_t *p_type_out, void *p_data_out)
{
    cJSON *json_root;
    cJSON *json_01_sn, *json_01_cmd, *json_01_uuid, *json_01_body;
    int ret = 0;

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
        ST_HOST_INFO *p_host_info = (ST_HOST_INFO *)p_data_out;

        ret = parse_online(json_01_body, p_host_info);
        if(ret < 0)
        {
            VLOG("parse_online failed.\n");
            cJSON_Delete(json_root);
            return -1;
        }
        snprintf(p_host_info->uuid, sizeof(p_host_info->uuid), "%s", json_01_uuid->valuestring);
    }

    cJSON_Delete(json_root);
    return 0;
}

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

