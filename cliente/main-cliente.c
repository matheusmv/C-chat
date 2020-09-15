#include "cliente.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {

    Cliente c1;

    if (argc != 4) {
        printf("%s <IP_SERVIDOR> <PORTA> <USUARIO>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    c1.usuario = argv[3];

    criar_socket(&c1);

    conectar(&c1, argv[1], (uint16_t) atoi(argv[2]));

    iniciar_chat(&c1);

    return EXIT_SUCCESS;
}
