#include <assert.h>
#include <string.h>

#include "protocol.h"
#include "serialize.h"

unsigned char * _snd_strn(unsigned char * ptr, char * str, size_t len);
unsigned char * _snd_chr(unsigned char * ptr, char c);
unsigned char * _snd_int(unsigned char * ptr, int x);

unsigned char * _get_strn(unsigned char * ptr, char * str, size_t len);
unsigned char * _get_chr(unsigned char * ptr, char * c);
unsigned char * _get_int(unsigned char * ptr, int * x);

unsigned char * _snd_strn(unsigned char * ptr, char * str, size_t len)
{
    assert(ptr != NULL);
    assert(str != NULL);
    memcpy(ptr, str, len);
    return ptr + len;
}

unsigned char * _snd_chr(unsigned char * ptr, char c)
{
    return _snd_strn(ptr, &c, sizeof(c));
}

unsigned char * _snd_int(unsigned char * ptr, int x)
{
    uint32_t number;
    assert(ptr != NULL);
    number = htonl(x);
    memcpy(ptr, &number, sizeof(number));
    return ptr + sizeof(number);
}

unsigned char * _get_strn(unsigned char * ptr, char * str, size_t len)
{
    assert(ptr != NULL);
    assert(str != NULL);
    memcpy(str, ptr, len);
    return ptr + len;
}

unsigned char * _get_chr(unsigned char * ptr, char * c)
{
    return _get_strn(ptr, c, sizeof(char));
}

unsigned char * _get_int(unsigned char * ptr, int * x)
{
    uint32_t number;
    assert(ptr != NULL);
    memcpy(&number, ptr, sizeof(number));
    number = ntohl(number);
    memcpy(x, &number, sizeof(number));
    return ptr + sizeof(number);
}


unsigned char * atc_plane_to_wire(struct atc_plane * plane, unsigned char wire[ATCP_MSG_LEN])
{
    unsigned char * ptr;
    ptr = wire;
    assert(plane != NULL);
    ptr = _snd_strn(ptr, plane->id, ATCD_ID_LENGTH);
    ptr = _snd_int(ptr, plane->x);
    ptr = _snd_int(ptr, plane->y);
    ptr = _snd_int(ptr, plane->z);
    ptr = _snd_int(ptr, (int) plane->time);
    ptr = _snd_chr(ptr, (char) plane->heading);
    ptr = _snd_int(ptr, plane->speed);
    ptr = _snd_chr(ptr, (char) plane->status);
    return ptr;
}


unsigned char * atc_wire_to_plane(struct atc_plane * plane, unsigned char wire[ATCP_MSG_LEN])
{
    unsigned char * ptr;
    ptr = wire;
    assert(plane != NULL);
    ptr = _get_strn(ptr, plane->id, ATCD_ID_LENGTH);
    ptr = _get_int(ptr, &plane->x);
    ptr = _get_int(ptr, &plane->y);
    ptr = _get_int(ptr, &plane->z);
    ptr = _get_int(ptr, (int *) &plane->time);
    ptr = _get_chr(ptr, (char *) &plane->heading);
    ptr = _get_int(ptr, &plane->speed);
    ptr = _get_chr(ptr, (char *) &plane->status);
    return ptr;
}


unsigned char * atc_airport_to_wire(struct atc_airport * airport, unsigned char wire[ATCP_MSG_LEN])
{
    unsigned char * ptr;
    ptr = wire;
    assert(airport != NULL);
    ptr = _get_int(ptr, &airport->x);
    ptr = _get_int(ptr, &airport->y);
    ptr = _get_strn(ptr, airport->id, 3);
    return ptr;
}

unsigned char * atc_wire_to_airport(struct atc_airport * airport, unsigned char wire[ATCP_MSG_LEN])
{
    unsigned char * ptr;
    ptr = wire;
    assert(airport != NULL);
    ptr = _snd_int(ptr, airport->x);
    ptr = _snd_int(ptr, airport->y);
    ptr = _snd_strn(ptr, airport->id, 3);
    return ptr;
}



unsigned char * atc_req_to_wire(struct atc_req * req, unsigned char wire[ATCP_MSG_LEN])
{
    unsigned char * ptr;
    ptr = wire;

    ptr = _snd_int(ptr, req->type);
    switch (req->type) {
        case atc_speed_up:
        case atc_speed_down:
        case atc_turn_left:
        case atc_turn_right:
        case atc_ascend:
        case atc_descend:
        ptr = atc_plane_to_wire(&req->plane, ptr);
        break;

        default:
        memset(ptr, 0, ATCP_MSG_LEN - (ptr - wire));
    }
    return ptr;
}

unsigned char * atc_wire_to_req(struct atc_req * req, unsigned char wire[ATCP_MSG_LEN])
{
    unsigned char * ptr;
    ptr = wire;

    ptr = _get_int(ptr, (int *) &req->type);
    switch (req->type) {
        case atc_speed_up:
        case atc_speed_down:
        case atc_turn_left:
        case atc_turn_right:
        case atc_ascend:
        case atc_descend:
        ptr = atc_wire_to_plane(&req->plane, ptr);
        break;

        default:
        memset(&req->plane, 0, sizeof(struct atc_plane));
        break;
    }
    return ptr;
}


int atc_header_to_res(struct atc_res * res, unsigned char wire[ATCP_HEADER_LEN])
{
    int header_type;
    int header_payload;
    unsigned char * ptr;

    ptr = wire;
    ptr = _get_int(ptr, &header_type);
    ptr = _get_int(ptr, &header_payload);
    
    res->type = (enum atc_res_type) header_type;
    switch (res->type) {
        case atc_ack:
        res->len.planes = 0;
        res->msg.return_code = header_payload;
        break;

        case atc_planes:
        res->len.planes = header_payload;
        break;

        case atc_airports:
        res->len.airports = header_payload;
        break;

        default:
        return -1;
    }
    return ptr - wire;
}

int atc_res_to_header(struct atc_res * res, unsigned char wire[ATCP_HEADER_LEN])
{
    unsigned char * ptr;

    ptr = wire;
    ptr = _snd_int(ptr, (int) res->type);
    
    switch (res->type) {
        case atc_ack:
        ptr = _snd_int(ptr, res->msg.return_code);
        break;

        case atc_planes:
        ptr = _snd_int(ptr, res->len.planes);
        break;

        case atc_airports:
        ptr = _snd_int(ptr, res->len.airports);
        break;

        default:
        return -1;
    }
    return ptr - wire;
}


