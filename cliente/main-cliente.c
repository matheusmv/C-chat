#include "cliente.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {

    Cliente c1;

    if (argc != 3) {
        printf("./<PROGRAMA> <IP_SERVIDOR> <PORTA>\n");
        exit(EXIT_FAILURE);
    }

    criar_socket(&c1);

    conectar(&c1, argv[1], (uint16_t) atoi(argv[2]));

    iniciar_chat(&c1);

    return EXIT_SUCCESS;
}
