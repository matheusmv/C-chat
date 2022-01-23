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

#define ISACTIVESOCKET(s) ((s) > 0)
#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int

#define BUFFER_SIZE 1024

#define SUCCESS_MESSSAGE ":success:"
#define DISCONNECT ":exit:"

SOCKET client_connect_to_server(const char *address, const uint16_t port);
SOCKET server_start_listening_on_port(const uint16_t port);
SOCKET server_accept_new_client(SOCKET *server_socket, struct sockaddr_in *client_details);
int receive_message(const SOCKET *socket, char *buffer, size_t size);
int send_message(const SOCKET *socket, const char *message, size_t length);

#endif
