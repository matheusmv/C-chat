#include "network.h"

SOCKET connect_to_server(const char *address, const uint16_t port)
{
        SOCKET socketfd = 0;
        SOCKET status = 0;

        struct sockaddr_in server_details;
        memset(&server_details, 0, sizeof(server_details));
        server_details.sin_addr.s_addr = inet_addr(address);
        server_details.sin_family = AF_INET;
        server_details.sin_port = htons(port);

        socketfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (!ISVALIDSOCKET(socketfd)) {
                char *err = strerror(GETSOCKETERRNO());
                fprintf(stderr, "socket() failed. (%d):(%s)\n", GETSOCKETERRNO(), err);
                exit(EXIT_FAILURE);
        }

        status = connect(socketfd, (struct sockaddr *) &server_details, sizeof(server_details));
        if (status < 0) {
                char *err = strerror(GETSOCKETERRNO());
                fprintf(stderr, "connect() failed. (%d):(%s)\n", GETSOCKETERRNO(), err);
                exit(EXIT_FAILURE);
        }

        return socketfd;
}

SOCKET create_server(const uint16_t port)
{
        SOCKET socketfd = 0;
        SOCKET status = 0;

        struct sockaddr_in server_details;
        memset(&server_details, 0, sizeof(server_details));
        server_details.sin_addr.s_addr = INADDR_ANY;
        server_details.sin_family = AF_INET;
        server_details.sin_port = htons(port);

        socketfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (!ISVALIDSOCKET(socketfd)) {
                char *err = strerror(GETSOCKETERRNO());
                fprintf(stderr, "socket() failed. (%d):(%s)\n", GETSOCKETERRNO(), err);
                exit(EXIT_FAILURE);
        }

        status = bind(socketfd, (struct sockaddr *) &server_details, sizeof(server_details));
        if (status < 0) {
                char *err = strerror(GETSOCKETERRNO());
                fprintf(stderr, "bind() failed. (%d):(%s)\n", GETSOCKETERRNO(), err);
                exit(EXIT_FAILURE);
        }

        status = listen(socketfd, 0);
        if (status < 0) {
                char *err = strerror(GETSOCKETERRNO());
                fprintf(stderr, "listen() failed. (%d):(%s)\n", GETSOCKETERRNO(), err);
                exit(EXIT_FAILURE);
        }

        fprintf(stdout, "server running on port %d\n", ntohs(server_details.sin_port));

        return socketfd;
}
