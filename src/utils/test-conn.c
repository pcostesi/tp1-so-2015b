#include "protocol.h"
#include "atcd.h"
#include <stdio.h>
#include <string.h>

int server(void);
int client(void);
static int handle_server_response(struct atc_conn * conn);

int main(int argc, char ** argv)
{
	if (argc < 2) {
		printf("Usage: %s [server|client]\n", argv[0]);
		return 1;
	}

	if (strcmp(argv[1], "server") == 0) {
		return server();
	} else if (strcmp(argv[1], "client") == 0) {
		return client();
	} else {
		printf("Invalid option: %s\n", argv[1]);
		return 1;
	}
	return -1;
}


int server(void)
{
	struct atc_conn server;
	struct atc_conn client;
	puts("Starting server.");
	atc_listen(&server);
	puts("Waiting for clients.");
	while (atc_accept(&server, &client)) {
		switch (fork()) {
			case 0:
			puts("connected.");
			atc_reply(&client, (atc_reply_handler) handle_server_response);
			atc_close(&client);
			break;

			case -1:
			return -1;

			default:
			atc_close(&client);
		}	
	}
	atc_close(&server);
	return 0;
}


int client(void)
{
	struct atc_conn client;
	int idx;
	puts("Starting client.");
	if (atc_connect(&client) == -1) {
		puts("Failed to connect.");
		return -1;
	}
	puts("Requesting");
	client.req.type = atc_join;
	atc_request(&client);
	printf("Response type is %c\n", (char) client.res.type);
	printf("Plane count is %d\n", client.res.len.planes);
	for (idx = 0; idx < client.res.len.planes; idx++) {
		printf("Plane id: %s\n", client.res.msg.planes[idx].id);
	}
	puts("Ready.");
	atc_close(&client);
	return 0;
}


static int handle_server_response(struct atc_conn * conn)
{
	struct atc_req * req;
	struct atc_res * res;
	struct atc_plane planes[2];
	memset(planes, 0, sizeof(planes));
	strncpy(planes[0].id, "HHD035", 6);
	strncpy(planes[1].id, "ECT620", 6);

	req = &conn->req;
	res = &conn->res;

	memcpy(res->msg.planes, planes, sizeof(planes));

	printf("- Received request %c.\n", (char) req->type);
	res->type = atc_planes;
	res->len.planes = sizeof(planes) / sizeof(struct atc_plane);
	printf("- Replying with %d planes\n", res->len.planes);
	return 0;
}

