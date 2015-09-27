#include "transport.h"
#include "msgqueue.h"

static struct mq_attr attr = {QUEUE_SIZE, MSG_SIZE};

int transport_send(struct transport_addr * addr, unsigned char * buffer, size_t size){
	struct msg_info msg = {LISTEN_ALL, buffer};
	return mq_send(addr->conn, &msg, size, NO_PRIORITY);
}

int transport_recv(struct transport_addr * addr, unsigned char * buffer, size_t size){
	struct msg_info msg = {LISTEN_ALL, buffer};
	return msgrcv(queueFd, &msg, size, size, NO_PRIORITY);
}

int transport_connect(struct transport_addr * addr){
	addr->conn = mq_open(SRV_NAME, O_RDWR, 0666, &attr));
	if (addr->conn == -1){
		return -1;
	}
	return 0;
}

int transport_listen(struct transport_addr * lsten){
	return lsten->conn;
}

int transport_close(struct transport_addr * addr){
	return mq_close(addr->conn);
}

int transport_serv_init(struct transport_addr * addr){
	addr->conn = mq_open(SRV_NAME, O_RDWR|O_CREAT, 0666, &attr);
	if (addr->conn == -1){
		return -1;
	}
	return 0;
}

int transport_accept(struct transport_addr * listen, struct transport_addr *accepts){
	char cli_name[100];
	sprintf(cli_name, "/ATC_Client_%d", getpid());
	return accepts->conn = mq_open(cli_name, O_RDWR|O_CREAT, 0666, &attr);
}
