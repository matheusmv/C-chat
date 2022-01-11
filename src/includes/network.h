#ifndef _NETWORK_H
#define _NETWORK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int
#define GETSOCKETERRNO() (errno)

#define BUFFER_SIZE 1024

#define SUCCESS_MESSSAGE ":success:"
#define DISCONNECT ":exit:"

SOCKET connect_to_server(const char *address, const uint16_t port);
SOCKET create_server(const uint16_t port);

#endif