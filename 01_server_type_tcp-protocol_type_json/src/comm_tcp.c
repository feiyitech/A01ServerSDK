#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include <pthread.h>
#include <errno.h>
#include "../include/vlog.h"
#include "comm_tcp.h"

static pthread_t server_comm_tcp_thread_id = 0;
static int serv_sock = 0;

struct DATA_FROM_CLIENT data_from_client[16];
static int write_index = 0;
static int read_index  = 0;



static int thread_flag;
static pthread_cond_t thread_flag_cv;
static pthread_mutex_t thread_flag_mutex;

#define FLAG_DATA_FROM_CLIENT  0x01

static void initialize_flag ()
{
  /* Initialize the mutex and condition variable.  */
  pthread_mutex_init (&thread_flag_mutex, NULL);
  pthread_cond_init (&thread_flag_cv, NULL);
  /* Initialize the flag value.  */
  thread_flag = 0;
}



static void *server_comm_tcp_thread(void *arg)
{
    int clnt_sock;
    struct sockaddr_in clnt_adr;
    socklen_t adr_sz;

    struct timeval timeout;
    fd_set reads, cpy_reads;
    int fd_max, fd_num;

    char buf[BUF_SIZE];
    int str_len = 0;
    int ret     = 0;

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
                        //data coming from client
                        memcpy(data_from_client[write_index].buf, buf, str_len);
                        data_from_client[write_index].size    = str_len;
                        data_from_client[write_index].fd      = fd_num;
                        snprintf(data_from_client[write_index].ip_addr, sizeof(data_from_client[write_index].ip_addr), "%s", inet_ntoa(clnt_adr.sin_addr));
                        write_index++;
                        if(write_index == 16)
                            write_index = 0;

                        pthread_mutex_lock (&thread_flag_mutex);
                        thread_flag |= FLAG_DATA_FROM_CLIENT;
                        if(write_index == 0) {
                            read_index = 15;
                        } else {
                            read_index = write_index - 1;
                        }
                        pthread_cond_signal (&thread_flag_cv);
                        pthread_mutex_unlock (&thread_flag_mutex);
                    }
                }
            }
        }
    }
}

int server_tcp_get_data(struct DATA_FROM_CLIENT *p_data)
{
    int data_available = 0;

	while(1) {
		pthread_mutex_lock (&thread_flag_mutex);
		while (!thread_flag) {
		  pthread_cond_wait (&thread_flag_cv, &thread_flag_mutex);
		}

		if(thread_flag & FLAG_DATA_FROM_CLIENT) {
			thread_flag &= ~FLAG_DATA_FROM_CLIENT;

            p_data->fd   = data_from_client[read_index].fd;
            p_data->size = data_from_client[read_index].size;
            memcpy(p_data->buf, data_from_client[read_index].buf, BUF_SIZE);
            memcpy(p_data->ip_addr, data_from_client[read_index].ip_addr, sizeof(data_from_client[read_index].ip_addr));

            pthread_mutex_unlock (&thread_flag_mutex);
            break;
		}
		pthread_mutex_unlock (&thread_flag_mutex);
	}

    return 0;
}


int server_tcp_start(uint16_t port)
{
    struct sockaddr_in serv_adr;
    int option;
    int str_len;
    int ret = 0;

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
    serv_adr.sin_port = htons(port);

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

    ret = pthread_create (&server_comm_tcp_thread_id, NULL, &server_comm_tcp_thread, NULL);
    if(ret) {
        printf("create server_comm_tcp thread failed: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

int server_tcp_stop()
{
    close(serv_sock);
    return 0;
}

