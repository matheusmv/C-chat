#include "server.h"
#include <signal.h>

struct client {
        SOCKET socket;
        char *address;
        uint16_t port;
        pthread_t tid;
        char username[BUFFER_SIZE];
        char message[BUFFER_SIZE];
};

static void handle_connections(SOCKET *);
static void increase_total_connections();
static void decrease_total_connections();
static int client_auth(struct client *);
static struct client *register_client(struct client *);
static void *server_thread_func(void *);
static void list_online_clients(struct client *);
static void build_message(const struct client *, char *, size_t);
static void get_current_time(char *, size_t);
static void send_public_message(struct client *);
static int extract_username_and_message(const char *, char *, char *);
static void send_private_message(struct client *);
static void disconnect_client(struct client *);
static void send_message(const uint16_t, const char *);

pthread_mutex_t MUTEX = PTHREAD_MUTEX_INITIALIZER;
struct client CONNECTED_CLIENTS[MAX_CONNECTIONS];
static int TOTAL_CONNECTIONS = 0;
static int RUN = 1;

void start_server(const uint16_t port)
{
        SOCKET s_socket = create_server(port);

        handle_connections(&s_socket);
}

static void sigint_handler(int signum)
{
        if (TOTAL_CONNECTIONS > 0) {
                LOG_WARNING("there are still users connected to the server.");
        } else {
                RUN = 0;
        }
}

static void handle_connections(SOCKET *s_socket)
{
        if (s_socket == NULL) {
                LOG_ERROR("invalid parameters");
                exit(EXIT_FAILURE);
        }

        struct sigaction action;
        action.sa_flags = 0;
        sigemptyset(&action.sa_mask);
        action.sa_handler = sigint_handler;
        sigaction(SIGINT, &action, NULL);

        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

        struct sockaddr_in client_details;
        struct client new_client;

        while (RUN) {
                SOCKET c_socket = accept_new_client(s_socket, &client_details);
                if (!ISVALIDSOCKET(c_socket)) {
                        LOG_ERROR("accept_new_client() failed. %s", strerror(errno));
                        continue;
                }

                memset(&new_client, 0, sizeof(new_client));
                new_client.socket = c_socket;
                new_client.address = inet_ntoa(client_details.sin_addr);
                new_client.port = ntohs(client_details.sin_port);

                if (TOTAL_CONNECTIONS == MAX_CONNECTIONS) {
                        send_message(new_client.socket, MAX_LIMIT_MESSAGE);
                        disconnect_client(&new_client);
                        continue;
                }

                if (client_auth(&new_client) > 0) {
                        struct client *arg = register_client(&new_client);
                        pthread_create(&new_client.tid, &attr, server_thread_func, (void *) arg);
                }
        }

        CLOSESOCKET(*s_socket);
        pthread_attr_destroy(&attr);
        pthread_mutex_destroy(&MUTEX);
        exit(EXIT_SUCCESS);
}

static void increase_total_connections()
{
        TOTAL_CONNECTIONS++;
}

static void decrease_total_connections()
{
        if (TOTAL_CONNECTIONS > 0) {
                TOTAL_CONNECTIONS--;
        }
}

