#ifndef _SERVER_H
#define _SERVER_H

#include "../includes/network.h"

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define MAX_CONNECTIONS 5

/* error messages */
#define MAX_LIMIT_MESSAGE "limit of simultaneous connections reached.\r\n"
#define AUTH_FAILURE_MESSAGE "username already registered, try again.\r\n"
#define NO_USERS_ONLINE_MESSAGE "there are no users online.\r\n"
#define PRIVATE_MESSAGE_FAILURE "user is not currently online or username is invalid.\r\n"
#define INVALID_MESSAGE_FORMAT "usage: :send: -u username -m message.\r\n"

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

void start_server(const uint16_t port);

#endif