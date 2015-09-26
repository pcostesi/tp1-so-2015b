#include "atcd.h"
#include "srv.h"
#include "protocol.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

struct pid_list pids;

int main(int argc, char ** arcv)
{
    printf("Initializing server...\n");
	init_server();
	while(1){
        printf("Waiting for new channels...\n");
		listen_channels();
	}
    return 0;
}

void init_server(void){
    atc_init();
    init_signal_handlers();
}

void init_signal_handlers(void){
    struct sigaction sigchld;
    memset(&sigchld, 0, sizeof(sigchld));
    sigchld.sa_handler = srv_sigchld_handler;
    sigaction(SIGCHLD, &sigchld, NULL);

    struct sigaction sigint;
    memset(&sigint, 0, sizeof(sigint));
    sigint.sa_handler = srv_sigint_handler;
    sigaction(SIGINT, &sigint, NULL);
}

void srv_sigchld_handler(int sig){
    pid_t p;
    int status;
    while ((p = waitpid(-1, &status, WNOHANG)) != -1){
        struct pid_node node = get_node_from_pid(p);
        atc_close(node.conn);
        free(node);
    }
}

void srv_sigint_handler(int sig){
    struct pid_node * next, cur_node;
    cur_node = next = pids.head;
    while (next != NULL){
        atc_close(cur_node->conn);
        next = cur_node->next;
        free(cur_node);
        cur_node = next;
    }        
}

void fork_client(struct atc_conn * conn){
	pid_t child_pid = fork();
    struct pid_node * node;
	if (child_pid == 0){
        printf("Waiting for requests in pid %d ...\n", child_pid);
		listen_child_channels();
	}else{
		node = (struct pid_node *) malloc(sizeof(pid_node));
        node->pid = child_pid;
        node->conn = conn;
        node->next = NULL;
		if (pids.head == NULL){
			pids.head = pids.tail = node;
		}else{
			pids.tail->next = node;
			pids.tail = node;
		}
	}
	return;
}

struct atc_conn * get_conn_from_pid(pid_t pid){
    struct pid_node * cur_node = pids.head;
    while(cur_node != NULL){
        if (cur_node->pid == pid){
            return cur_node->conn;
        }
    }
    return NULL;
}

void kill_client(pid_t pid){
	struct pid_node *prev_node;
	struct pid_node *cur_node = pids.head;
	while(cur_node != NULL){
		if (cur_node->pid == pid){
            if (cur_node == pids.tail){
                pids.tail = prev_node;
            }
            if (cur_node == pids.head){
                pids.head = cur_node->next;
            }
            if (prev_node != NULL){
                prev_node->next = cur_node->next;
            }
            printf("Killed pid %d ...\n", pid);
			kill(pid, SIGTERM);
            free(cur_node);
			return;
		}
		prev_node = cur_node;
		cur_node = cur_node->next;
	}
	return;
}

void listen_channels(void){
	struct atc_conn conn;
	while (!atc_listen(&conn)){
	}
	if (conn.req.type == atc_join){
		fork_client();
	}
	return;
}

void listen_child_channels(void){
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