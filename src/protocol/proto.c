#include <assert.h>
#include <string.h>

#include "protocol.h"
#include "transport.h"
#include "serialize.h"

int atc_connect(struct atc_conn * conn)
{
    return transport_connect(&conn->addr);
}


int atc_listen(struct atc_conn * conn)
{
    return transport_listen(&conn->addr);
}


int atc_close(struct atc_conn * conn)
{
    return transport_close(&conn->addr);
}


int atc_request(struct atc_conn * conn)
{
    unsigned char send_buffer[ATCP_MSG_LEN];
    unsigned char recv_buffer[ATCP_MSG_LEN];
    int response;

    atc_req_to_wire(&conn->req, send_buffer);
    response = transport_send(&conn->addr, send_buffer, sizeof(send_buffer));
    if (response == -1) {
        return -1;
    }
    response = transport_recv(&conn->addr, recv_buffer, sizeof(recv_buffer));
    atc_wire_to_res(&conn->res, recv_buffer);
    return 0;
}


int atc_reply(struct atc_conn * conn, atc_reply_handler handler)
{
    return 0;
}

