#include <assert.h>
#include <string.h>

#include "protocol.h"
#include "serialize.h"


int atc_send(struct atc_addr * addr, struct atc_req * req, struct atc_res * res)
{
    /* msg -> wire */
    return atc_sendbytes(addr, NULL, 0);
}

int atc_recv(struct atc_addr * addr, struct atc_req * req, struct atc_res * res)
{
    /* wire -> msg */
    return atc_recvbytes(addr, NULL, 0);
}


