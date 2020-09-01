#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

typedef struct servidor {
    struct sockaddr_in cfg_servidor;
    char *IP_SERVIDOR;
    uint16_t PORTA;
} Servidor;


typedef struct cliente {
    int cliente_socket;
} Cliente;


void criar_socket(Cliente *cliente) {
    cliente->cliente_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (cliente->cliente_socket == -1) {
        printf("Erro ao criar o socket\n");
        exit(1);
    }

    printf("Socket criado\n");
}

void _configura_servidor(Servidor *servidor, char *IP, uint16_t PORTA) {
    servidor->cfg_servidor.sin_addr.s_addr = inet_addr(IP);
    servidor->cfg_servidor.sin_family = AF_INET;
    servidor->cfg_servidor.sin_port = htons(PORTA);
}

void conectar(Cliente *cliente, Servidor *servidor) {
    _configura_servidor(servidor, servidor->IP_SERVIDOR, servidor->PORTA);

    if (connect(cliente->cliente_socket, (struct sockaddr *) &servidor->cfg_servidor, sizeof(servidor->cfg_servidor)) < 0) {
        printf("Erro ao tentar conectar-se com o servidor\n");
        exit(1);
    }

    printf("Conectado com sucesso!\n");
}

void enviar(Cliente *cliente) {
    char *mensagem = "GET / HTTP/1.1\r\n\r\n";
    
    if (send(cliente->cliente_socket, mensagem, strlen(mensagem), 0) < 0) {
        printf("Erro ao enviar mensagem\n");
        exit(1);
    }

    printf("Mensagem enviada!\n");
}

void ler_mensagem(Cliente *cliente) {
    char resposta_servidor[4096];
    if (recv(cliente->cliente_socket, resposta_servidor, sizeof(resposta_servidor), 0) < 0) {
        printf("Falha ao recebem mensagem do servidor\n");
    }

    printf("Mensagem do servidor: \n\n");
    puts(resposta_servidor);
}

void fechar_conexao(Cliente *cliente) {
    close(cliente->cliente_socket);
}

int main(int argc, char *argv[]) {

    Cliente c1;
    Servidor s1;

    if (argc != 3) {
        printf("./<PROGRAMA> <IP_SERVIDOR> <PORTA>\n");
        exit(1);
    }

    criar_socket(&c1);

    s1.IP_SERVIDOR = argv[1];
    s1.PORTA = (uint16_t) atoi(argv[2]);

    conectar(&c1, &s1);

    enviar(&c1);

    ler_mensagem(&c1);

    fechar_conexao(&c1);

    return 0;
}
