#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

typedef struct _servidor {
    struct sockaddr_in cfg_servidor;
    char *IP_SERVIDOR;
    uint16_t PORTA;
} Servidor;

typedef struct _cliente {
    int cliente_socket;
} Cliente;

void criar_socket(Cliente *cliente);

void conectar(Cliente *cliente, Servidor *servidor, char *IP, uint16_t PORTA);

void enviar_mensagem(Cliente *cliente);

void receber_mensagem(Cliente *cliente);

void fechar_conexao(Cliente *cliente);

int main(int argc, char *argv[]) {

    Cliente c1;
    Servidor s1;


    if (argc != 3) {
        printf("./<PROGRAMA> <IP_SERVIDOR> <PORTA>\n");
        exit(EXIT_FAILURE);
    }

    criar_socket(&c1);

    conectar(&c1, &s1, argv[1], (uint16_t) atoi(argv[2]));

    while (1) {
        enviar_mensagem(&c1);
        receber_mensagem(&c1);
    }

    fechar_conexao(&c1);

    return EXIT_SUCCESS;
}

void criar_socket(Cliente *cliente) {
    cliente->cliente_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (cliente->cliente_socket == -1) {
        printf("Erro ao criar o socket\n");
        exit(EXIT_FAILURE);
    }

    printf("Socket criado\n");
}

void configura_servidor(Servidor *servidor, char *IP, uint16_t PORTA) {
    servidor->IP_SERVIDOR = IP;
    servidor->PORTA = PORTA;
    servidor->cfg_servidor.sin_addr.s_addr = inet_addr(IP);
    servidor->cfg_servidor.sin_family = AF_INET;
    servidor->cfg_servidor.sin_port = htons(PORTA);
}

void conectar(Cliente *cliente, Servidor *servidor, char *IP, uint16_t PORTA) {
    configura_servidor(servidor, IP, PORTA);

    if (connect(cliente->cliente_socket, (struct sockaddr *) &servidor->cfg_servidor, sizeof(servidor->cfg_servidor)) < 0) {
        printf("Erro ao tentar conectar-se com o servidor\n");
        exit(EXIT_FAILURE);
    }

    printf("Conectado com sucesso!\n");
}

void enviar_mensagem(Cliente *cliente) {
    char mensagem[4096];

    bzero(mensagem, sizeof(mensagem));

    printf("Msg > ");
    fgets(mensagem, sizeof(mensagem), stdin);

    if (send(cliente->cliente_socket, mensagem, strlen(mensagem), 0) < 0) {
        printf("Erro ao enviar mensagem\n");
    }

    printf("Mensagem enviada!\n");
}

void receber_mensagem(Cliente *cliente) {
    char resposta_servidor[4096];

    if (recv(cliente->cliente_socket, resposta_servidor, sizeof(resposta_servidor), 0) < 0) {
        printf("Falha ao recebem mensagem do servidor\n");
    }

    printf("Mensagem do servidor: ");
    printf("%s\n", resposta_servidor);
    bzero(resposta_servidor, sizeof(resposta_servidor));
}

void fechar_conexao(Cliente *cliente) {
    close(cliente->cliente_socket);
}
