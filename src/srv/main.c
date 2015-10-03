#include "atcd.h"
#include "srv.h"
#include "protocol.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

static int cur_clients = 0;
static struct pid_node nodes[50];
static struct atc_conn serverConn;

int main(int argc, char ** arcv)
{
    printf("Initializing server...\n");
	init_server();
    printf("Waiting for new channels...\n");
	listen_channels();

    return 0;
}

void init_server(void){
    if (atc_init() == -1) {
        perror("Could not init");
        exit(-1);
    }

    if (atc_listen(&serverConn) == -1) {
      perror("Could not listen");
      exit(-1); 
    }
    
    init_signal_handlers();
}

void init_signal_handlers(void){
    struct sigaction sigchld;
    struct sigaction sigint;
    
    memset(&sigchld, 0, sizeof(sigchld));
    sigchld.sa_handler = srv_sigchld_handler;
    sigaction(SIGCHLD, &sigchld, NULL);

    memset(&sigint, 0, sizeof(sigint));
    sigint.sa_handler = srv_sigint_handler;
    sigaction(SIGINT, &sigint, NULL);
}

void srv_sigchld_handler(int sig){
    pid_t pid;
    int status;
    while ((pid = waitpid(-1, &status, WNOHANG)) != -1){
        kill_client(pid);
    }
}

void srv_sigint_handler(int sig){
    int i;
    for(i = 0; i < cur_clients; i++){
        atc_close(&nodes[i].conn);
    }        
    exit(0);
}

void fork_client(struct atc_conn * conn){
	pid_t child_pid = fork();
    cur_clients++;
	if (child_pid == 0){
        printf("Waiting for requests in pid %d ...\n", getpid());
		listen_child_channels(conn);
	}else{
        nodes[cur_clients].pid = child_pid;
        nodes[cur_clients].conn = *conn;
	}
}

void kill_client(pid_t pid){
    int i;
    cur_clients--;
	for(i = 0; i < cur_clients; i++){
        int startMoving = 0;
        if (nodes[i].pid == pid){
            startMoving = 1;
            atc_close(&nodes[i].conn);
            continue;
        }if (startMoving){
            nodes[i-1].pid = nodes[i].pid;  
            nodes[i-1].conn = nodes[i].conn;         
        }
    }
}

void listen_channels(void){
    struct atc_conn *childConn;
    int result;
	while (1) {
        if (cur_clients + 1 < MAX_CLIENTS){
            childConn = &(nodes[cur_clients + 1].conn);
            result = atc_accept(&serverConn, childConn);
            if (result == -1) {
                puts("Could not accept connection.");
                return;
            }
            fork_client(childConn); 
        }
	}
    return;
}

void listen_child_channels(struct atc_conn * conn){
	while (atc_reply(conn, (atc_reply_handler) &reply_handler) != -1) {
    }
    atc_close(conn);
	return;
}

int reply_handler(struct atc_conn * conn){
    int aux;
    struct atc_res * response = &conn->res;
    response->type = atc_ack;
    response->msg.return_code = 0;
	switch (conn->req.type){
		case atc_speed_up:
    	if (set(speed_up, &(conn->req.plane)) == -1){
    		response->msg.return_code = -1;
    	}
		break;

    	case atc_speed_down:
    	if(set(speed_down, &(conn->req.plane)) == -1){
    		response->msg.return_code = -1;
    	}
    	break;

    	case atc_turn_left:
    	if(set(turn_left, &(conn->req.plane)) == -1){
    		response->msg.return_code = -1;
    	}
    	break;

    	case atc_turn_right:
    	if(set(turn_right, &(conn->req.plane)) == -1){
    		response->msg.return_code = -1;
    	}
    	break;

    	case atc_ascend:
    	if (set(climb, &(conn->req.plane)) == -1){
    		response->msg.return_code = -1;
    	}
    	break;

    	case atc_descend:
    	if(set(descend, &(conn->req.plane)) == -1){
    		response->msg.return_code = -1;
    	}
    	break;

    	case atc_get_planes:
    	response->type = atc_planes;
    	aux = get_airplanes(response->msg.planes);
        if (aux == -1){
            response->msg.return_code = -1;
        }else{
            response->len.planes = aux;
        }
    	break;

    	case atc_get_airports:
    	response->type = atc_airports;
    	response->len.airports = get_airports(response->msg.airports);
    	break;

    	case atc_get_landed:
    	response->type = atc_ack;
        aux = get_landed();
        if (aux == -1){
            response->msg.return_code = -1;
        }else{
            response->msg.return_code = aux;
        }
    	break;

    	case atc_get_crashed:
        response->type = atc_ack;
    	aux = get_crashed();
        if (aux == -1){
            response->msg.return_code = -1;
        }else{
            response->msg.return_code = aux;
        }
    	break;

    	case atc_create_plane:
    	create_plane();
    	break;

    	case atc_join:
    	break;

    	case atc_leave:
        puts("Hanging");
        return -1;
    	break;
	}
    return 0;
}
