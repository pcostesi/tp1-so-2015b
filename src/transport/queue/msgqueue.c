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
	return mq_send(addr->conn.mqueue, (char *)buffer, size, NO_PRIORITY);
}

int transport_recv(struct transport_addr * addr, unsigned char * buffer, size_t size){
	return mq_receive(addr->conn.mqueue, (char *)buffer, MSG_SIZE+1, NO_PRIORITY);
}

int transport_connect(struct transport_addr * addr){
	addr->type = transport_conn_queue;
	addr->conn.mqueue = mq_open(SRV_NAME, O_RDWR);
	if (addr->conn.mqueue == -1){
		perror("FALLO CONNECT OPEN SRV");
		return -1;
	}
	if ((mq_send(addr->conn.mqueue, "Connect to SRV", sizeof("Connect to SRV")+1, NO_PRIORITY)) == -1){
		perror("FALLO CONNECT SEND");	
		return -1;
	}
	return 0;
}

int transport_listen(struct transport_addr * lsten){
	return lsten->conn.mqueue;
}

int transport_close(struct transport_addr * addr){
	return mq_close(addr->conn.mqueue);
}

int transport_serv_init(struct transport_addr * addr){
	attr.mq_maxmsg = QUEUE_SIZE;
	attr.mq_msgsize = MSG_SIZE;
	addr->conn.mqueue = mq_open(SRV_NAME, O_RDWR|O_CREAT, 0666, &attr);
	if (addr->conn.mqueue == -1){
		perror("FALLO SRV INIT");
		return -1;
	}	
	return 0;
}

int transport_accept(struct transport_addr * listen, struct transport_addr *accepts){
	char buffer[50];
	char cli_name[100];
	int read;
	if ((read = mq_receive(listen->conn.mqueue, buffer, MSG_SIZE+1, NO_PRIORITY)) == -1){
		perror("PETIO RCV BLOCK");
	}
	printf("LEYO %s DEL BUFFER", buffer);
	sprintf(cli_name, "/ATC_Client_%d", mq_count++);
	accepts->conn.mqueue = mq_open(cli_name, O_CREAT|O_RDWR, 0666, &attr);
	if (accepts->conn.mqueue == -1){
		perror("PETIO CLI CONN");
		return -1;
	}
	return 0;
}
