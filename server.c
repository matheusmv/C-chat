#include "server.h"

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

typedef struct client Client;

Client CONNECTED_CLIENTS[MAX_CONNECTIONS];

static int TOTAL_CONNECTIONS = 0;

static void increase_total_connections()
{
        TOTAL_CONNECTIONS++;
}

static void decrement_total_connections()
{
        if (TOTAL_CONNECTIONS > 0) {
                TOTAL_CONNECTIONS--;
        }
}

void start_server(const uint16_t port)
{
        struct sockaddr_in server;

        memset(&server, 0, sizeof(server));

        server.sin_addr.s_addr = INADDR_ANY;
        server.sin_family = AF_INET;
        server.sin_port = htons(port);

        SOCKET s_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        if (!ISVALIDSOCKET(s_socket)) {
                fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
                exit(EXIT_FAILURE);
        }

        if (bind(s_socket, (struct sockaddr *) &server, sizeof(server)) < 0) {
                fprintf(stderr, "bind() failed. (%d)\n", GETSOCKETERRNO());
                exit(EXIT_FAILURE);
        }

        if (listen(s_socket, BACKLOG) < 0) {
                fprintf(stderr, "listen() failed. (%d)\n", GETSOCKETERRNO());
                exit(EXIT_FAILURE);
        }

        printf("server running on port %d\n", ntohs(server.sin_port));

        struct sockaddr_in client_details;

        int addrlen = sizeof(struct sockaddr_in);

        Client *new_client = NULL;

        while (1)
        {
                if (new_client != NULL) {
                        free(new_client);
                        new_client = NULL;
                }

                memset(&client_details, 0, sizeof(client_details));

                int c_socket = accept(
                        s_socket,
                        (struct sockaddr *) &client_details,
                        (socklen_t *) &addrlen);

                if (!ISVALIDSOCKET(c_socket)) {
                        fprintf(stderr, "accept() failed. (%d)\n", GETSOCKETERRNO());
                        exit(EXIT_FAILURE);
                }

                new_client = malloc(sizeof(Client));

                new_client->socket = c_socket;
                new_client->address = inet_ntoa(client_details.sin_addr);
                new_client->port = ntohs(client_details.sin_port);

                if (TOTAL_CONNECTIONS == MAX_CONNECTIONS) {
                        send_message(new_client->socket, MAX_LIMIT_MESSAGE);
                        disconnect_client(new_client);
                        continue;
                }

                if (client_auth(new_client) > 0) {
                        pthread_attr_t attr;
                        pthread_attr_init(&attr);
                        pthread_attr_setdetachstate(&attr,
                                                    PTHREAD_CREATE_DETACHED);
                        pthread_create(&new_client->tid,
                                       &attr,
                                       server_thread_func,
                                       (void *) register_client(new_client));
                        pthread_attr_destroy(&attr);
                }
        }
}

static int client_auth(struct client *client)
{
        int auth_status = -1;
        char client_message[BUFFER_SIZE];

        if (recv(client->socket, client->message, sizeof(client->message), 0) < 0) {
                fprintf(stderr, "recv() failed. (%d)\n", GETSOCKETERRNO());
                exit(EXIT_FAILURE);
        }

        strncpy(client->username, client->message, sizeof(client->username));
        memset(client->message, 0, sizeof(client->message));

        for (int i = 0; i < MAX_CONNECTIONS; i++) {
                if (strcmp(CONNECTED_CLIENTS[i].username, client->username) == 0) {
                        send_message(client->socket, AUTH_FAILURE_MESSAGE);
                        disconnect_client(client);
                        return auth_status;
                }
        }

        send_message(client->socket, SUCCESS_MESSSAGE);
        auth_status = 1;

        return auth_status;
}

static struct client *register_client(struct client *client)
{
        for (int i = 0; i < MAX_CONNECTIONS; i++) {
                if (CONNECTED_CLIENTS[i].socket <= 0) {
                        CONNECTED_CLIENTS[i] = *client;
                        increase_total_connections();
                        return &CONNECTED_CLIENTS[i];
                }
        }
}

