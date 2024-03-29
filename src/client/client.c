#include "client.h"
#include "../includes/clogger.h"

static int server_auth(const int, const char *);
static void *client_send_thr(void *);
static void *client_recv_thr(void *);

void
start_client(const char *address, const uint16_t port, const char *username)
{
        SOCKET c_socket = client_connect_to_server(address, port);

        pthread_t sendthread, recvthread;

        if (server_auth(c_socket, username) > 0) {
                pthread_create(&sendthread, NULL, client_send_thr, (void *) &c_socket);
                pthread_create(&recvthread, NULL, client_recv_thr, (void *) &c_socket);

                pthread_join(sendthread, NULL);
                pthread_join(recvthread, NULL);
        }
}

static int
server_auth(const int client_socket, const char *username)
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
                printf("Login failed!\n");
                printf("%s", server_response);
                CLOSESOCKET(client_socket);
        }

        return auth_status;
}

static void *
client_send_thr(void *arg)
{
        SOCKET *client_socket = arg;

        char client_input[BUFFER_SIZE];
        char final_message[BUFFER_SIZE];

        while (1) {
                memset(client_input, 0, sizeof(client_input));
                memset(final_message, 0, sizeof(final_message));

                if (fgets(client_input, sizeof(client_input), stdin) == NULL)
                        continue;

                strncat(final_message, client_input,
                        (sizeof(final_message) - strlen(client_input) - 1));

                if (strncmp(final_message, DISCONNECT, strlen(DISCONNECT)) == 0) {
                        send_message(client_socket, DISCONNECT, strlen(DISCONNECT));
                        client_socket = NULL;
                        return NULL;
                }

                send_message(client_socket, final_message, strlen(final_message));
        }

        return NULL;
}

static void *
client_recv_thr(void *arg)
{
        SOCKET *client_socket = arg;

        char server_response[BUFFER_SIZE];

        while (1) {
                memset(server_response, 0, sizeof(server_response));

                if (recv(*client_socket, server_response, sizeof(server_response), 0) <= 0) {
                        LOG_ERROR("recv() failed. %s", strerror(errno));
                        return NULL;
                }

                if (strncmp(server_response, DISCONNECT, strlen(DISCONNECT)) == 0) {
                        printf("disconnected from server.\n");
                        CLOSESOCKET(*client_socket);
                        client_socket = NULL;
                        return NULL;
                }

                printf("%s", server_response);
        }

        return NULL;
}
