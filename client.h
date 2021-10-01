#ifndef _CLIENT_H
#define _CLIENT_H

#include "network.h"

void conect_to_server(const char *, const uint16_t, const char *);
static int server_auth(const int, const char *);
static void *client_send_thr(void *);
static void *client_recv_thr(void *);
static void send_message(const uint16_t, const char *);

#endif