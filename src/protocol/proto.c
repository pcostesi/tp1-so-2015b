#include <assert.h>
#include <string.h>

#include "protocol.h"
#include "transport.h"
#include "serialize.h"

static int _atc_read_planes(struct atc_conn * conn);
static int _atc_read_airports(struct atc_conn * conn);
static int _atc_read_response(struct atc_conn * conn);
static int _atc_write_response(struct atc_conn * conn);


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


static int _atc_read_planes(struct atc_conn * conn)
{
    int idx;
    int max_planes;
    unsigned char buffer[ATCP_MSG_LEN];

    max_planes = conn->res.len.planes;
    for (idx = 0; idx < max_planes; idx++) {
        if (transport_recv(&conn->addr, buffer, sizeof(buffer)) == -1) return -1;
        atc_wire_to_plane(conn->res.msg.planes + idx, buffer);
    }
    return max_planes;
}

static int _atc_read_airports(struct atc_conn * conn)
{
    int idx;
    int max_airports;
    unsigned char buffer[ATCP_MSG_LEN];

    max_airports = conn->res.len.airports;
    for (idx = 0; idx < max_airports; idx++) {
        if (transport_recv(&conn->addr, buffer, sizeof(buffer)) == -1) return -1;
        atc_wire_to_airport(conn->res.msg.airports + idx, buffer);
    }
    return max_airports;   
}

static int _atc_write_planes(struct atc_conn * conn)
{
    int idx;
    int max_planes;
    unsigned char buffer[ATCP_MSG_LEN];

    max_planes = conn->res.len.planes;
    for (idx = 0; idx < max_planes; idx++) {
        atc_plane_to_wire(conn->res.msg.planes + idx, buffer);
        if (transport_send(&conn->addr, buffer, sizeof(buffer)) == -1) return -1;
    }
    return max_planes;
}

static int _atc_write_airports(struct atc_conn * conn)
{
    int idx;
    int max_airports;
    unsigned char buffer[ATCP_MSG_LEN];

    max_airports = conn->res.len.airports;
    for (idx = 0; idx < max_airports; idx++) {
        atc_airport_to_wire(conn->res.msg.airports + idx, buffer);
        if (transport_send(&conn->addr, buffer, sizeof(buffer)) == -1) return -1;
    }
    return max_airports;   
}

static int _atc_read_response(struct atc_conn * conn)
{
    struct atc_res * res;
    struct transport_addr * addr;
    unsigned char buffer[ATCP_HEADER_LEN];
    int response;

    res = &conn->res;
    addr = &conn->addr;
    
    /* first we have to read the header */
    if (transport_recv(addr, buffer, sizeof(buffer)) == -1) return -1;
    if (atc_header_to_res(res, buffer) == -1) return -1;

    switch (res->type) {
        case atc_ack:
        return 0;
        break;

        case atc_planes:
        if ((response = _atc_read_planes(conn)) == -1) return -1;
        break;

        case atc_airports:
        if ((response = _atc_read_airports(conn)) == -1) return -1;
        break;

        default:
        return -1;
    }
    return response;
}


static int _atc_write_response(struct atc_conn * conn)
{
    struct atc_res * res;
    struct transport_addr * addr;
    unsigned char buffer[ATCP_HEADER_LEN];
    int response;

    res = &conn->res;
    addr = &conn->addr;
    
    atc_res_to_header(res, buffer);
    if (transport_send(addr, buffer, sizeof(buffer)) == -1) return -1;

    switch (res->type) {
        case atc_ack:
        return 0;
        break;

        case atc_planes:
        if ((response = _atc_write_planes(conn)) == -1) return -1;
        break;

        case atc_airports:
        if ((response = _atc_write_airports(conn)) == -1) return -1;
        break;

        default:
        return -1;
    }
    return response;
}


int atc_request(struct atc_conn * conn)
{
    unsigned char req_buffer[ATCP_MSG_LEN];
    int response;

    atc_req_to_wire(&conn->req, req_buffer);
    response = transport_send(&conn->addr, req_buffer, sizeof(req_buffer));
    if (response == -1) {
        return -1;
    }
    return _atc_read_response(conn);
}


int atc_reply(struct atc_conn * conn, atc_reply_handler handler)
{
    unsigned char buffer[ATCP_MSG_LEN];
    int response;

    response = transport_recv(&conn->addr, buffer, sizeof(buffer));

    atc_wire_to_req(&conn->req, buffer);
    if (handler(conn) == -1) {
        conn->res.type = atc_ack;
        conn->res.msg.return_code = -1;
        _atc_write_response(conn);
        return -1;
    }
    _atc_write_response(conn);
    return response;
}

