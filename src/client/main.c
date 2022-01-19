#include <getopt.h>
#include <unistd.h>

#include "client.h"
#include "../includes/clogger.h"
#include "../includes/utils.h"

int main(int argc, char *argv[])
{
        clogger_create(stderr);

        char *server_ip = "";
        char *server_port = "";
        char *username = "";

        struct option LongOptions[] = {
                {"address", required_argument, NULL, 'a'},
                {"port", required_argument, NULL, 'p'},
                {"username", required_argument, NULL, 'u'}
        };

        char optc = 0;
        while ((optc = getopt_long(argc, argv, "a:p:u:", LongOptions, NULL)) != -1) {
                switch (optc) {
                case 'a':
                        if (!validate_ipv4(optarg)) {
                                LOG_ERROR("invalid ip address %s", optarg);
                                exit(EXIT_FAILURE);
                        }
                        server_ip = optarg;
                        break;
                case 'p':
                        if (!validate_port(optarg)) {
                                LOG_ERROR("invalid port %s", optarg);
                                exit(EXIT_FAILURE);
                        }
                        server_port = optarg;
                        break;
                case 'u':
                        if (!validate_username(optarg)) {
                                LOG_ERROR("invalid username %s", optarg);
                                exit(EXIT_FAILURE);
                        }
                        username = optarg;
                        break;
                default:
                        exit(EXIT_FAILURE);
                }
        }

        if (!validate_ipv4(server_ip) ||
            !validate_port(server_port) ||
            !validate_username(username)) {

                LOG_ERROR("usage: %s -a ip -p port -u username", argv[0]);
                exit(EXIT_FAILURE);
        }

        start_client(server_ip, (uint16_t) atoi(server_port), username);

        return EXIT_SUCCESS;
}
