// TODO interromper execução caso a conexão com o servidor seja rejeitada
// TODO implementar etapa de autenticação

#include "cliente.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define BUFFER_SIZE 4096

typedef struct servidor {
    struct sockaddr_in cfg_servidor;
    char *IP_SERVIDOR;
    uint16_t PORTA;
} Servidor;

void iniciar_chat(Cliente *);

void fechar_conexao(Cliente *);

void* func_thread_send_cliente(void *);

void* func_thread_recv_cliente(void *);

void auth_servidor(Cliente *);

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

void conectar(Cliente *cliente, char *IP, uint16_t PORTA) {
    Servidor servidor = *(Servidor *) malloc(sizeof(Servidor));
    configura_servidor(&servidor, IP, PORTA);

    if (connect(cliente->cliente_socket, (struct sockaddr *) &servidor.cfg_servidor, sizeof(servidor.cfg_servidor)) < 0) {
        printf("Erro ao tentar conectar-se com o servidor\n");
        exit(EXIT_FAILURE);
    }

    printf("Conectado com sucesso!\n");
}

void fechar_conexao(Cliente *cliente) {
    close(cliente->cliente_socket);
}

void iniciar_chat(Cliente *cliente) {
    pthread_t client_recv_thread, client_send_thread;
    Cliente *p_cliente = malloc(sizeof(Cliente));
    p_cliente = cliente;

    auth_servidor(cliente);

    pthread_create(&client_send_thread, NULL, func_thread_send_cliente, (void *) p_cliente);
    pthread_create(&client_recv_thread, NULL, func_thread_recv_cliente, (void *) p_cliente);

    pthread_join(client_send_thread, NULL);
    pthread_join(client_recv_thread, NULL);
}

void* func_thread_send_cliente(void *argumento) {
    Cliente cliente = *(Cliente *) argumento;

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
}

void* func_thread_recv_cliente(void *argumento) {
    Cliente cliente = *(Cliente *) argumento;

    int len;
    char resposta_servidor[BUFFER_SIZE];

    while (1) {
        len = recv(cliente.cliente_socket, resposta_servidor, sizeof(resposta_servidor), 0);

        if (len < 0) {
            printf("Falha ao recebem mensagem do servidor\n");
        }
        
        printf("%s", resposta_servidor);

        bzero(resposta_servidor, sizeof(resposta_servidor));
    }
}

void auth_servidor(Cliente *cliente) {
    send(cliente->cliente_socket, cliente->usuario, strlen(cliente->usuario), 0);
}
