#include "protocol.h"
#include <assert.h>
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
    if (atc_listen(&server) == -1) {
        perror("listen");
        return -1;
    };
    puts("Waiting for clients.");
    while (atc_accept(&server, &client) != -1) {
        switch (fork()) {
            case 0:
                puts("connected.");
                while (atc_reply(&client, (atc_reply_handler) handle_server_response) != -1) {
                    puts("Sent.");
                };
                puts("Connection closed.");
                atc_close(&client);
                break;

            case -1:
                perror("accept");
                return -1;

            default:
                atc_close(&client);
        }	
    }
    atc_close(&server);
    puts("Quitting.");
    return 0;
}

void request_planes(struct atc_conn * client)
{
    int idx;
    client->req.type = atc_get_planes;
    atc_request(client);
    printf("Response type is %c\n", (char) client->res.type);
    printf("Plane count is %d\n", client->res.len.planes);
    for (idx = 0; idx < client->res.len.planes; idx++) {
        printf("Plane id: %s\n", client->res.msg.planes[idx].id);
    }
}

void request_airports(struct atc_conn * client)
{
    int idx;
    client->req.type = atc_get_airports;
    atc_request(client);
    printf("Response type is %c\n", (char) client->res.type);
    printf("Airport count is %d\n", client->res.len.airports);
    for (idx = 0; idx < client->res.len.airports; idx++) {
        printf("Airport  %3s: x %d, y %d\n",
                client->res.msg.airports[idx].id,
                client->res.msg.airports[idx].x,
                client->res.msg.airports[idx].y);
    }
}

int client(void)
{
    struct atc_conn client;
    puts("Starting client.");
    if (atc_connect(&client) == -1) {
        perror("Failed to connect");
        return -1;
    }
    puts("Requesting");
    request_planes(&client);
    request_airports(&client);
    puts("Ready.");
    client.req.type = atc_leave;
    atc_request(&client);
    puts("Left");
    atc_close(&client);
    return 0;
}


static int handle_airport_request(struct atc_conn * conn)
{
    struct atc_req * req;
    struct atc_res * res;
    struct atc_airport airports[2];
    memset(airports, 0, sizeof(airports));
    strncpy(airports[0].id, "HHD035", 3);
    strncpy(airports[1].id, "ECT620", 3);
    airports[0].x = 1;
    airports[0].y = 1;
    airports[0].x = 100;
    airports[0].y = 100;

    req = &conn->req;
    res = &conn->res;

    memcpy(res->msg.airports, airports, sizeof(airports));

    printf("- Received request %c.\n", (char) req->type);
    if (req->type == atc_leave) {
        return -1;
    }
    res->type = atc_airports;
    res->len.airports = sizeof(airports) / sizeof(struct atc_airport);
    printf("- Replying with %d airports\n", res->len.airports);
    return 0;
}

static int handle_planes_request(struct atc_conn * conn)
{
    struct atc_req * req;
    struct atc_res * res;
    struct atc_plane planes[2];
    memset(planes, 0, sizeof(planes));
    strncpy(planes[0].id, "HHD035", 3);
    strncpy(planes[1].id, "ECT620", 3);
    planes[0].x = 1;
    planes[0].y = 1;
    planes[0].x = 100;
    planes[0].y = 100;

    req = &conn->req;
    res = &conn->res;

    memcpy(res->msg.planes, planes, sizeof(planes));

    printf("- Received request %c.\n", (char) req->type);
    if (req->type == atc_leave) {
        return -1;
    }
    res->type = atc_planes;
    res->len.planes = sizeof(planes) / sizeof(struct atc_plane);
    printf("- Replying with %d planes\n", res->len.planes);
    return 0;
}

static int handle_server_response(struct atc_conn * conn)
{
    struct atc_req * req = &conn->req;
    if (req->type == atc_get_planes) {
        return handle_planes_request(conn);
    } else if (req->type == atc_get_airports) {
        return handle_airport_request(conn);
    }

    puts("Unknown message");
    return -1;
}