static int client_auth(struct client *client)
{
        int auth_status = -1;

        memset(client->username, 0, sizeof(client->message));
        memset(client->message, 0, sizeof(client->message));

        if (recv(client->socket, client->message, sizeof(client->message), 0) <= 0) {
                LOG_ERROR("recv() failed. %s", strerror(errno));
                return auth_status;
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
        pthread_mutex_lock(&MUTEX);
        int client_index;

        for (client_index = 0; client_index < MAX_CONNECTIONS; client_index++) {
                if (CONNECTED_CLIENTS[client_index].socket <= 0) {
                        memmove(&CONNECTED_CLIENTS[client_index], client, sizeof(struct client));
                        increase_total_connections();
                        break;
                }
        }
        pthread_mutex_unlock(&MUTEX);

        return &CONNECTED_CLIENTS[client_index];
}

static void *server_thread_func(void *arg)
{
        struct client *client = arg;

        LOG_INFO("Client connected - IP: %s PORT: %d", client->address, client->port);

        int status = 0;
        while ((status = recv(client->socket, client->message, sizeof(client->message), 0))) {
                if (status < 0) {
                        LOG_ERROR("recv() failed. %s", strerror(errno));
                        break;
                }

                if (strncmp(client->message, SEND_PRIVATE_MESSAGE, strlen(SEND_PRIVATE_MESSAGE)) == 0) {
                        send_private_message(client);
                        continue;
                }

                if (strncmp(client->message, LIST_ONLINE_CLIENTS, strlen(LIST_ONLINE_CLIENTS)) == 0) {
                        list_online_clients(client);
                        continue;
                }

                if (strncmp(client->message, DISCONNECT, strlen(DISCONNECT)) == 0) {
                        send_message(client->socket, DISCONNECT);
                        disconnect_client(client);
                        return NULL;
                }

                if (!strncmp(client->message, "\n", strlen("\n")) == 0) {
                        printf("[%s:%d] %s >>> %s",
                               client->address, client->port,
                               client->username, client->message);

                        send_public_message(client);
                }
        }

        disconnect_client(client);

        return NULL;
}

static void list_online_clients(struct client *client)
{
        if (TOTAL_CONNECTIONS > 1) {
                const size_t message_size = 3 * BUFFER_SIZE * MAX_CONNECTIONS;

                char message[message_size];
                char buffer[message_size];
                memset(message, 0, sizeof(message));
                memset(buffer, 0, sizeof(buffer));

                for (int i = 0; i < MAX_CONNECTIONS; i++) {
                        if (CONNECTED_CLIENTS[i].socket > 0 &&
                            CONNECTED_CLIENTS[i].socket != client->socket) {
                                snprintf(buffer, message_size, "[%s:%d] %s\r\n",
                                         CONNECTED_CLIENTS[i].address,
                                         CONNECTED_CLIENTS[i].port,
                                         CONNECTED_CLIENTS[i].username);
                                strncat(message, buffer, message_size - 1);
                        }
                }

                send_message(client->socket, message);

                memset(client->message, 0, sizeof(client->message));

                return;
        }

        memset(client->message, 0, sizeof(client->message));
        send_message(client->socket, NO_USERS_ONLINE_MESSAGE);
}

static void build_message(const struct client *client, char *message, size_t message_size)
{
        /* Msg of 'username' ['address':'port']:[hh:mm GMT] 'message' */
        char port[6];
        char sent_at[50];

        memset(port, 0, sizeof(port));
        memset(sent_at, 0, sizeof(sent_at));
        memset(message, 0, message_size);

        strncat(message, "Msg of ",
                (BUFFER_SIZE - strlen(message) - 1));
        strncat(message, client->username,
                (BUFFER_SIZE - strlen(message) - 1));
        strncat(message, " [",
                (BUFFER_SIZE - strlen(message) - 1));
        strncat(message, client->address,
                (BUFFER_SIZE - strlen(message) - 1));
        strncat(message, ":",
                (BUFFER_SIZE - strlen(message) - 1));

        sprintf(port, "%d", client->port);
        strncat(message, port,
                (BUFFER_SIZE - strlen(message) - 1));
        strncat(message, "]:",
                (BUFFER_SIZE - strlen(message) - 1));
        strncat(message, "[",
                (BUFFER_SIZE - strlen(message) - 1));

        get_current_time(sent_at, sizeof(sent_at));
        strncat(message, sent_at,
                (BUFFER_SIZE - strlen(message) - 1));
        strncat(message, "] ",
                (BUFFER_SIZE - strlen(message) - 1));
        strncat(message, client->message,
                (BUFFER_SIZE - strlen(message) - 1));
}

static void get_current_time(char *buffer, size_t buffer_size)
{
        time_t seconds;
        struct tm time_info;

        seconds = time(NULL);
        gmtime_r(&seconds, &time_info);

        strftime(buffer, buffer_size, "%H:%M %Z", &time_info);
}

static void send_public_message(struct client *client)
{
        char message[BUFFER_SIZE];
        memset(message, 0, sizeof(message));

        build_message(client, message, sizeof(message));

        for (int i = 0; i < MAX_CONNECTIONS; i++) {
                if (CONNECTED_CLIENTS[i].socket > 0 &&
                    CONNECTED_CLIENTS[i].socket != client->socket) {
                        send_message(CONNECTED_CLIENTS[i].socket, message);
                }
        }

        memset(client->message, 0, sizeof(client->message));
}

static int extract_username_and_message(const char *message_rcvd, char *username, char *message)
{
        const char *utoken = "-u ";
        const char *mtoken = "-m ";

        char *usr = strstr(message_rcvd, utoken);
        char *msg = strstr(message_rcvd, mtoken);
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

        for (int i = usr_start, j = 0; i < usr_end; i++, j++) {
                username[j] = usr[i];
        }

        for (int i = msg_start, j = 0; i < msg_end; i++, j++) {
                message[j] = msg[i];
        }

        return 0;
}

static void send_private_message(struct client *client)
{
        if (TOTAL_CONNECTIONS < 2) {
                memset(client->message, 0, sizeof(client->message));
                send_message(client->socket, NO_USERS_ONLINE_MESSAGE);
                return;
        }

        int status = -1;

        char message[BUFFER_SIZE];
        char dest_username[BUFFER_SIZE];
        memset(message, 0, sizeof(message));
        memset(dest_username, 0, sizeof(dest_username));

        status = extract_username_and_message(client->message, dest_username, message);
        if (status < 0) {
                memset(client->message, 0, sizeof(client->message));
                send_message(client->socket, INVALID_MESSAGE_FORMAT);
                return;
        }

        for (int i = 0; i < MAX_CONNECTIONS; i++) {
                if (strncmp(CONNECTED_CLIENTS[i].username, dest_username,
                            sizeof(CONNECTED_CLIENTS[i].username)) == 0) {

                        memset(client->message, 0, sizeof(client->message));
                        strncpy(client->message, message, sizeof(client->message));
                        build_message(client, message, sizeof(message));
                        send_message(CONNECTED_CLIENTS[i].socket, message);

                        status = 1;

                        break;
                }
        }

        memset(client->message, 0, sizeof(client->message));

        if (status < 1) {
                send_message(client->socket, PRIVATE_MESSAGE_FAILURE);
        }
}

static void disconnect_client(struct client *client)
{
        pthread_mutex_lock(&MUTEX);
        LOG_INFO("Client disconnects - IP: %s PORT: %d", client->address, client->port);

        CLOSESOCKET(client->socket);

        for (int i = 0; i < MAX_CONNECTIONS; i++) {
                if (CONNECTED_CLIENTS[i].socket == client->socket) {
                        memset(&CONNECTED_CLIENTS[i], 0, sizeof(struct client));

                        decrease_total_connections();

                        break;
                }
        }
        pthread_mutex_unlock(&MUTEX);
}

static void send_message(const uint16_t socketfd, const char *message)
{
        int status = 0;

        status = send(socketfd, message, strlen(message), 0);
        if (status < 0) {
                LOG_ERROR("send() failed. %s", strerror(errno));
        }
}
