#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

/* Types */

#define BUFFER_SIZE 4096

typedef struct cliente {
    char *usuario;
    uint16_t cliente_socket;
} Cliente;

typedef struct servidor {
    struct sockaddr_in cfg_servidor;
    char *IP_SERVIDOR;
    uint16_t PORTA;
} Servidor;

/* Function declarations */

void criar_socket(Cliente *);

void configurar_servidor(Servidor *, char *, uint16_t);

void conectar(Cliente *, char *, uint16_t);

void iniciar_chat(Cliente *);

int auth_servidor(Cliente *);

void *func_thread_send_cliente(void *);

void *func_thread_recv_cliente(void *);

void fechar_conexao(Cliente *);

/* Function definitions */

void criar_socket(Cliente *cliente)
{
    cliente->cliente_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (cliente->cliente_socket == -1) {
        printf("Erro ao criar o socket\n");
        exit(EXIT_FAILURE);
    }

    printf("Socket criado\n");
}

void configurar_servidor(Servidor *servidor, char *IP, uint16_t PORTA)
{
    servidor->IP_SERVIDOR = IP;
    servidor->PORTA = PORTA;
    servidor->cfg_servidor.sin_addr.s_addr = inet_addr(IP);
    servidor->cfg_servidor.sin_family = AF_INET;
    servidor->cfg_servidor.sin_port = htons(PORTA);
}

void conectar(Cliente *cliente, char *IP, uint16_t PORTA)
{
    Servidor servidor = *(Servidor *)malloc(sizeof(Servidor));
    configurar_servidor(&servidor, IP, PORTA);

    if (connect(cliente->cliente_socket, (struct sockaddr *)&servidor.cfg_servidor, sizeof(servidor.cfg_servidor)) < 0) {
        printf("Erro ao tentar conectar-se com o servidor\n");
        exit(EXIT_FAILURE);
    }

    printf("Conectado com sucesso!\n");
}

void iniciar_chat(Cliente *cliente)
{
    pthread_t client_recv_thread, client_send_thread;
    Cliente *p_cliente = malloc(sizeof(Cliente));
    p_cliente = cliente;

    if (auth_servidor(cliente) == 1) {
        pthread_create(&client_send_thread, NULL, func_thread_send_cliente, (void *)p_cliente);
        pthread_create(&client_recv_thread, NULL, func_thread_recv_cliente, (void *)p_cliente);

        pthread_join(client_send_thread, NULL);
        pthread_join(client_recv_thread, NULL);
    }

    printf("Tente alterar o <USUARIO>.\n");
}

int auth_servidor(Cliente *cliente)
{
    int confirmacao_servidor = 0;
    char resposta_servidor[BUFFER_SIZE];
    char *sucesso = "sucesso\r\n";

    bzero(resposta_servidor, sizeof(resposta_servidor));

    send(cliente->cliente_socket, cliente->usuario, strlen(cliente->usuario), 0);
    recv(cliente->cliente_socket, resposta_servidor, sizeof(resposta_servidor), 0);

    if (strncmp(resposta_servidor, sucesso, strlen(sucesso)) == 0) {
        printf("Login bem sucedido!\n");
        confirmacao_servidor = 1;
    }
    else {
        printf("%s", resposta_servidor);
    }

    return confirmacao_servidor;
}

void *func_thread_send_cliente(void *argumento)
{
    Cliente cliente = *(Cliente *)argumento;

    char input_cliente[BUFFER_SIZE];
    char mensagem_enviada[BUFFER_SIZE];

    while (1) {
        bzero(input_cliente, sizeof(input_cliente));
        bzero(mensagem_enviada, sizeof(mensagem_enviada));

        fgets(input_cliente, sizeof(input_cliente), stdin);

        strncat(mensagem_enviada, input_cliente, strlen(input_cliente) - 1);
        strncat(mensagem_enviada, "\r\n", sizeof("\r\n"));

        if (send(cliente.cliente_socket, mensagem_enviada, strlen(mensagem_enviada), 0) < 0) {
            printf("Erro ao enviar mensagem\n");
        }
    }

    return NULL;
}

void *func_thread_recv_cliente(void *argumento)
{
    Cliente cliente = *(Cliente *)argumento;

    int len;
    char resposta_servidor[BUFFER_SIZE];

    while (1) {
        len = recv(cliente.cliente_socket, resposta_servidor, sizeof(resposta_servidor), 0);

        if (len == 0) {
            printf("Conexão com o servidor foi encerrada\n");
            return NULL;
        }
        else if (len == -1) {
            printf("Falha ao recebem mensagem do servidor\n");
        }

        printf("%s", resposta_servidor);

        bzero(resposta_servidor, sizeof(resposta_servidor));
    }

    return NULL;
}

void fechar_conexao(Cliente *cliente)
{
    close(cliente->cliente_socket);
}

/* main */

int main(int argc, char *argv[])
{
    Cliente c1;

    if (argc != 4) {
        printf("%s <IP_SERVIDOR> <PORTA> <USUARIO>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    c1.usuario = argv[3];

    criar_socket(&c1);

    conectar(&c1, argv[1], (uint16_t)atoi(argv[2]));

    iniciar_chat(&c1);

    return EXIT_SUCCESS;
}
