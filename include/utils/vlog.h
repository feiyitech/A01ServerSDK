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
 * @Description: 本文件实现了VLOG和FLOG, 方便打印log
 * @Author:  jicanmeng
 * @email:   jicanmeng@163.com
 * @version: v0.1 2023.04.28
 */

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