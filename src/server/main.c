#include <getopt.h>
#include <unistd.h>

#include "server.h"
#include "../includes/clog.h"
#include "../includes/utils.h"

int main(int argc, char *argv[])
{
        char *server_port = "";

        struct option LongOptions[] = {
                {"port", required_argument, NULL, 'p'}
        };

        char optc = 0;
        while ((optc = getopt_long(argc, argv, "p:", LongOptions, NULL)) != -1) {
                switch (optc) {
                case 'p':
                        if (!validate_port(optarg)) {
                                LOG_ERROR("invalid port: %s", optarg);
                                exit(EXIT_FAILURE);
                        }
                        server_port = optarg;
                        break;
                default:
                        exit(EXIT_FAILURE);
                }
        }

        if (!validate_port(server_port)) {
                LOG_ERROR("usage: %s -p port\n", argv[0]);
                exit(EXIT_FAILURE);
        }

        start_server((uint16_t) atoi(server_port));

        return EXIT_SUCCESS;
}
