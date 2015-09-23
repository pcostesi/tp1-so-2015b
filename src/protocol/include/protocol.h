#ifndef _ATC_PROTO
#define _ATC_PROTO 1

#include "atcd.h"

enum atc_req_type {
    atc_speed_up,
    atc_speed_down,
    atc_turn_left,
    atc_turn_right,
    atc_ascend,
    atc_descend,
    atc_get_time,
    atc_get_planes,
    atc_get_airports,
    atc_join,
    atc_leave,
    atc_get_landed,
    atc_get_crashed,
    atc_create_plane
};

enum atc_res_type {
    atc_ack,
    atc_err,
    atc_planes,
    atc_airports,
    atc_time
};


enum atc_conn_type {
    atc_conn_socket,
    atc_conn_fifo,
    atc_conn_shmem,
    atc_conn_file,
    atc_conn_queue
};


struct atc_req {
    enum atc_req_type type;
    char id[ATCD_ID_LENGTH];
};


struct atc_res {
    enum atc_res_type type;
    union {
        unsigned int airports;
        unsigned int planes;
        int error_code;
    } len;
    union {
        struct atc_plane planes[MAX_PLANES];
        struct atc_plane airports[MAX_AIRPORTS];
    } msg;
};

struct atc_addr {
    enum atc_conn_type type;
    union {
        struct sockaddr socket;
        void * shmem;
    } conn;
};

/* TODO: Add handshake, handoff, sendmsg, receivemsg */

int atc_handshake(struct atc_addr * addr);
int atc_handoff(struct atc_addr * addr);

#endif
