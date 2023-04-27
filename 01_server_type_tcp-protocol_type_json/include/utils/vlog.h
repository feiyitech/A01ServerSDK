#ifndef _V_LOG_H
#define _V_LOG_H

#include <time.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/timeb.h>

#define filename(x) (strrchr(x, '/') ? strrchr(x, '/') + 1 : x)

/* add timestamp, file name and line number. */
#define VLOG(format, ...)                                                              \
    do                                                                                 \
    {                                                                                  \
        struct timespec start;                                                         \
        clock_gettime(CLOCK_REALTIME, &start);                                         \
        struct tm ptm;                                                                 \
        localtime_r(&start.tv_sec, &ptm);                                              \
        fprintf(stderr, ("<%04d-%02d-%02d %02d:%02d:%02d:%03ld> %s:%s:%05d: " format), \
                ptm.tm_year + 1900, ptm.tm_mon + 1, ptm.tm_mday,                       \
                ptm.tm_hour, ptm.tm_min, ptm.tm_sec, start.tv_nsec / 1000000,          \
                filename(__FILE__),                                                    \
                __FUNCTION__,                                                          \
                __LINE__,                                                              \
                ##__VA_ARGS__);                                                        \
    } while (0)

/* save log to a file */
#define FLOG(format, ...)                                                                                   \
    do                                                                                                      \
    {                                                                                                       \
        char buffer[1024];                                                                                  \
        FILE *fp = NULL;                                                                                    \
        int ret = 0;                                                                                        \
        struct timespec start;                                                                              \
        clock_gettime(CLOCK_REALTIME, &start);                                                              \
        struct tm ptm;                                                                                      \
        localtime_r(&start.tv_sec, &ptm);                                                                   \
        fp = fopen("test.log", "ab+");                                                                      \
        if (fp == NULL)                                                                                     \
        {                                                                                                   \
            fprintf(stderr, "%s:%d: fopen failed:%s\n", __func__, __LINE__, strerror(errno));               \
        }                                                                                                   \
        else                                                                                                \
        {                                                                                                   \
            snprintf(buffer, sizeof(buffer), ("<%04d-%02d-%02d %02d:%02d:%02d:%03ld> %s:%s:%05d: " format), \
                     ptm.tm_year + 1900, ptm.tm_mon + 1, ptm.tm_mday,                                       \
                     ptm.tm_hour, ptm.tm_min, ptm.tm_sec, start.tv_nsec / 1000000,                          \
                     filename(__FILE__),                                                                    \
                     __FUNCTION__, __LINE__, ##__VA_ARGS__);                                                \
            ret = fwrite(buffer, strlen(buffer), 1, fp);                                                    \
            if (ret != 1)                                                                                   \
            {                                                                                               \
                fprintf(stderr, "%s:%d: fwrite failed:%s\n", __func__, __LINE__, strerror(errno));          \
            }                                                                                               \
            fflush(fp);                                                                                     \
            fclose(fp);                                                                                     \
        }                                                                                                   \
    } while (0)

#endif