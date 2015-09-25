#include "atcd.h"
#include "srv.h"
#include "protocol.h"
#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

/*struct listener_list listeners;*/

int main(int argc, char ** arcv)
{
	atc_init();
	while(1){
		listen_channels();
	}
    /*int atcd_res = atcd_test();
    printf("This is the server calling itself: %d\n", atcd_res);*/
    return 0;
}

void fork_client(){
	pid_t child_pid = fork();
	if (child_pid == 0){
		listen_child_channels();
	}/*else{
		struct listener_node new_node = {child_pid, NULL};
		if (listeners.head == NULL){
			listeners.head = listeners.tail = new_node;
		}else{
			listeners.tail.next = new_node;
			listeners.tail = new_node;
		}
	}*/
	return;
}

/*void kill_client(pid_t pid){
	struct listener_node prev_node;
	struct listener_node cur_node = listeners.head;
	while(cur_node != NULL){
		if (cur_node.pid == pid){
			kill(pid, SIGTERM);
			prev_node.next = cur_node.tail;
			return;
		}
		prev_node = cur_node;
		cur_node = cur_node.next;
	}
	return;
}*/

void listen_channels(){
	struct atc_conn conn;
	while (!atc_listen(&conn)){

	}
	if (conn.req.type == atc_join){
		fork_client();
	}
	return;
}

void listen_child_channels(){
	struct atc_conn conn;
	while (!atc_reply(&conn, &reply_handler)){

	}
	return;
}

void reply_handler(struct atc_conn * conn){
	struct atc_res response = conn->res;
	switch (conn->req.type){
		case atc_speed_up:
		response.type = atc_ack;
    	if (set(speed_down, &(conn->req.plane)) == -1){
    		response.type = atc_err;
    		response.msg.error_code = -1;
    	}
		break;
    	case atc_speed_down:
    	response.type = atc_ack;
    	if(set(speed_down, &(conn->req.plane)) == -1){
    		response.type = atc_err;
    		response.msg.error_code = -1;
    	}
    	break;
    	case atc_turn_left:
    	response.type = atc_ack;
    	if(set(turn_left, &(conn->req.plane)) == -1){
    		response.type = atc_err;
    		response.msg.error_code = -1;
    	}
    	break;
    	case atc_turn_right:
    	response.type = atc_ack;
    	if(set(turn_right, &(conn->req.plane)) == -1){
    		response.type = atc_err;
    		response.msg.error_code = -1;
    	}
    	break;
    	case atc_ascend:
    	response.type = atc_ack;
    	if (set(climb, &(conn->req.plane)) == -1){
    		response.type = atc_err;
    		response.msg.error_code = -1;
    	}
    	break;
    	case atc_descend:
    	response.type = atc_ack;
    	if(set(descend, &(conn->req.plane)) == -1){
    		response.type = atc_err;
    		response.msg.error_code = -1;
    	}
    	break;
    	case atc_get_planes:
    	response.type = atc_planes;
    	response.len.planes = get_airplanes(response.msg.planes);
    	break;
    	case atc_get_airports:
    	response.type = atc_airports;
    	response.len.airports = get_airports(response.msg.airports);
    	break;
    	case atc_get_landed:
    	/*TODO*/
    	break;
    	case atc_get_crashed:
    	/*TODO*/
    	break;
    	case atc_create_plane:
    	response.type = atc_ack;
    	create_plane();
    	break;
    	case atc_join:
    	response.type = atc_err;
    	response.msg.error_code = -1;
    	break;
    	case atc_leave:
    	exit(0);
    	break;
	}
}