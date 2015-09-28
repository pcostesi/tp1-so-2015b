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
#include <time.h>


#define ALL_RW S_IRWXU|S_IRWXG|S_IRWXO
#define MAX_FIFO_NAME 35

static int _get_fifo_name(char buff[], char name[]);


static char inc_fifo[] = "/tmp/scfinc";
static char srv_to_cli[] = "/tmp/scfifo";
static char cli_to_srv[] = "/tmp/csfifo";
static int new_fifo_index = 0;



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
	char name_buffer[MAX_FIFO_NAME] = {0};
	int fd, new_serv_out_fd, new_serv_in_fd, status;
	printf("%s", "Opening fifo for requests \n");
	fd = open(inc_fifo, O_RDONLY);
	if(fd == -1){
		return -1;
	}
	printf("%s", "Connection request detected \n");
	/*Reads client in FIFO filename*/
	status = read(fd, name_buffer, sizeof(name_buffer));
	if(status == -1){
		close(fd);
		return -1;
	}	
	printf("%s", "Received client IN name \n");
	/*Opens client_in/srv_out fifo*/
	new_serv_out_fd = open(name_buffer, O_WRONLY);

	/*once opened, sends the server in fifo*/

	status = _get_fifo_name(name_buffer, cli_to_srv);
	if(status == -1){
		close(fd);
		return -1;
	}


	/*Sends new serv_in fifo to client*/
	status = write(new_serv_out_fd, name_buffer, sizeof(name_buffer));
		if(status == -1 ){
		close(fd);
		close(new_serv_out_fd);
		return -1;
	}

	printf("%s", "Sent client OUT name \n");

	new_serv_in_fd = open(name_buffer, O_RDONLY);
	if(new_serv_in_fd == -1){
		close(fd);
		close(new_serv_out_fd);
		return -1;
	}


	accepts->type = transport_conn_fifo;
	accepts->conn.fifo_fd[0] = new_serv_in_fd;
	accepts->conn.fifo_fd[1] = new_serv_out_fd;

	printf("%s", "Connection established.\n");
	return 0;

}


int transport_connect(struct transport_addr * addr)
{
	srand(time(NULL));
	new_fifo_index = rand();

	struct flock lock;
	memset (&lock, 0, sizeof(lock));
	lock.l_type = F_WRLCK;
	char name_buffer[MAX_FIFO_NAME] = {0};
	int fd, new_cli_out_fd, new_cli_in_fd, status;
	/*Opens serv request Fifo*/
	fd = open(inc_fifo, O_WRONLY);
	if(fd == -1 ){
		return -1;
	}
	fcntl (fd, F_SETLKW, &lock);

	/*Sends client_in fifo name to srv and the opens it*/

	status = _get_fifo_name(name_buffer, srv_to_cli);
	if(status == -1){
		close(fd);
		return status;
	}	

	status = write(fd, name_buffer, sizeof(name_buffer));
	if(status == -1){
		close(fd);
		return status;
	}	
	lock.l_type = F_UNLCK;
	 fcntl (fd, F_SETLKW, &lock);

	new_cli_in_fd = open(name_buffer, O_RDONLY);
	if(new_cli_in_fd == -1){
		close(fd);
		return -1;
	}

	/*Reads from client_in to receive client_out name from server*/
	status = read(new_cli_in_fd, name_buffer, sizeof(name_buffer));

	/*Opens client_out fifo*/
	new_cli_out_fd = open(name_buffer, O_WRONLY);

	if(new_cli_out_fd == -1){
		close(new_cli_in_fd);
		return -1;
	}

	/*If no error occurs, saves fifos fd*/
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
	addr->type = transport_conn_fifo;
	if ( access(inc_fifo, 0) == -1 && mkfifo(inc_fifo, ALL_RW | S_IFIFO) == -1 ){
			return -1;
	}
	return 0;
}


int _get_fifo_name(char buff[], char name[])
{

	printf("%s", "Generating name/file \n");
	memset(buff, 0, sizeof(*buff));
	memcpy(buff, name, sizeof(*name));
	sprintf(buff+strlen(buff),"%d", new_fifo_index++);
	if ( access(buff, 0) == -1 && mkfifo(buff, ALL_RW | S_IFIFO) == -1 ){
			return -1;
	}

	printf("%s %d \n", "Ended generatig name/file, with indx", new_fifo_index);
	return 0;

}