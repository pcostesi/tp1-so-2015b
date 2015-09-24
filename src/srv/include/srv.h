#ifndef __SRV_H
#define __SRV_H 1

#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

struct listener_list{
	listener_node head;
	listener_node tail;
};

struct listener_node{
	pid_t pid;
	listener_node next;
};

void listen_client();
void listen_channel();
void fork_client();
void kill_client(pid_t pid);


