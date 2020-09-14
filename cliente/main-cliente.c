#include "cliente.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {

    Cliente c1;

    if (argc != 4) {
        printf("%s <IP_SERVIDOR> <PORTA> <USUARIO>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    criar_socket(&c1);

    conectar(&c1, argv[1], (uint16_t) atoi(argv[2]), argv[3]);

    iniciar_chat(&c1);

    return EXIT_SUCCESS;
}
