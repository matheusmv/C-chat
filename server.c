#include "server.h"

typedef struct client Client;

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

Client CONNECTED_CLIENTS[MAX_CONNECTIONS];

static int TOTAL_CONNECTIONS = 0;

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

        while (1) {
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
                        continue;
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
                fprintf(stderr, "recv() failed. (%d)\n", GETSOCKETERRNO());
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
        int client_index;

        for (client_index = 0; client_index < MAX_CONNECTIONS; client_index++) {
                if (CONNECTED_CLIENTS[client_index].socket <= 0) {
                        CONNECTED_CLIENTS[client_index] = *client;
                        increase_total_connections();
                        break;
                }
        }

        return &CONNECTED_CLIENTS[client_index];
}

static void *server_thread_func(void *arg)
{
        Client *client = arg;

        printf("Client connected - IP: %s PORT: %d\n", client->address, client->port);

        while (recv(client->socket, client->message, sizeof(client->message), 0) > 0) {
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
                        client = NULL;
                        return NULL;
                }

                if (!strncmp(client->message, "\n", strlen("\n")) == 0) {
                        printf("[%s:%d] %s >>> %s",
                               client->address, client->port,
                               client->username, client->message);

                        send_public_message(client);
                }
        }

        fprintf(stderr, "recv() failed. (%d)\n", GETSOCKETERRNO());
        disconnect_client(client);
        client = NULL;

        return NULL;
}

static void list_online_clients(struct client *client)
{
        if (TOTAL_CONNECTIONS > 1) {
                char message[BUFFER_SIZE];
                char port[6];

                memset(message, 0, sizeof(message));
                memset(port, 0, sizeof(port));

                for (int i = 0; i < MAX_CONNECTIONS; i++) {
                        if (CONNECTED_CLIENTS[i].socket > 0 &&
                            CONNECTED_CLIENTS[i].socket != client->socket) {

                                strncat(message, "[",
                                        (BUFFER_SIZE - strlen(message) - 1));
                                strncat(message, CONNECTED_CLIENTS[i].address,
                                        (BUFFER_SIZE - strlen(message) - 1));
                                strncat(message, ":",
                                        (BUFFER_SIZE - strlen(message) - 1));

                                sprintf(port, "%d", CONNECTED_CLIENTS[i].port);
                                strncat(message, port,
                                        (BUFFER_SIZE - strlen(message) - 1));
                                strncat(message, "] ",
                                        (BUFFER_SIZE - strlen(message) - 1));
                                strncat(message, CONNECTED_CLIENTS[i].username,
                                        (BUFFER_SIZE - strlen(message) - 1));
                                strncat(message, "\r\n",
                                        (BUFFER_SIZE - strlen(message) - 1));

                                memset(port, 0, sizeof(port));
                        }
                }

                send_message(client->socket, message);

                memset(port, 0, sizeof(port));
                memset(message, 0, sizeof(message));
                memset(client->message, 0, sizeof(client->message));

                return;
        }

        send_message(client->socket, NO_USERS_ONLINE_MESSAGE);
}

static void build_message(const struct client *client, char *message, size_t message_size)
{
        /* Msg of 'username' ['address':'port']:[hh:mm UTC] 'message' */
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

        memset(port, 0, sizeof(port));
        memset(sent_at, 0, sizeof(sent_at));
}

static void get_current_time(char *buffer, size_t buffer_size)
{
        time_t rawtime;
        struct tm *info;
        char hh[5], mm[5];

        memset(&info, 0, sizeof(info));
        memset(hh, 0, sizeof(hh));
        memset(mm, 0, sizeof(mm));

        time(&rawtime);
        info = gmtime(&rawtime);

        sprintf(hh, "%d", (info->tm_hour % 24));
        sprintf(mm, "%d", info->tm_min);

        strncat(buffer, hh,
                (buffer_size - strlen(buffer) - 1));
        strncat(buffer, ":",
                (buffer_size - strlen(buffer) - 1));
        strncat(buffer, mm,
                (buffer_size - strlen(buffer) - 1));
        strncat(buffer, " UTC",
                (buffer_size - strlen(buffer) - 1));
}

static void send_public_message(struct client *client)
{
        char message[BUFFER_SIZE];

        build_message(client, message, sizeof(message));

        for (int i = 0; i < MAX_CONNECTIONS; i++) {
                if (CONNECTED_CLIENTS[i].socket > 0 &&
                    CONNECTED_CLIENTS[i].socket != client->socket) {
                        send_message(CONNECTED_CLIENTS[i].socket, message);
                }
        }

        memset(message, 0, sizeof(message));
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
                send_message(client->socket, NO_USERS_ONLINE_MESSAGE);
                return;
        }

        int status = -1;
        char message[BUFFER_SIZE];
        char dest_username[BUFFER_SIZE];

        memset(message, 0, sizeof(message));
        memset(dest_username, 0, sizeof(dest_username));

        status = extract_username_and_message(client->message,
                                              dest_username,
                                              message);

        if (status < 0) {
                memset(client->message, 0, sizeof(client->message));
                send_message(client->socket, INVALID_MESSAGE_FORMAT);
                return;
        }

        for (int i = 0; i < MAX_CONNECTIONS; i++) {
                if (strncmp(CONNECTED_CLIENTS[i].username,
                            dest_username,
                            sizeof(CONNECTED_CLIENTS[i].username)) == 0) {

                        memset(client->message, 0, sizeof(client->message));
                        strncpy(client->message, message, sizeof(client->message));
                        build_message(client, message, sizeof(message));
                        send_message(CONNECTED_CLIENTS[i].socket, message);

                        status = 1;

                        break;
                }
        }

        memset(message, 0, sizeof(message));
        memset(dest_username, 0, sizeof(dest_username));
        memset(client->message, 0, sizeof(client->message));

        if (status < 1) {
                send_message(client->socket, PRIVATE_MESSAGE_FAILURE);
        }
}

static void disconnect_client(struct client *client)
{
        printf("client disconnects - IP: %s PORT: %d\n",
               client->address, client->port);

        CLOSESOCKET(client->socket);

        for (int i = 0; i < MAX_CONNECTIONS; i++) {
                if (CONNECTED_CLIENTS[i].socket == client->socket) {
                        memset(CONNECTED_CLIENTS[i].username, 0,
                               sizeof(CONNECTED_CLIENTS[i].username));

                        CONNECTED_CLIENTS[i].address = "";

                        CONNECTED_CLIENTS[i].port = 0;

                        CONNECTED_CLIENTS[i].socket = 0;

                        memset(CONNECTED_CLIENTS[i].message, 0,
                               sizeof(CONNECTED_CLIENTS[i].message));

                        decrease_total_connections();

                        break;
                }
        }
}

static void send_message(const uint16_t client_socket, const char *message)
{
        if (send(client_socket, message, strlen(message), 0) < 0) {
                fprintf(stderr, "send() failed. (%d)\n", GETSOCKETERRNO());
        }
}
