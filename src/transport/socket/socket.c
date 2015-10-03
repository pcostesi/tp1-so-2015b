#include "transport.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT "8080"
#define IP "localhost"


int transport_send(struct transport_addr * addr, unsigned char * buffer, size_t size)
{
	int total_sent = 0;
	int bytesleft = size;
	int sent = 0;
	while(total_sent < size && sent != -1) {
		sent = send(addr->conn.sockfd, buffer+total_sent, bytesleft, 0);
		total_sent += sent;
		bytesleft -= sent;
	}
	return total_sent==size ? 0:-1;
}


int transport_recv(struct transport_addr * addr, unsigned char * buffer, size_t size)
{
	return recv(addr->conn.sockfd, buffer, size, MSG_WAITALL);
}


int transport_connect(struct transport_addr * addr)
{
	struct addrinfo address, *servinfo, *p;
	int status;
	int sockfd;


	memset(&address, 0, sizeof address);
	address.ai_family = AF_UNSPEC;
	address.ai_socktype = SOCK_STREAM;

	/*Primeri: ip que queremos conectar, segundo el puerto, */
	if ((status = getaddrinfo(IP, PORT, &address, &servinfo)) != 0) {
		return -1;
	}

	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			continue;
		}
		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			continue;
		}
		break;
	}

	if (p == NULL) {
		return -1;
	}

	addr->type = transport_conn_socket;
	addr->conn.sockfd = sockfd;
	return 0;
}


int transport_listen(struct transport_addr * addr)
{
	return listen(addr->conn.sockfd, 20);
}


int transport_close(struct transport_addr * addr)
{
	return close(addr->conn.sockfd);
}


int transport_accept(struct transport_addr * listen, struct transport_addr *accepts)
{
	int new_fd;
	struct sockaddr_storage their_addr;
	socklen_t addr_size;

	addr_size = sizeof(their_addr);
	new_fd = accept(listen->conn.sockfd, (struct sockaddr *)&their_addr, &addr_size);
	accepts->type = transport_conn_socket;
	accepts->conn.sockfd = new_fd;
	return new_fd;
}


int transport_serv_init(struct transport_addr * addr)
{
	
	int sockfd;
	struct addrinfo hints, *servinfo, *p; 
	int yes=1;	

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	

	if ((getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		return -1;
	}
	

	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
			p->ai_protocol)) == -1) {
			continue;
		}
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
			return -1;
		}
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			continue;
		}
		break;
	}

	freeaddrinfo(servinfo);
	if (p == NULL) {
		return -1;
	}

	addr->type = transport_conn_socket;
	addr->conn.sockfd = sockfd;

	/*save in addr*/
	return 0;
}
