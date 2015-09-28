#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdint.h>
#include "transport.h"
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>

#define ALL_RW S_IRWXU|S_IRWXG|S_IRWXO


static char fifo_name_indx;
static char inc_fifo[] = "/tmp/scfinc";
static char srv_to_cli[] = "/tmp/scfifo";
static char cli_to_srv[] = "/tmp/csfifo";

int transport_send(struct transport_addr * addr, unsigned char * buffer, size_t size)
{
	return write(addr->conn.fifo_fd[1], buffer, size);
}


int transport_recv(struct transport_addr * addr, unsigned char * buffer, size_t size)
{
	return read(addr->conn.fifo_fd[0], buffer, size);
}


int transport_accept(struct transport_addr * listen, struct transport_addr *accepts)
{
	int fd, new_serv_out_fd, new_serv_in_fd;
	fd = open(inc_fifo, O_RDONLY);
	if(fd == -1){
		return fd;
	}
	printf("%s", "Connection request detected \n");
	new_serv_out_fd = open(srv_to_cli, O_WRONLY);
	new_serv_in_fd = open(cli_to_srv, O_RDONLY);
	if(new_serv_out_fd == -1 || new_serv_in_fd == -1){
		return -1;
	}
	accepts->type = transport_conn_fifo;
	accepts->conn.fifo_fd[0] = new_serv_in_fd;
	accepts->conn.fifo_fd[1] = new_serv_out_fd;
	return 0;

}


int transport_connect(struct transport_addr * addr)
{
	int fd, new_cli_out_fd, new_cli_in_fd;
	fd = open(inc_fifo, O_WRONLY);
	printf("%s", "Connection request detected \n");
	new_cli_in_fd = open(srv_to_cli, O_RDONLY);
	new_cli_out_fd = open(cli_to_srv, O_WRONLY);
	if(new_cli_in_fd == -1 || new_cli_out_fd == -1){
		return -1;
	}
	addr->type = transport_conn_fifo;
	addr->conn.fifo_fd[0] = new_cli_in_fd;
	addr->conn.fifo_fd[1] = new_cli_out_fd;
	close(fd);
	return 0;

}


int transport_listen(struct transport_addr * lsten)
{
	return 0;
}


int transport_close(struct transport_addr * addr)
{
	close(addr->conn.fifo_fd[0]);
	return close(addr->conn.fifo_fd[1]);
}


int transport_serv_init(struct transport_addr * addr)
{
	fifo_name_indx = 32;
	addr->type = transport_conn_fifo;
	
	if ( access(srv_to_cli, 0) == -1 && mkfifo(srv_to_cli, ALL_RW | S_IFIFO) == -1 ){
			return -1;
	}
	if ( access(cli_to_srv, 0) == -1 && mkfifo(cli_to_srv, ALL_RW | S_IFIFO) == -1 ){
			return -1;
	}
	if ( access(inc_fifo, 0) == -1 && mkfifo(inc_fifo, ALL_RW | S_IFIFO) == -1 ){
			return -1;
	}
	return 0;
}