static void *server_thread_func(void *arg)
{
        Client *client = arg;

        printf("Client connected - IP: %s PORT: %d\n", client->address, client->port);

        while (1)
        {
                if (recv(client->socket, client->message, sizeof(client->message), 0) < 0) {
                        fprintf(stderr, "recv() failed. (%d)\n", GETSOCKETERRNO());
                        disconnect_client(client);
                        client = NULL;
                        return NULL;
                }

                if (strncmp(client->message, DISCONNECT, strlen(DISCONNECT)) == 0) {
                        send_message(client->socket, DISCONNECT);
                        disconnect_client(client);
                        client = NULL;
                        return NULL;
                }

                if (strncmp(client->message, LIST_ONLINE_CLIENTS, strlen(LIST_ONLINE_CLIENTS)) == 0) {
                        list_online_clients(client->socket);
                        memset(client->message, 0, sizeof(client->message));
                        continue;
                }

                if (strncmp(client->message, SEND_PRIVATE_MESSAGE, strlen(SEND_PRIVATE_MESSAGE)) == 0) {
                        send_private_message(client);
                        continue;
                }

                printf("[%s:%d] %s >>> %s",
                       client->address, client->port,
                       client->username, client->message);

                send_public_message(client);
        }

        return NULL;
}

static void list_online_clients(const uint16_t client_socket)
{
        if (TOTAL_CONNECTIONS == 1) {
                send_message(client_socket, NO_USERS_ONLINE_MESSAGE);
        } else {
                char message[BUFFER_SIZE];
                char port[6];

                memset(message, 0, sizeof(message));
                memset(port, 0, sizeof(port));

                for (int i = 0; i < MAX_CONNECTIONS; i++) {
                        if (CONNECTED_CLIENTS[i].socket > 0 && CONNECTED_CLIENTS[i].socket != client_socket) {
                                strncat(message, "[", sizeof("["));
                                strncat(message, CONNECTED_CLIENTS[i].address, strlen(CONNECTED_CLIENTS[i].address));
                                strncat(message, ":", sizeof(":"));
                                sprintf(port, "%d", CONNECTED_CLIENTS[i].port);
                                strncat(message, port, strlen(port));
                                strncat(message, "] ", sizeof("] "));
                                strncat(message, CONNECTED_CLIENTS[i].username, strlen(CONNECTED_CLIENTS[i].username));
                                strncat(message, "\r\n", sizeof("\r\n"));

                                memset(port, 0, sizeof(port));
                        }
                }

                send_message(client_socket, message);
                memset(message, 0, sizeof(message));
                memset(port, 0, sizeof(port));
        }
}

static void build_message(const struct client *client, char *message)
{
        /* Msg of 'username' ['address':'port']:[Fri Oct  1 11:45:12 2021] 'message' */
        char port[6];
        char sent_at[50];

        memset(port, 0, sizeof(port));
        memset(sent_at, 0, sizeof(sent_at));

        time_t now;
        time(&now);

        strncat(sent_at, ctime(&now), sizeof(sent_at) - strlen(sent_at) - 1);

        strncat(message, "Msg of ", sizeof("Msg of "));
        strncat(message, client->username, strlen(client->username));
        strncat(message, " [", sizeof(" ["));
        strncat(message, client->address, strlen(client->address));
        strncat(message, ":", sizeof(":"));
        sprintf(port, "%d", client->port);
        strncat(message, port, strlen(port));
        strncat(message, "]:", sizeof("]:"));
        strncat(message, "[", sizeof("["));
        strncat(message, sent_at, (strlen(sent_at) - 1));
        strncat(message, "] ", sizeof("] "));
        strncat(message, client->message, strlen(client->message));

        memset(port, 0, sizeof(port));
        memset(sent_at, 0, sizeof(sent_at));
}

