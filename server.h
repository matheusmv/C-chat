#ifndef _SERVER_H
#define _SERVER_H

#include "network.h"

#define BACKLOG 5
#define MAX_CONNECTIONS 5

#define MAX_LIMIT_MESSAGE "limit of simultaneous connections reached.\r\n"
#define AUTH_FAILURE_MESSAGE "username already registered, try again.\r\n"
#define NO_USERS_ONLINE_MESSAGE "there are no users online.\r\n"
#define PRIVATE_MESSAGE_FAILURE "user is not currently online or username is invalid.\r\n"

#define LIST_ONLINE_CLIENTS ":list:\r\n"
#define SEND_PRIVATE_MESSAGE ":send:"

struct client {
        char username[50];
        char *address;
        uint16_t port;
        SOCKET socket;
        char message[BUFFER_SIZE];
        struct sockaddr_in client_details;
};

static void init_clients();
static void increase_total_connections();
static void decrement_total_connections();
void start_server(const uint16_t);
static int client_auth(struct client *);
static struct client *register_client(struct client *);
static void *server_thread_func(void *);
static void list_online_clients(const uint16_t);
static void build_message(const struct client *, char *);
static void send_public_message(struct client *);
static void send_private_message(struct client *);
static void disconnect_client(struct client *);
static void send_message(const uint16_t, const char *);

#endif