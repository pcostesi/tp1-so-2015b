#ifndef __SRV_H
#define __SRV_H 1

#include "protocol.h"

#define MAX_CLIENTS 50

struct pid_node{
	pid_t pid;
	struct atc_conn conn;
};

void init_server(void);
void init_signal_handlers(void);
void srv_sigchld_handler(int sig);
void srv_sigint_handler(int sig);
void listen_channels(void);
void listen_child_channels(struct atc_conn * conn);
void kill_client(pid_t pid);
void fork_client(struct atc_conn * conn);
int reply_handler(struct atc_conn * conn);

#endif
