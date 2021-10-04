#include "server.h"
#include "utils.h"

#include <unistd.h>
#include <getopt.h>

int main(int argc, char *argv[])
{
        if (argc < 2) {
                fprintf(stderr, "usage: ./server -p port\n");
                exit(EXIT_FAILURE);
        }

        char *server_port = "";

        char optc = 0;

        struct option LongOptions[] = {
                {"port", required_argument, NULL, 'p'}
        };

        while ((optc = getopt_long(argc, argv, "p:", LongOptions, NULL)) != -1) {
                switch (optc) {
                case 'p':
                        if (!validate_ipv4(optarg)) {
                                fprintf(stderr, "invalid port\n");
                                exit(EXIT_FAILURE);
                        }
                        server_port = optarg;
                        break;
                default:
                        exit(EXIT_FAILURE);
                }
        }

        if (!validate_port(server_port)) {
                fprintf(stderr, "usage: ./server -p port\n");
                exit(EXIT_FAILURE);
        }

        start_server((uint16_t) atoi(server_port));

        return EXIT_SUCCESS;
}
