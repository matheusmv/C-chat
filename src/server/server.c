#include "server.h"
#include "../includes/clogger.h"

#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define true  1
#define false 0

typedef struct client {
        pthread_t thread;
        uint16_t  port;
        char      *address;
        SOCKET    tcp_socket;
        char      username[BUFFER_SIZE];
        char      message[BUFFER_SIZE];
} client_t;

typedef struct server {
        SOCKET          tcp_socket;
        int8_t          running;
        int8_t          initialized;
        int32_t         connected_clients;
        client_t        clients[MAX_CONNECTIONS];
        pthread_mutex_t mutex;
} server_t;

/* client */

static client_t client_create(const SOCKET *tcp_socket, const struct sockaddr_in *net_detatils);
static void *client_thread_fn(void *arg);

/* server */

static server_t server_create(SOCKET *socket);
static void server_mutex_init(server_t *server);
static void server_mutex_lock(server_t *server);
static void server_mutex_unlock(server_t *server);
static void server_mutex_destroy(server_t *server);
static void server_increase_connected_clients(server_t *server);
static void server_decrease_connected_clients(server_t *server);
static int server_authenticate_client(server_t *server, client_t *client);
static client_t *server_register_authenticated_client(server_t *server, client_t *client);
static void server_list_online_clients(server_t *server, client_t *client);
static void server_send_private_message(server_t *server, client_t *client);
static void server_send_public_message(server_t *server, client_t *client);
static void server_disconnect_client(server_t *server, client_t *client);

/* utils */

static void sigint_handler(int signum);
static void handle_connections(server_t * server);
static void build_message(const client_t *client, char *buffer, size_t size);
static void get_current_time(char *buffer, size_t size);
static int extract_username_and_message(const char *src, char *username, size_t usize, char *message, size_t msize);

static server_t prv_server;

void
start_server(const uint16_t port)
{
        SOCKET s_socket = server_start_listening_on_port(port);
        if (!ISVALIDSOCKET(s_socket)) {
                LOG_ERROR("create_server_socket() failed. %s", strerror(errno));
                exit(EXIT_FAILURE);
        }

        prv_server = server_create(&s_socket);

        handle_connections(&prv_server);
}

/* client impl */

static client_t
client_create(const SOCKET *socket, const struct sockaddr_in *net_detatils)
{
        client_t new_client = (client_t) {
                .tcp_socket     = *socket,
                .port           = ntohs(net_detatils->sin_port),
                .address        = inet_ntoa(net_detatils->sin_addr)
        };

        return new_client;
}

static void *
client_thread_fn(void *arg)
{
        client_t *client = arg;

        LOG_INFO("Client connected - IP: %s PORT: %d", client->address, client->port);

        int status = 0;
        while ((status = receive_message(&client->tcp_socket, client->message, BUFFER_SIZE))) {
                if (status < 0) {
                        LOG_ERROR("receive_message() failed. %s", strerror(errno));
                        break;
                }

                char *msg = client->message;

                if (strncmp(msg, DISCONNECT, strlen(DISCONNECT)) == 0) {
                        send_message(&client->tcp_socket, DISCONNECT, strlen(DISCONNECT));
                        server_disconnect_client(&prv_server, client);
                        return NULL;
                }

                if (strncmp(msg, LIST_ONLINE_CLIENTS, strlen(LIST_ONLINE_CLIENTS)) == 0) {
                        server_list_online_clients(&prv_server, client);
                        continue;
                }

                if (strncmp(msg, SEND_PRIVATE_MESSAGE, strlen(SEND_PRIVATE_MESSAGE)) == 0) {
                        server_send_private_message(&prv_server, client);
                        continue;
                }

                if (!strncmp(msg, "\n", strlen("\n")) == 0) {
                        server_send_public_message(&prv_server, client);
                        continue;
                }
        }

        server_disconnect_client(&prv_server, client);

        return NULL;
}

/* server impl */

static server_t
server_create(SOCKET *socket)
{
        server_t new_server = (server_t) {
                .tcp_socket     = *socket,
                .running        = true,
                .initialized    = true,
        };

        server_mutex_init(&new_server);

        return new_server;
}

static void
server_mutex_init(server_t *server)
{
        pthread_mutex_init(&server->mutex, NULL);
}

static void
server_mutex_lock(server_t *server)
{
        pthread_mutex_lock(&server->mutex);
}

static void
server_mutex_unlock(server_t *server)
{
        pthread_mutex_unlock(&server->mutex);
}

static void
server_mutex_destroy(server_t *server)
{
        pthread_mutex_destroy(&server->mutex);
}

static void
server_increase_connected_clients(server_t *server)
{
        server->connected_clients += 1;
}

static void
server_decrease_connected_clients(server_t *server)
{
        if (server->connected_clients > 0) {
                server->connected_clients -= 1;
        }
}

