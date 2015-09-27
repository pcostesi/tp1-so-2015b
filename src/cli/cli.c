#include "atcd.h"
#include "cli.h"
#include "protocol.h"
#include <signal.h>
#include <string.h>

static struct atc_conn conn;

int atc_init(void)
{
	init_signal_handler();
	conn.req.type = atc_join;
	return atc_connect(&conn);
}

void init_signal_handler(void){
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = cli_sigint_handler;
    sigaction(SIGINT, &sa, NULL);
}

void cli_sigint_handler(int sig){
	conn.req.type = atc_leave;
	atc_request(&conn);
	atc_close(&conn);	
}

int get_airplanes(struct atc_plane buff[])
{
	int count;
	int i;
	conn.req.type = atc_get_planes;
	atc_request(&conn);
	/*	check errors*/
	if(conn.res.type == atc_ack){
		return conn.res.msg.return_code;
	}
	count = conn.res.len.planes;
	for( i = 0; i < count ; i++){
		memcpy(&buff[i], &conn.res.msg.planes[i], sizeof(struct atc_plane));
	}
	return count;
}

int get_crashed(void)
{
	conn.req.type = atc_get_crashed;
	atc_request(&conn);
	return conn.res.msg.return_code;
}


int get_landed(void)
{
	conn.req.type = atc_get_landed;
	atc_request(&conn);
	return conn.res.msg.return_code;

}


int get_airports(struct atc_airport buff[])
{
	int count;
	int i;
	conn.req.type = atc_get_airports;
	atc_request(&conn);
	if(conn.res.msg.return_code == -1){
		return conn.res.msg.return_code;
	}
	count = conn.res.len.airports;
	for(i = 0; i < count ; i++){
		memcpy(&buff[i], &conn.res.msg.airports[i], sizeof(struct atc_airport));
	}
	return count;
}

void create_plane(void)
{
	conn.req.type = atc_create_plane;
	atc_request(&conn);
	return;
}

int set(enum atc_commands cmd, struct atc_plane *plane)
{
	switch (cmd){
			case speed_up : conn.req.type = atc_speed_up;
						break;
			case speed_down : conn.req.type = atc_speed_down;
						break;
			case climb : conn.req.type = atc_ascend;
						break;
			case descend : conn.req.type = atc_descend;
						break;
			case turn_right : conn.req.type = atc_turn_right;
						break;
			case turn_left : conn.req.type = atc_turn_left;  
						break;
			default: return -1;
		}
	conn.req.plane = *plane;
	atc_request(&conn);
	return conn.res.msg.return_code;
}


