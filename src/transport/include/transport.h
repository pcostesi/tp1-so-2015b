#ifndef __TRANSPORT_CONN
#define __TRANSPORT_CONN 1

#if defined(__APPLE__) && defined(__MACH__)
#define NOQUEUE 1
#endif

#include <stddef.h>
#include <sys/types.h>
#ifndef NOQUEUE
#include <mqueue.h>
#endif
#include <semaphore.h>

enum transport_conn_type {
    transport_conn_socket,
    transport_conn_fifo,
    transport_conn_shmem,
    transport_conn_file,
    transport_conn_queue
};

struct transport_addr {
    enum transport_conn_type type;
    union {
        int sockfd;
        int fifo_fd[2]; /*0 is in, 1 out*/
#ifndef NOQUEUE
        mqd_t mqueue[2];
#endif
        struct {
            int i_am_the_server;
            int i_am_the_listen;
            int connected;
            int fd;
            int port;
            char * zone;
            size_t rd;
            size_t wr;
            union {
                struct {
                    sem_t * available;
                    sem_t * free;
                } listen;
                struct {
                    sem_t * available_srv;
                    sem_t * available_cli;
                    sem_t * free_srv;
                    sem_t * free_cli;
                } connection;
            } locks;
        } shmem;
    } conn;
};

#endif


int transport_send(struct transport_addr * addr, unsigned char * buffer, size_t size);
int transport_recv(struct transport_addr * addr, unsigned char * buffer, size_t size);

int transport_connect(struct transport_addr * addr);
int transport_listen(struct transport_addr * lsten);
int transport_close(struct transport_addr * addr);
int transport_serv_init(struct transport_addr * addr);
int transport_accept(struct transport_addr * listen, struct transport_addr *accepts);
