#ifndef __ATCP_SERIAL
#define __ATCP_SERIAL 1

#include "atcd.h"
#include <stddef.h>

union _atcp_msg {
    struct atc_plane plane;
    struct atc_airport airport;
    struct {
        enum atc_req_type type;
        char id[ATCD_ID_LENGTH];
        struct atc_plane plane;
    } req;
};

#define ATCP_MSG_LEN (sizeof(union _atcp_msg))
#define ATCP_HEADER_LEN (sizeof(uint32_t) * 2)

unsigned char * atc_plane_to_wire(struct atc_plane * plane, unsigned char wire[ATCP_MSG_LEN]);
unsigned char * atc_wire_to_plane(struct atc_plane * plane, unsigned char wire[ATCP_MSG_LEN]);

unsigned char * atc_airport_to_wire(struct atc_airport * airport, unsigned char wire[ATCP_MSG_LEN]);
unsigned char * atc_wire_to_airport(struct atc_airport * airport, unsigned char wire[ATCP_MSG_LEN]);


unsigned char * atc_req_to_wire(struct atc_req * req, unsigned char wire[ATCP_MSG_LEN]);
unsigned char * atc_wire_to_req(struct atc_req * req, unsigned char wire[ATCP_MSG_LEN]);

int atc_header_to_res(struct atc_res * res, unsigned char wire[ATCP_HEADER_LEN]);
int atc_res_to_header(struct atc_res * res, unsigned char wire[ATCP_HEADER_LEN]);

#endif
