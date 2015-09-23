#include <netinet/in.h>
#include <assert.h>
#include <string.h>

#include "protocol.h"
#include "atcd.h"

#define ATCP_PLANE_MSG_LEN (ATCD_ID_LENGTH + 4 * sizeof(uint32_t) + 2 * sizeof(char))

static void _plane_to_wire(struct atc_plane * plane, unsigned char wire[ATCP_PLANE_MSG_LEN]);
static void _wire_to_plane(struct atc_plane * plane, unsigned char wire[ATCP_PLANE_MSG_LEN]);

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
    assert(ptr != NULL);
    uint32_t number = htonl(x);
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


static void _plane_to_wire(struct atc_plane * plane, unsigned char wire[ATCP_PLANE_MSG_LEN])
{
    unsigned char * ptr = wire;
    assert(plane != NULL);
    ptr = _snd_strn(ptr, plane->id, ATCD_ID_LENGTH);
    ptr = _snd_int(ptr, plane->x);
    ptr = _snd_int(ptr, plane->y);
    ptr = _snd_int(ptr, plane->z);
    ptr = _snd_int(ptr, (int) plane->time);
    ptr = _snd_chr(ptr, (char) plane->heading);
    ptr = _snd_int(ptr, plane->speed);
    ptr = _snd_chr(ptr, (char) plane->status);
}


static void _wire_to_plane(struct atc_plane * plane, unsigned char wire[ATCP_PLANE_MSG_LEN])
{
    unsigned char * ptr = wire;
    assert(plane != NULL);
    ptr = _get_strn(ptr, plane->id, ATCD_ID_LENGTH);
    ptr = _get_int(ptr, &plane->x);
    ptr = _get_int(ptr, &plane->y);
    ptr = _get_int(ptr, &plane->z);
    ptr = _get_int(ptr, (int *) &plane->time);
    ptr = _get_chr(ptr, (char *) &plane->heading);
    ptr = _get_int(ptr, &plane->speed);
    ptr = _get_chr(ptr, (char *) &plane->status);
}


