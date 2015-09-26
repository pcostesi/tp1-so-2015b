#ifndef _ATC_PROTO
#define _ATC_PROTO 1

#include <netinet/in.h>
#include "atcd.h"
#include "transport.h"

enum atc_req_type {
    atc_speed_up,
    atc_speed_down,
    atc_turn_left,
    atc_turn_right,
    atc_ascend,
    atc_descend,
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
    atc_planes,
    atc_airports
};

struct atc_req {
    enum atc_req_type type;
    struct atc_plane plane;
};


struct atc_res {
    enum atc_res_type type;
    union {
        unsigned int airports;
        unsigned int planes;
    } len;
    union {
        int return_code;
        struct atc_plane planes[MAX_PLANES];
        struct atc_airport airports[MAX_AIRPORTS];
    } msg;
};

struct atc_conn {
    struct transport_addr addr;
    struct atc_req req;
    struct atc_res res;
};


typedef int (*atc_reply_handler)(struct atc_conn * conn);

int atc_connect(struct atc_conn * conn);
int atc_listen(struct atc_conn * conn);
int atc_close(struct atc_conn * conn);

int atc_request(struct atc_conn * conn);
int atc_reply(struct atc_conn * conn, atc_reply_handler handler);

#endif
