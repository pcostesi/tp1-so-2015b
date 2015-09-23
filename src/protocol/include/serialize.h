#ifndef __ATCP_SERIAL
#define __ATCP_SERIAL 1

#include "atcd.h"

#define ATCP_PLANE_MSG_LEN (ATCD_ID_LENGTH + 4 * sizeof(uint32_t) + 2 * sizeof(char))

void atc_plane_to_wire(struct atc_plane * plane, unsigned char wire[ATCP_PLANE_MSG_LEN]);
void atc_wire_to_plane(struct atc_plane * plane, unsigned char wire[ATCP_PLANE_MSG_LEN]);


#endif

