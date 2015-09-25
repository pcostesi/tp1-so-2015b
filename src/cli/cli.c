#include "atcd.h"
#include "protocol.h"

static struct atc_conn conn;


int atc_init(void)
{
	conn.req.type = atc_join;
	return atc_connect(&conn);
}


int get_planes(struct atc_plane buff[])
{
	conn.req.type = atc_get_planes;
	atc_request(&conn);
/*	check errors*/
	if(conn.res.type == atc_err){
		return conn.res.msg.error_code;
	}
	int count = conn.res.len.planes;
	int i = 0;
	for( i = 0; i < count ; i++){
		memcpy(&buff[i], &conn.res.msg.planes[i], sizeof(struct atc_plane));
	}
	return count;
}


int get_airports(struct atc_airport buff[])
{
	conn.req.type = get_airports;
	atc_request(&conn);
	if(conn.res.type == atc_err){
		return conn.res.msg.error_code;
	}
	int count = conn.res.len.airports;
	int i = 0;
	for(i = 0; i < count ; i++){
		memcpy(&buff[i], &conn.res.msg.airports[i], sizeof(struct atc_airport));
	}
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
			case turn_right : conn.req.type = atc_turn_rigth;
						break;
			case turn_left : conn.req.type = atc_turn_left;  
						break;
			default: return -1;
		}
	conn.req.data.plane = &plane;
	atc_request(&conn);
	if(conn.res.type == atc_ack){
		return 0;
	}
	else{
		return conn.res.msg.error_code;
	}

}