static int
server_authenticate_client(server_t *server, client_t *client)
{
        server_mutex_lock(server);

        int successfully_authenticated = false;

        memset(client->username, 0, BUFFER_SIZE);
        memset(client->message, 0, BUFFER_SIZE);

        int status = 0;
        status = receive_message(&client->tcp_socket, client->message, BUFFER_SIZE);
        if (status < 0) {
                LOG_ERROR("receive_message() failed. %s", strerror(errno));
                server_mutex_unlock(server);
                return successfully_authenticated;
        }

        memmove(client->username, client->message, BUFFER_SIZE);
        memset(client->message, 0, BUFFER_SIZE);

        for (int i = 0; i < MAX_CONNECTIONS; ++i) {
                char *name = server->clients[i].username;

                if (strcmp(name, client->username) == 0) {
                        send_message(&client->tcp_socket, AUTH_FAILURE_MESSAGE, strlen(AUTH_FAILURE_MESSAGE));
                        server_mutex_unlock(server);
                        server_disconnect_client(server, client);
                        return successfully_authenticated;
                }
        }

        send_message(&client->tcp_socket, SUCCESS_MESSSAGE, strlen(SUCCESS_MESSSAGE));

        successfully_authenticated = true;

        server_mutex_unlock(server);

        return successfully_authenticated;
}

static client_t *
server_register_authenticated_client(server_t *server, client_t *client)
{
        server_mutex_lock(server);

        int index;
        for (index = 0; index < MAX_CONNECTIONS; ++index) {
                SOCKET socket = server->clients[index].tcp_socket;

                if (!ISACTIVESOCKET(socket)) {
                        memmove(&server->clients[index], client, sizeof(client_t));
                        server_increase_connected_clients(server);
                        break;
                }
        }

        server_mutex_unlock(server);

        return &server->clients[index];
}

static void
server_list_online_clients(server_t *server, client_t *client)
{
        server_mutex_lock(server);

        if (server->connected_clients < 2) {
                memset(client->message, 0, BUFFER_SIZE);
                send_message(&client->tcp_socket, NO_USERS_ONLINE_MESSAGE, strlen(NO_USERS_ONLINE_MESSAGE));
                server_mutex_unlock(server);
                return;
        }

        buffer *buff = new_buffer(5 * BUFFER_SIZE);

        for (int i = 0; i < MAX_CONNECTIONS; ++i) {
                SOCKET socket = server->clients[i].tcp_socket;

                if (ISACTIVESOCKET(socket) && socket != client->tcp_socket) {
                        char *address = server->clients[i].address;
                        uint16_t port = server->clients[i].port;
                        char *username = server->clients[i].username;

                        buffer_appendf(buff, "[%s:%d] %s\r\n", address, port, username);
                }
        }

        char *msg = buffer_to_string(buff);

        send_message(&client->tcp_socket, msg, strlen(msg));

        buffer_free(buff);
        free(msg);

        memset(client->message, 0, BUFFER_SIZE);

        server_mutex_unlock(server);
}

static void
server_send_private_message(server_t *server, client_t *client)
{
        server_mutex_lock(server);

        if (server->connected_clients < 2) {
                memset(client->message, 0, BUFFER_SIZE);
                send_message(&client->tcp_socket, NO_USERS_ONLINE_MESSAGE, strlen(NO_USERS_ONLINE_MESSAGE));
                server_mutex_unlock(server);
                return;
        }

        char recipient_username[BUFFER_SIZE];
        char message_to_recipient[BUFFER_SIZE];

        int status = 0;
        status = extract_username_and_message(client->message, recipient_username, BUFFER_SIZE, message_to_recipient, BUFFER_SIZE);
        if (status < 0) {
                memset(client->message, 0, BUFFER_SIZE);
                send_message(&client->tcp_socket, INVALID_MESSAGE_FORMAT, strlen(INVALID_MESSAGE_FORMAT));
                server_mutex_unlock(server);
                return;
        }

        int message_sent = false;

        for (int i = 0; i < MAX_CONNECTIONS; ++i) {
                char *username = server->clients[i].username;

                if (strncmp(username, recipient_username, BUFFER_SIZE) == 0) {
                        memset(client->message, 0, BUFFER_SIZE);
                        strncpy(client->message, message_to_recipient, BUFFER_SIZE);
                        build_message(client, message_to_recipient, BUFFER_SIZE);
                        send_message(&server->clients[i].tcp_socket, message_to_recipient, strlen(message_to_recipient));
                        message_sent = true;
                        break;
                }
        }

        memset(client->message, 0, BUFFER_SIZE);

        if (!message_sent) {
                send_message(&client->tcp_socket, PRIVATE_MESSAGE_FAILURE, strlen(PRIVATE_MESSAGE_FAILURE));
        }

        server_mutex_unlock(server);
}

