#include "network.h"

SOCKET connect_to_server(const char *address, const uint16_t port)
{
        SOCKET socketfd = 0;
        int status = 0;

        struct sockaddr_in server_details;
        memset(&server_details, 0, sizeof(server_details));
        server_details.sin_addr.s_addr = inet_addr(address);
        server_details.sin_family = AF_INET;
        server_details.sin_port = htons(port);

        socketfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (!ISVALIDSOCKET(socketfd)) {
                LOG_ERROR("socket() failed. %s", strerror(errno));
                exit(EXIT_FAILURE);
        }

        status = connect(socketfd, (struct sockaddr *) &server_details, sizeof(server_details));
        if (status < 0) {
                LOG_ERROR("connect() failed. %s", strerror(errno));
                exit(EXIT_FAILURE);
        }

        return socketfd;
}

SOCKET create_server(const uint16_t port)
{
        SOCKET socketfd = 0;
        int status = 0;

        struct sockaddr_in server_details;
        memset(&server_details, 0, sizeof(server_details));
        server_details.sin_addr.s_addr = INADDR_ANY;
        server_details.sin_family = AF_INET;
        server_details.sin_port = htons(port);

        socketfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (!ISVALIDSOCKET(socketfd)) {
                LOG_ERROR("socket() failed. %s", strerror(errno));
                exit(EXIT_FAILURE);
        }

        status = bind(socketfd, (struct sockaddr *) &server_details, sizeof(server_details));
        if (status < 0) {
                LOG_ERROR("bind() failed. %s", strerror(errno));
                exit(EXIT_FAILURE);
        }

        status = listen(socketfd, 0);
        if (status < 0) {
                LOG_ERROR("listen() failed. %s", strerror(errno));
                exit(EXIT_FAILURE);
        }

        LOG_INFO("server running on port %d", port);

        return socketfd;
}

SOCKET accept_new_client(SOCKET *server_socket, struct sockaddr_in *client_details)
{
        if (server_socket == NULL || client_details == NULL) {
                LOG_ERROR("invalid parameters");
                exit(EXIT_FAILURE);
        }

        memset(client_details, 0, sizeof(struct sockaddr_in));

        SOCKET socketfd = 0;
        socklen_t addrlen = sizeof(struct sockaddr_in);

        socketfd = accept(*server_socket, (struct sockaddr *) client_details, &addrlen);

        return socketfd;
}
