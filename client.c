#include "client.h"

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void conect_to_server(const char *address, const uint16_t port, const char *username)
{
        struct sockaddr_in server;

        memset(&server, 0, sizeof(server));

        server.sin_addr.s_addr = inet_addr(address);
        server.sin_family = AF_INET;
        server.sin_port = htons(port);

        SOCKET c_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        if (!ISVALIDSOCKET(c_socket)) {
                fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
                exit(EXIT_FAILURE);
        }

        if (connect(c_socket, (struct sockaddr *) &server, sizeof(server)) < 0) {
                fprintf(stderr, "connect() failed. (%d)\n", GETSOCKETERRNO());
                exit(EXIT_FAILURE);
        }

        pthread_t sendthread, recvthread;

        if (server_auth(c_socket, username) > 0) {
                pthread_create(&sendthread, NULL, client_send_thr, (void *) &c_socket);
                pthread_create(&recvthread, NULL, client_recv_thr, (void *) &c_socket);

                pthread_join(sendthread, NULL);
                pthread_join(recvthread, NULL);
        }
}

static int server_auth(const int client_socket, const char *username)
{
        int auth_status = -1;

        char server_response[BUFFER_SIZE];

        memset(server_response, 0, sizeof(server_response));

        send(client_socket, username, strlen(username), 0);
        recv(client_socket, server_response, sizeof(server_response), 0);

        if (strncmp(server_response, SUCCESS_MESSSAGE, strlen(SUCCESS_MESSSAGE)) == 0) {
                printf("Login successful!\n");
                auth_status = 1;
        } else {
                printf("%s", server_response);
        }

        return auth_status;
}

static void *client_send_thr(void *arg)
{
        SOCKET *client_socket = arg;

        char client_input[BUFFER_SIZE];
        char final_message[BUFFER_SIZE];

        while (1)
        {
                memset(client_input, 0, sizeof(client_input));
                memset(final_message, 0, sizeof(final_message));

                fgets(client_input, sizeof(client_input), stdin);

                strncat(final_message, client_input, (strlen(client_input) - 1));
                strncat(final_message, "\r\n", sizeof("\r\n"));

                if (send(*client_socket, final_message, strlen(final_message), 0) < 0) {
                        fprintf(stderr, "send() failed. (%d)\n", GETSOCKETERRNO());
                }

                if (strncmp(final_message, DISCONNECT, strlen(DISCONNECT)) == 0) {
                        close(*client_socket);
                        client_socket = NULL;
                        return NULL;
                }
        }

        return NULL;
}

static void *client_recv_thr(void *arg)
{
        SOCKET *client_socket = arg;

        char server_response[BUFFER_SIZE];

        while (1)
        {
                memset(server_response, 0, sizeof(server_response));

                if (recv(*client_socket, server_response, sizeof(server_response), 0) < 0) {
                        fprintf(stderr, "recv() failed. (%d)\n", GETSOCKETERRNO());
                        return NULL;
                }

                if (strncmp(server_response, DISCONNECT, strlen(DISCONNECT)) == 0) {
                        printf("disconnected from server.\n");
                        close(*client_socket);
                        client_socket = NULL;
                        return NULL;
                }

                printf("%s", server_response);
        }

        return NULL;
}

int main(int argc, char *argv[])
{
        if (argc < 4) {
                fprintf(stderr, "usage: ./client server_ip server_port username\n");
                exit(EXIT_FAILURE);
        }

        conect_to_server(argv[1], (uint16_t) atoi(argv[2]), argv[3]);

        return EXIT_SUCCESS;
}
