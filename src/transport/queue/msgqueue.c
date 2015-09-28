#include "transport.h"
#include "msgqueue.h"
#include <mqueue.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>

static struct mq_attr attr;
static int mq_count = 0;

int transport_send(struct transport_addr * addr, unsigned char * buffer, size_t size){
	return mq_send(addr->conn.mqueue[1], (char *)buffer, size, NO_PRIORITY);
}

int transport_recv(struct transport_addr * addr, unsigned char * buffer, size_t size){
	return mq_receive(addr->conn.mqueue[0], (char *)buffer, size, NO_PRIORITY);
}

int transport_connect(struct transport_addr * addr){
	mqd_t buffer[2], srv_out, srv_in;

	srv_in = mq_open(SRV_IN, O_RDWR);
	if (srv_in == -1){
		perror("FALLO CONNECT OPEN SRV");
		return -1;
	}
	printf("send a srv in\n");
	if ((mq_send(srv_in, "Connect to SRV", sizeof("Connect to SRV")+1, NO_PRIORITY)) == -1){
		perror("FALLO CONNECT SEND");	
		return -1;
	}
	printf("cierro srv in\n");
	mq_close(srv_in);

	printf("abro srv out\n");
	srv_out = mq_open(SRV_OUT, O_RDWR);

	printf("llego al receive\n");
	mq_receive(srv_out, (char *)buffer, MSG_SIZE, NO_PRIORITY);
	printf("salio del receive\n");
	addr->conn.mqueue[0] = buffer[1];
	addr->conn.mqueue[1] = buffer[0];
	printf("IN:%d OUT:%d\n", addr->conn.mqueue[0], addr->conn.mqueue[1]);
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
		return -1;
	}	
	return 0;
}

int transport_accept(struct transport_addr * listen, struct transport_addr *accepts){
	char buffer[50], cli_in[100], cli_out[100];

	printf("Viene receive");
	mq_receive(listen->conn.mqueue[0], buffer, MSG_SIZE+1, NO_PRIORITY);
	printf("Sale receive");

	sprintf(cli_in, "/Cli_in_%d", mq_count);
	sprintf(cli_out, "/Cli_out_%d", mq_count++);
	accepts->conn.mqueue[0] = mq_open(cli_in, O_CREAT|O_RDWR, 0666, &attr);
	accepts->conn.mqueue[1] = mq_open(cli_out, O_CREAT|O_RDWR, 0666, &attr);
	if (accepts->conn.mqueue[0] == -1 || accepts->conn.mqueue[1] == -1){
		return -1;
	}

	printf("IN:%d OUT:%d\n", accepts->conn.mqueue[0], accepts->conn.mqueue[1]);
	mq_send(listen->conn.mqueue[1], (char *)(accepts->conn.mqueue), sizeof(accepts->conn.mqueue), NO_PRIORITY);
	return 0;
}
