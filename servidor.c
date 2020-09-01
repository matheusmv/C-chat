#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

typedef struct servidor {
    int servidor_socket;
    struct sockaddr_in cfg_servidor;
} Servidor;


typedef struct cliente {
    char *IP;
    uint16_t PORTA;
    int cliente_socket;
    struct sockaddr_in cfg_cliente;
} Cliente;

void criar_socket(Servidor *servidor) {
    servidor->servidor_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (servidor->servidor_socket == -1) {
        printf("Erro ao criar o socket\n");
        exit(1);
    }

    printf("Socket criado\n");
}

void configurar_servidor(Servidor *servidor, uint16_t PORTA) {
    servidor->cfg_servidor.sin_family = AF_INET;
    servidor->cfg_servidor.sin_addr.s_addr = INADDR_ANY;
    servidor->cfg_servidor.sin_port = htons(PORTA);

    if (bind(servidor->servidor_socket, (struct sockaddr *) &servidor->cfg_servidor, sizeof(servidor->cfg_servidor)) < 0) {
        printf("Erro ao configurar o servidor");
        exit(1);
    }

    printf("Servidor rodando na porta %d\n", PORTA);

    listen(servidor->servidor_socket, 3);
}

void enviar_resposta(Cliente *cliente) {
    char *mensagem = "Olá, eu recebi sua conexão. Adeus.\n";
    write(cliente->cliente_socket, mensagem, strlen(mensagem));
}

void aceitar_conexoes(Servidor *servidor, Cliente *cliente) {
    int len = sizeof(struct sockaddr_in);
    
    while((cliente->cliente_socket = accept(servidor->servidor_socket, (struct sockaddr *) &cliente->cfg_cliente, (socklen_t *) &len))) {       
        cliente->IP = inet_ntoa(cliente->cfg_cliente.sin_addr);
        cliente->PORTA = ntohs(cliente->cfg_cliente.sin_port);

        printf("Conexão bem sucedida\n");
        printf("IP: %s PORTA: %d\n", cliente->IP, cliente->PORTA);

        enviar_resposta(cliente);
    }

    if (cliente->cliente_socket < 0) {
        printf("Erro ao aceitar conexões\n");
        exit(1);
    }
}

int main(int argc, char *argv[]) {

    Servidor s1;
    Cliente c1;

    if (argc != 2) {
        printf("./<PROGRAMA> <PORTA_SERVIDOR>");
    }

    criar_socket(&s1);

    configurar_servidor(&s1, (uint16_t) atoi(argv[1]));

    aceitar_conexoes(&s1, &c1);

    return 0;
}
