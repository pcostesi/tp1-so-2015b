#ifndef __ATCP_SERIAL
#define __ATCP_SERIAL 1

#include "atcd.h"

union _atcp_msg {
	struct atc_plane plane;
	struct atc_airport airport;
	struct {
        enum atc_req_type type;
        char id[ATCD_ID_LENGTH];
	} req;
};

#define ATCP_MSG_LEN (sizeof(union _atcp_msg))

void atc_plane_to_wire(struct atc_plane * plane, unsigned char wire[ATCP_MSG_LEN]);
void atc_wire_to_plane(struct atc_plane * plane, unsigned char wire[ATCP_MSG_LEN]);

void atc_airport_to_wire(struct atc_airport * airport, unsigned char wire[ATCP_MSG_LEN]);
void atc_wire_to_airport(struct atc_airport * airport, unsigned char wire[ATCP_MSG_LEN]);

#endif

