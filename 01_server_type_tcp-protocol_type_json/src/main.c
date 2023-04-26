#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include <errno.h>
#include "../include/vlog.h"

#define BUF_SIZE 4096

int main(int argc, char *argv[])
{
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_adr, clnt_adr;
    struct timeval timeout;
    fd_set reads, cpy_reads;
    int ret = 0;
    int option;

    socklen_t adr_sz;
    int fd_max, str_len, fd_num;
    char buf[BUF_SIZE];
    if (argc != 2)
    {
        VLOG("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (ret < 0)
    {
        VLOG("create socket failed: %s\n", strerror(errno));
        return -1;
    }

    option = 1;
    ret = setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
    if (ret < 0)
    {
        VLOG("setsockopt failed: %s\n", strerror(errno));
        return -1;
    }

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    ret = bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr));
    if (ret < 0)
    {
        VLOG("bind failed: %s\n", strerror(errno));
        return -1;
    }

    ret = listen(serv_sock, 5);
    if (ret < 0)
    {
        VLOG("listen failed: %s\n", strerror(errno));
        return -1;
    }

    FD_ZERO(&reads);
    FD_SET(serv_sock, &reads);
    fd_max = serv_sock;

    while (1)
    {
        cpy_reads = reads;
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        ret = select(fd_max + 1, &cpy_reads, 0, 0, &timeout);
        if(ret < 0)
        {
            VLOG("select failed: %s\n", strerror(errno));
            break;
        }
        else if(ret == 0)
        {
            VLOG("select timed out...\n");
            continue;
        }

        for (fd_num = 0; fd_num < fd_max + 1; fd_num++)
        {
            if (FD_ISSET(fd_num, &cpy_reads))
            {
                if (fd_num == serv_sock)                // connection request!
                {
                    adr_sz = sizeof(clnt_adr);
                    clnt_sock =
                        accept(serv_sock, (struct sockaddr *)&clnt_adr, &adr_sz);
                    FD_SET(clnt_sock, &reads);
                    if (fd_max < clnt_sock)
                        fd_max = clnt_sock;
                    VLOG("connected client %d: %s:%d\n", clnt_sock,
                        inet_ntoa(clnt_adr.sin_addr), ntohs(clnt_adr.sin_port));
                }
                else                                    // read message!
                {
                    str_len = read(fd_num, buf, BUF_SIZE);
                    if (str_len == 0)                   // close request!
                    {
                        FD_CLR(fd_num, &reads);
                        close(fd_num);
                        VLOG("closed client: %d \n", fd_num);
                    }
                    else
                    {
                        VLOG("length: %d, data: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n", str_len, buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
                    }
                }
            }
        }
    }
    close(serv_sock);
    return 0;
}

