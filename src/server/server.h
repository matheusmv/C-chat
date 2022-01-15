#ifndef _SERVER_H
#define _SERVER_H

#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "../includes/buffer.h"
#include "../includes/network.h"

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

void start_server(const uint16_t port);

#endif