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


#define CONN_MSG_SIZE 20

static void _next_fifo_name(char buff[]);

static uint32_t fifo_name_indx;
static char* fifo_aux_name = "/tmp/fifo-num-";
static char* fifo_conn_req = "/tmp/atc_fifo";

int transport_send(struct transport_addr * addr, unsigned char * buffer, size_t size)
{
	return write(addr->conn.fifo_fd[1], buffer, size);
}


int transport_recv(struct transport_addr * addr, unsigned char * buffer, size_t size)
{
	return read(addr->conn.fifo_fd[0], buffer, size);
}


int transport_connect(struct transport_addr * addr)
{
	char name_buff[CONN_MSG_SIZE] = {0};
	int fd, new_cli_in_fd, aux;
	fd = open(fifo_conn_req, O_WRONLY);
	if(fd == -1){
		return fd;
	}
	_next_fifo_name(name_buff);
	if ( access(name_buff, 0) == -1 && mknod(name_buff, S_IFIFO|0666, 0) == -1 ){
			return -1;
	}
	aux = write(fd, name_buff, sizeof(name_buff));
	if(aux == -1){
		return aux;
	}
	/*Opening clients IN fifo*/
	new_cli_in_fd = open(name_buff, O_RDONLY, 0);
	aux = read(new_cli_in_fd, name_buff, sizeof(name_buff));
	if(aux == -1){
		return aux;
	}
	/*Opening Clients OUT Fifo*/
	fd = open(name_buff, O_RDONLY, 0);
	if(fd == -1){
		return -1;
	}
	addr->type = transport_conn_fifo;
	addr->conn.fifo_fd[0] = new_cli_in_fd;
	addr->conn.fifo_fd[1] = fd;
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
	fifo_name_indx = 0;
	addr->type = transport_conn_fifo;
	if ( access(fifo_conn_req, 0) == -1 && mknod(fifo_conn_req, S_IFIFO|0666, 0) == -1 ){
			return -1;
	}
	return 0;
}


int transport_accept(struct transport_addr * listen, struct transport_addr *accepts)
{
	char name_buff[CONN_MSG_SIZE] = {0};
	int fd, new_cli_in_fd, aux;
	fd = open(fifo_conn_req, O_RDONLY);
	printf("%s", "Connection request detected \n");
	listen->conn.fifo_fd[0] = fd;
	aux = read(fd, name_buff, sizeof(name_buff));
	if(aux == -1){
		return aux;
	}
	/*Opening clients IN fifo*/
	new_cli_in_fd = open(name_buff, O_WRONLY);
	_next_fifo_name(name_buff);
	if ( access(name_buff, 0) == -1 && mknod(name_buff, S_IFIFO|0666, 0) == -1 ){
			return -1;
	}
	aux = write(new_cli_in_fd, name_buff, sizeof(name_buff));
	if(aux == -1){
		return aux;
	}
	/*Opening Clients OUT Fifo*/

	fd = open(name_buff, O_RDONLY, 0);
	if(fd == -1){
		return -1;
	}
	accepts->type = transport_conn_fifo;
	accepts->conn.fifo_fd[0] = fd;
	accepts->conn.fifo_fd[1] = new_cli_in_fd;
	printf("%s", "Connection established with a client. \n");
	return 0;
}


static void _next_fifo_name(char buff[])
{

	printf("%s", "Pisalo chicho!. \n");
	strcat(buff, fifo_aux_name );
	memcpy(buff+sizeof(fifo_aux_name), &fifo_name_indx, sizeof(fifo_name_indx));
	fifo_name_indx++;

	printf("%s", "Que lo pisaras joder!. \n");
	printf("%s \n", buff);
	return;
}