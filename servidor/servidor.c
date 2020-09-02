#include "servidor.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

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

    listen(servidor->servidor_socket, 10);
}

void enviar_resposta(Cliente *cliente) {
    send(cliente->cliente_socket, cliente->mensagem_cliente, strlen(cliente->mensagem_cliente), 0);
}

void* funcao_servidor(void *argumento) {
    Cliente cliente = *(Cliente*) argumento;
    int addr_len = sizeof(struct sockaddr_in);
    int len;

    while (1) {
        len = recv(cliente.cliente_socket, cliente.mensagem_cliente, sizeof(cliente.mensagem_cliente), 0);

        if (len == 0) {
            printf("Cliente desconectado - IP: %s PORTA: %d\n", cliente.IP, cliente.PORTA);
            close(cliente.cliente_socket);
            return NULL;
        } else if (len == -1) {
            printf("Falha ao receber mensagem\n");
            return NULL;
        }

        printf("Mensagem do cliente: ");
        printf("[%s:%d] %s\n", cliente.IP, cliente.PORTA, cliente.mensagem_cliente);
        enviar_resposta(&cliente);
        bzero(cliente.mensagem_cliente, sizeof(cliente.mensagem_cliente));
    }

    return NULL;
}

void aceitar_conexoes(Servidor *servidor) {
    Cliente cliente = *(Cliente *) malloc(sizeof(Cliente));
    int addr_len = sizeof(struct sockaddr_in);
    
    while (1) {

        cliente.cliente_socket = accept(servidor->servidor_socket, (struct sockaddr *) &cliente.cfg_cliente, (socklen_t *) &addr_len);
        
        if (cliente.cliente_socket < 0) {
            perror("Erro ao aceitar conexões");
            exit(EXIT_FAILURE);
        }
        
        cliente.IP = inet_ntoa(cliente.cfg_cliente.sin_addr);
        cliente.PORTA = ntohs(cliente.cfg_cliente.sin_port);

        printf("Conexão bem sucedida - IP: %s PORTA: %d\n", cliente.IP, cliente.PORTA);

        pthread_t server_thread;
        Cliente *p_cliente = malloc(sizeof(Cliente));
        *p_cliente = cliente;

        pthread_create(&server_thread, NULL, funcao_servidor, (void *) p_cliente);
    }
}