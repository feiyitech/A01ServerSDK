#ifndef __PROTOCOL_JSON_H
#define __PROTOCOL_JSON_H

#include <stdint.h>

#ifdef  __cplusplus
extern "C"
{
#endif

#define TYPE_STATUS_READ           0x01
#define TYPE_PROPERTY_READ         0x02

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

int construct_status_read(char *p_uuid, char *p_data_out, int *p_len);
int protocol_json_parse(uint8_t *p_data_in, uint16_t size_in, uint8_t *p_type_out, void *p_data_out);

#ifdef  __cplusplus
}
#endif
#endif
