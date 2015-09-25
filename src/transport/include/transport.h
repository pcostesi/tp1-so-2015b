#ifndef __TRANSPORT_CONN
#define __TRANSPORT_CONN 1

#include <stddef.h>

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
        struct sockaddr socket;
        void * shmem;
    } conn;
};

#endif


int transport_send(struct transport_addr * addr, unsigned char * buffer, size_t size);
int transport_recv(struct transport_addr * addr, unsigned char * buffer, size_t size);

int transport_connect(struct transport_addr * addr);
int transport_listen(struct transport_addr * addr);
int transport_close(struct transport_addr * addr);