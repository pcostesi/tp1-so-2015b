#include "transport.h"
#include "msgqueue.h"
#include <mqueue.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

static struct mq_attr attr;

int transport_send(struct transport_addr * addr, unsigned char * buffer, size_t size){
	int res = mq_send(addr->conn.mqueue[1], (char *)buffer, size, NO_PRIORITY);
	if (res == -1){
		perror("Fallo send");
	}
	return res;
}

int transport_recv(struct transport_addr * addr, unsigned char * buffer, size_t size){
	int res = mq_receive(addr->conn.mqueue[0], (char *)buffer, MSG_SIZE+1, NO_PRIORITY);
	if (res == -1){
		perror("Fallo en receive");
	}
	return res;
}

int transport_connect(struct transport_addr * addr){
	char buffer[50], cli_in[100], cli_out[100];
	int my_pid = getpid();
	mqd_t srv_out, srv_in;

	srv_in = mq_open(SRV_IN, O_RDWR);
	srv_out = mq_open(SRV_OUT, O_RDWR);
	if (srv_in == -1 || srv_out == -1){
		perror("Falla en open de connect");
		return -1;
	}
	if ((mq_send(srv_in, (char *) &my_pid, sizeof(int), NO_PRIORITY)) == -1){
		perror("Falla en send a srv out");	
		return -1;
	}

	mq_receive(srv_out, (char *)buffer, MSG_SIZE+1, NO_PRIORITY);
	sprintf(cli_in, "/Cli_in_%d", my_pid);
	sprintf(cli_out, "/Cli_out_%d", my_pid);
	addr->conn.mqueue[0] = mq_open(cli_out, O_RDWR);
	addr->conn.mqueue[1] = mq_open(cli_in, O_RDWR);
	if (addr->conn.mqueue[0] == -1 || addr->conn.mqueue[1] == -1){
		perror("Falla en open de mqs del cli");
	}
	/*printf("IN:%d OUT:%d\n", addr->conn.mqueue[0], addr->conn.mqueue[1]);*/

	mq_close(srv_in);
	mq_close(srv_out);
	return 0;
}

int transport_listen(struct transport_addr * lsten){
	return 0;
}

int transport_close(struct transport_addr * addr){
	mq_close(addr->conn.mqueue[0]);
	mq_close(addr->conn.mqueue[1]);
	return 0;
}

int transport_serv_init(struct transport_addr * addr){
	attr.mq_maxmsg = QUEUE_SIZE;
	attr.mq_msgsize = MSG_SIZE;
	addr->conn.mqueue[0] = mq_open(SRV_IN, O_RDWR|O_CREAT, 0666, &attr);
	addr->conn.mqueue[1] = mq_open(SRV_OUT, O_RDWR|O_CREAT, 0666, &attr);
	if (addr->conn.mqueue[0] == -1 || addr->conn.mqueue[1] == -1){
		perror("Falla en open de srv init");
		return -1;
	}	
	return 0;
}

int transport_accept(struct transport_addr * listen, struct transport_addr *accepts){
	char cli_in[100], cli_out[100];
	int child_pid;

	mq_receive(listen->conn.mqueue[0], (char *)&child_pid, MSG_SIZE+1, NO_PRIORITY);
	printf("Conexion aceptada...\n");

	sprintf(cli_in, "/Cli_in_%d", child_pid);
	sprintf(cli_out, "/Cli_out_%d", child_pid);
	accepts->conn.mqueue[0] = mq_open(cli_in, O_CREAT|O_RDWR, 0666, &attr);
	accepts->conn.mqueue[1] = mq_open(cli_out, O_CREAT|O_RDWR, 0666, &attr);
	if (accepts->conn.mqueue[0] == -1 || accepts->conn.mqueue[1] == -1){
		perror("Falla en open de accept");
		return -1;
	}

	/*printf("IN:%d OUT:%d\n", accepts->conn.mqueue[0], accepts->conn.mqueue[1]);*/
	mq_send(listen->conn.mqueue[1], "Accepted", sizeof("Accepted")+1, NO_PRIORITY);
	return 0;
}
