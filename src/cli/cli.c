#include "atcd.h"
#include "cli.h"


static struct atc_addr com;
static struct atc_req request;
static struct atc_res response;
static struct atc_plane buff[MAX_PLANES];
static struct atc_airport[MAX_AIRPORTS];



int atc_init(void)
{
	req.type = atc_join;
	req.id = 
	return atc_connect(&com, &request, &response);
}


void create_plane(void)
{
	req.type
	atc_req(&com, );
    return;
}

int get_airports(struct atc_airport ports[])
{
	req.type = atc_get_airports;
    return -1;
}

int set(enum atc_commands cmd, struct atc_plane *plane)
{
    return -1;
}

int get_airplanes(struct atc_plane buffer[])
{
    return -1;
}

static void _no_id()
{
	int i = 0;
	for(i = 0; i < ATCD_ID_LENGTH; )
}

/*

struct atc_req {
    enum atc_req_type type;
    char id[ATCD_ID_LENGTH];
};
*/

/*
struct atc_addr {
    enum atc_conn_type type;
    union {
        struct sockaddr socket;
        void * shmem;
    } conn;
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
*/
