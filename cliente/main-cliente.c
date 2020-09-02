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

    while (1) {
        enviar_mensagem(&c1);
        receber_mensagem(&c1);
    }

    fechar_conexao(&c1);

    return EXIT_SUCCESS;
}