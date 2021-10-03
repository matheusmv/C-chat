#ifndef _SERVER_H
#define _SERVER_H

#include "network.h"

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define BACKLOG 5
#define MAX_CONNECTIONS 5

/* error messages */
#define MAX_LIMIT_MESSAGE "limit of simultaneous connections reached.\r\n"
#define AUTH_FAILURE_MESSAGE "username already registered, try again.\r\n"
#define NO_USERS_ONLINE_MESSAGE "there are no users online.\r\n"
#define PRIVATE_MESSAGE_FAILURE "user is not currently online or username is invalid.\r\n"

/* commands */
#define LIST_ONLINE_CLIENTS ":list:"
#define SEND_PRIVATE_MESSAGE ":send:"

struct client {
        char username[BUFFER_SIZE];
        char *address;
        uint16_t port;
        SOCKET socket;
        char message[BUFFER_SIZE];
        pthread_t tid;
};

static void increase_total_connections();
static void decrement_total_connections();
void start_server(const uint16_t);
static int client_auth(struct client *);
static struct client *register_client(struct client *);
static void *server_thread_func(void *);
static void list_online_clients(struct client *);
static void build_message(const struct client *, char *, size_t);
static void get_current_time(char *, size_t);
static void send_public_message(struct client *);
static void send_private_message(struct client *);
static void disconnect_client(struct client *);
static void send_message(const uint16_t, const char *);

#endif