#include "utils.h"

int
validate_number(const char *number)
{
        while (*number) {
                if (!isdigit(*number)) {
                        return 0;
                }

                number++;
        }

        return 1;
}

int
validate_ipv4(const char *ip)
{
        if (ip == NULL)
                return 0;

        int number = 0;
        int dots = 0;
        char IP[50];

        memset(IP, 0, sizeof(IP));

        strncpy(IP, ip, (sizeof(IP) - strlen(ip) - 1));

        char *split_ip = strtok(IP, ".");

        if (split_ip == NULL)
                return 0;

        while (split_ip) {
                if (!validate_number(split_ip))
                        return 0;

                number = atoi(split_ip);

                if (number >= 0 && number <= 255) {
                        split_ip = strtok(NULL, ".");

                        if (split_ip != NULL)
                                dots++;
                } else {
                        return 0;
                }
        }

        if (dots != 3)
                return 0;

        return 1;
}

int
validate_port(const char *port)
{
        if (port == NULL)
                return 0;

        int input_length = strlen(port);

        if (input_length < 1 || input_length > 5)
                return 0;

        if (!validate_number(port))
                return 0;

        int port_number = atoi(port);

        if (port_number < 0 || port_number > 65535)
                return 0;

        return 1;
}

int
validate_username(const char *username)
{
        if (username == NULL)
                return 0;

        int input_length = strlen(username);

        if (input_length < 1 || input_length > 255)
                return 0;

        if (validate_number(username))
                return 0;

        return 1;
}
