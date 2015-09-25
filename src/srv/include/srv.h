#ifndef __SRV_H
#define __SRV_H 1

#include "protocol.h"

/*struct listener_node{
	pid_t pid;
	struct listener_node *next;
};

struct listener_list{
	struct listener_node *head;
	struct listener_node *tail;
};*/
void reply_handler(struct atc_conn * conn);
void listen_child_channels();
void listen_channels();
void fork_client();
/*void kill_client(pid_t pid);*/

#endif
