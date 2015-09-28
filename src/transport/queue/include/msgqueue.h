#ifndef __MSGQ
#define __MSGQ 1

#define SRV_NAME "/ATC_Server"
#define NO_PRIORITY 0
#define QUEUE_SIZE 1
#define MSG_SIZE 2048
#define LISTEN_ALL 0

struct msg_info{
	long mtype;
	unsigned char * mtext;
};

#endif