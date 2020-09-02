#include "servidor.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {

    Servidor s1;

    if (argc != 2) {
        printf("./<PROGRAMA> <PORTA_SERVIDOR>\n");
        exit(EXIT_FAILURE);
    }

    criar_socket(&s1);

    configurar_servidor(&s1, (uint16_t) atoi(argv[1]));

    aceitar_conexoes(&s1);

    return EXIT_SUCCESS;
}