static void
server_send_public_message(server_t *server, client_t *client)
{
        server_mutex_lock(server);

        char message_sent[BUFFER_SIZE];
        build_message(client, message_sent, BUFFER_SIZE);

        for (int i = 0; i < MAX_CONNECTIONS; ++i) {
                SOCKET socket = server->clients[i].tcp_socket;

                if (ISACTIVESOCKET(socket) && socket != client->tcp_socket) {
                        send_message(&socket, message_sent, strlen(message_sent));
                }
        }

        memset(client->message, 0, BUFFER_SIZE);

        server_mutex_unlock(server);
}

static void
server_disconnect_client(server_t *server, client_t *client)
{
        server_mutex_lock(server);

        LOG_INFO("Client disconnects - IP: %s PORT: %d", client->address, client->port);

        CLOSESOCKET(client->tcp_socket);

        for (int i = 0; i < MAX_CONNECTIONS; ++i) {
                SOCKET socket = server->clients[i].tcp_socket;

                if (socket == client->tcp_socket) {
                        memset(&server->clients[i], 0, sizeof(client_t));
                        server_decrease_connected_clients(server);
                        break;
                }
        }

        server_mutex_unlock(server);
}

/* utils impl */

static void
sigint_handler(int signum)
{
        switch (signum)
        {
        case SIGINT:
                if (prv_server.connected_clients > 0) {
                        LOG_WARNING("there are still %d users connected to the server.", prv_server.connected_clients);
                } else {
                        CLOSESOCKET(prv_server.tcp_socket);
                        prv_server.running = false;
                        server_mutex_destroy(&prv_server);
                }
                break;
        default:
                break;
        }
}

static void
handle_connections(server_t *server)
{
        assert(server != NULL);

        struct sigaction action;
        action.sa_flags = 0;
        sigemptyset(&action.sa_mask);
        action.sa_handler = sigint_handler;
        sigaction(SIGINT, &action, NULL);

        pthread_attr_t detached_attr;
        pthread_attr_init(&detached_attr);
        pthread_attr_setdetachstate(&detached_attr, PTHREAD_CREATE_DETACHED);

        struct sockaddr_in client_details;
        client_t new_client;

        while (server->running) {
                SOCKET c_socket = server_accept_new_client(&server->tcp_socket, &client_details);
                if (!ISVALIDSOCKET(c_socket)) {
                        LOG_ERROR("accept_new_client() failed. %s", strerror(errno));
                        continue;
                }

                new_client = client_create(&c_socket, &client_details);

                if (server->connected_clients == MAX_CONNECTIONS) {
                        send_message(&new_client.tcp_socket, MAX_LIMIT_MESSAGE, strlen(MAX_LIMIT_MESSAGE));
                        server_disconnect_client(server, &new_client);
                        continue;
                }

                if (server_authenticate_client(server, &new_client)) {
                        client_t *arg = server_register_authenticated_client(server, &new_client);
                        pthread_create(&arg->thread, &detached_attr, client_thread_fn, (void *) arg);
                }
        }

        pthread_attr_destroy(&detached_attr);

        exit(EXIT_SUCCESS);
}

static void
get_current_time(char *buffer, size_t buffer_size)
{
        time_t seconds;
        struct tm time_info;

        seconds = time(NULL);
        gmtime_r(&seconds, &time_info);

        strftime(buffer, buffer_size, "%H:%M %Z", &time_info);
}

static void
build_message(const client_t *client, char *message, size_t message_size)
{
        /* Msg of 'username' ['address':'port']:[hh:mm GMT] 'message' */
        buffer *buff = new_buffer(5 * BUFFER_SIZE);

        char sent_at[50];
        memset(sent_at, 0, sizeof(sent_at));
        get_current_time(sent_at, sizeof(sent_at));

        const char *format = "Msf of %s [%s:%d]:[%s] %s";
        buffer_appendf(buff, format, client->username, client->address, client->port, sent_at, client->message);

        char *result = buffer_to_string(buff);

        memset(message, 0, message_size);
        memmove(message, result, strlen(result));

        buffer_free(buff);
        free(result);
}

static int
extract_username_and_message(const char *src, char *username, size_t usize, char *message, size_t msize)
{
        const char *utoken = "-u ";
        const char *mtoken = "-m ";

        char *usr = strstr(src, utoken);
        char *msg = strstr(src, mtoken);
        if (usr == NULL || msg == NULL) {
                return -1;
        }

        const int msg_start = strlen(mtoken);
        const int msg_end = strlen(msg);
        const int usr_start = strlen(utoken);
        const int usr_end = strlen(usr) - msg_end - 1;

        if (strlen(usr) < strlen(msg)) {
                return -1;
        }

        memset(username, 0, usize);
        memset(message, 0, msize);

        for (int i = usr_start, j = 0; i < usr_end; ++i, ++j) {
                username[j] = usr[i];
        }

        for (int i = msg_start, j = 0; i < msg_end; ++i, ++j) {
                message[j] = msg[i];
        }

        return 0;
}
