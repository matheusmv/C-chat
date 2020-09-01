#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

typedef struct _servidor {
    int servidor_socket;
    struct sockaddr_in cfg_servidor;
} Servidor;

typedef struct _cliente {
    char *IP;
    uint16_t PORTA;
    int cliente_socket;
    char mensagem_cliente[4096];
    struct sockaddr_in cfg_cliente;
} Cliente;

void criar_socket(Servidor *servidor);

void configurar_servidor(Servidor *servidor, uint16_t PORTA);

void enviar_resposta(Cliente *cliente);

void receber_resposta(Cliente *cliente);

void aceitar_conexoes(Servidor *servidor, Cliente *cliente);

int main(int argc, char *argv[]) {

    Servidor s1;
    Cliente c1;

    if (argc != 2) {
        printf("./<PROGRAMA> <PORTA_SERVIDOR>");
        exit(EXIT_FAILURE);
    }

    criar_socket(&s1);

    configurar_servidor(&s1, (uint16_t) atoi(argv[1]));

    aceitar_conexoes(&s1, &c1);

    return EXIT_SUCCESS;
}

void criar_socket(Servidor *servidor) {
    servidor->servidor_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (servidor->servidor_socket == -1) {
        printf("Erro ao criar o socket\n");
        exit(EXIT_FAILURE);
    }

    printf("Socket criado\n");
}

void configurar_servidor(Servidor *servidor, uint16_t PORTA) {
    servidor->cfg_servidor.sin_family = AF_INET;
    servidor->cfg_servidor.sin_addr.s_addr = INADDR_ANY;
    servidor->cfg_servidor.sin_port = htons(PORTA);

    if (bind(servidor->servidor_socket, (struct sockaddr *) &servidor->cfg_servidor, sizeof(servidor->cfg_servidor)) < 0) {
        printf("Erro ao configurar o servidor");
        exit(EXIT_FAILURE);
    }

    printf("Servidor rodando na porta %d\n", PORTA);

    listen(servidor->servidor_socket, 3);
}

void enviar_resposta(Cliente *cliente) {
    send(cliente->cliente_socket, cliente->mensagem_cliente, strlen(cliente->mensagem_cliente), 0);
}

void receber_resposta(Cliente *cliente) {
    int len;
    while((len = recv(cliente->cliente_socket, cliente->mensagem_cliente, sizeof(cliente->mensagem_cliente), 0)) > 0) {
        printf("Mensagem do cliente:\n");
        printf("[%s:%d] %s\n", cliente->IP, cliente->PORTA, cliente->mensagem_cliente);
        enviar_resposta(cliente);
        bzero(cliente->mensagem_cliente, sizeof(cliente->mensagem_cliente));
    }

    if (len == 0) {
        printf("Cliente desconectado - IP: %s PORTA: %d\n", cliente->IP, cliente->PORTA);
        close(cliente->cliente_socket);
    } else if (len == -1) {
        printf("Falha ao receber mensagem\n");
    }
}

void aceitar_conexoes(Servidor *servidor, Cliente *cliente) {
    int len = sizeof(struct sockaddr_in);
    
    while((cliente->cliente_socket = accept(servidor->servidor_socket, (struct sockaddr *) &cliente->cfg_cliente, (socklen_t *) &len))) {       
        cliente->IP = inet_ntoa(cliente->cfg_cliente.sin_addr);
        cliente->PORTA = ntohs(cliente->cfg_cliente.sin_port);

        printf("Conexão bem sucedida - IP: %s PORTA: %d\n", cliente->IP, cliente->PORTA);

        receber_resposta(cliente);
    }

    if (cliente->cliente_socket < 0) {
        printf("Erro ao aceitar conexões\n");
        exit(EXIT_FAILURE);
    }
}
