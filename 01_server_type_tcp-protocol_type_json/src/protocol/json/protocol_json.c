#include <stdio.h>
#include <string.h>
#include "vlog.h"
#include "cJSON.h"

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
        memcpy(p_data_out, pData, strlen(pData));
        *p_len = strlen(pData);
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


