#include "atcd.h"
#include "srv.h"
#include <stdio.h>

struct listener_list listeners;

int main(int argc, char ** arcv)
{
	atc_init();
	while(true){
		listen_clients();
	}
    /*int atcd_res = atcd_test();
    printf("This is the server calling itself: %d\n", atcd_res);*/
    return 0;
}

void fork_client(){
	pid_t child_pid = fork();
	if (child_pid == 0){
		listen_channel();
	}else{
		struct listener_node new_node = {child_pid, NULL};
		if (listeners.head == NULL){
			listeners.head = listeners.tail = new_node;
		}else{
			listeners.tail.next = new_node;
			listeners.tail = new_node;
		}
	}
	return;
}

void kill_client(pid_t pid){
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
}

void listen_clients(){
	struct atc_addr channel;
	int heardChannel = 0;
	while (!atc_listen(&channel){

	}
	fork_client();
	return;
}

void listen_channel(){
	struct atc_conn conn;
	while (!atc_reply(&conn, atc_reply_handler)){

	}
	return;
}

int atc_reply_handler(struct atc_conn conn){
	enum atc_req_type req_type = conn.req.type;
	switch (req_type){
		case atc_speed_up:
		break;
    	case atc_speed_down:
    	break;
    	case atc_turn_left:
    	break;
    	case atc_turn_right:
    	break;
    	case atc_ascend:
    	break;
    	case atc_descend:
    	break;
    	case atc_get_time:
    	break;
    	case atc_get_planes:
    	break;
    	case atc_get_airports:
    	break;
    	case atc_get_landed:
    	break;
    	case atc_get_crashed:
    	break;
    	case atc_create_plane:
    	break;
	}
}