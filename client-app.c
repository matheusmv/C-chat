#include "client.h"
#include "utils.h"

#include <unistd.h>
#include <getopt.h>

int main(int argc, char *argv[])
{
        char *server_ip = "";
        char *server_port = "";
        char *username = "";

        char optc = 0;

        struct option LongOptions[] = {
                {"address", required_argument, NULL, 'a'},
                {"port", required_argument, NULL, 'p'},
                {"username", required_argument, NULL, 'u'}
        };

        while ((optc = getopt_long(argc, argv, "a:p:u:", LongOptions, NULL)) != -1) {
                switch (optc) {
                case 'a':
                        if (!validate_ipv4(optarg)) {
                                fprintf(stderr, "invalid ip address\n");
                                exit(EXIT_FAILURE);
                        }
                        server_ip = optarg;
                        break;
                case 'p':
                        if (!validate_port(optarg)) {
                                fprintf(stderr, "invalid port\n");
                                exit(EXIT_FAILURE);
                        }
                        server_port = optarg;
                        break;
                case 'u':
                        if (!validate_username(optarg)) {
                                fprintf(stderr, "invalid username\n");
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

                fprintf(stderr, "usage: ./client -a ip -p port -u username\n");
                exit(EXIT_FAILURE);
        }

        start_client(server_ip, (uint16_t) atoi(server_port), username);

        return EXIT_SUCCESS;
}