static void send_public_message(struct client *client)
{
        char message[BUFFER_SIZE];

        memset(message, 0, sizeof(message));

        build_message(client, message);

        for (int i = 0; i < MAX_CONNECTIONS; i++) {
                if (CONNECTED_CLIENTS[i].socket > 0 && CONNECTED_CLIENTS[i].socket != client->socket) {
                        send_message(CONNECTED_CLIENTS[i].socket, message);
                }
        }

        memset(message, 0, sizeof(message));
        memset(client->message, 0, sizeof(client->message));
}

static void extract_username_and_message(const char *message_rcvd, char *username, char *message)
{
        const int message_length = strlen(message_rcvd);

        /* :send: <message> username */
        for (int i = 0; i < message_length; i++) {
                if (message_rcvd[i] == '<') {
                        i++;
                        int aux = 0;

                        while (message_rcvd[i] != '>')
                        {
                                message[aux++] = message_rcvd[i++];
                        }

                        strncat(message, "\r\n", sizeof("\r\n"));
                }

                if (message_rcvd[i] == '>') {
                        i += 2;
                        int aux = 0;

                        while (message_rcvd[i] != '\r')
                        {
                                username[aux++] = message_rcvd[i++];
                        }
                }
        }
}

static void send_private_message(struct client *client)
{
        if (TOTAL_CONNECTIONS == 1) {
                send_message(client->socket, NO_USERS_ONLINE_MESSAGE);
        } else {
                int status = -1;
                char message[BUFFER_SIZE];
                char recipient_username[50];

                memset(message, 0, sizeof(message));
                memset(recipient_username, 0, sizeof(recipient_username));

                extract_username_and_message(client->message, recipient_username, message);

                memset(client->message, 0, sizeof(client->message));

                for (int i = 0; i < MAX_CONNECTIONS; i++) {
                        if (strncmp(CONNECTED_CLIENTS[i].username, recipient_username, strlen(recipient_username)) == 0) {
                                strncpy(client->message, message, sizeof(message));
                                memset(message, 0, sizeof(message));
                                build_message(client, message);
                                send_message(CONNECTED_CLIENTS[i].socket, message);
                                status = 1;
                                break;
                        }
                }

                memset(message, 0, sizeof(message));
                memset(recipient_username, 0, sizeof(recipient_username));
                memset(client->message, 0, sizeof(client->message));

                if (status < 0) {
                        send_message(client->socket, PRIVATE_MESSAGE_FAILURE);
                }
        }
}

static void disconnect_client(struct client *client)
{
        printf("client disconnects - IP: %s PORT: %d\n", client->address, client->port);

        shutdown(client->socket, SHUT_RDWR);
        close(client->socket);

        for (int i = 0; i < MAX_CONNECTIONS; i++) {
                if (CONNECTED_CLIENTS[i].socket == client->socket) {
                        memset(CONNECTED_CLIENTS[i].username, 0, sizeof(CONNECTED_CLIENTS[i].username));
                        CONNECTED_CLIENTS[i].address = "";
                        CONNECTED_CLIENTS[i].port = 0;
                        CONNECTED_CLIENTS[i].socket = 0;
                        memset(CONNECTED_CLIENTS[i].message, 0, sizeof(CONNECTED_CLIENTS[i].message));

                        decrement_total_connections();

                        break;
                }
        }
}

static void send_message(const uint16_t client_socket, const char *message)
{
        if (send(client_socket, message, strlen(message), 0) < 0) {
                fprintf(stderr, "send() failed. (%d)\n", GETSOCKETERRNO());
                exit(EXIT_FAILURE);
        };
}

int main(int argc, char *argv[])
{
        if (argc < 2) {
                fprintf(stderr, "usage: ./server server_port\n");
                exit(EXIT_FAILURE);
        }

        start_server((uint16_t) atoi(argv[1]));

        return EXIT_SUCCESS;
}
