// TODO autenticação cliente-servidor
// TODO gerenciar dados do cliente no servidor
// TODO mensagem privadas cliente->cliente 

#include "servidor.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_CONEXOES 5
#define BUFFER_SIZE 4096
#define SERVIDOR_BACKLOG 5

typedef struct cliente {
    char *IP;
    uint16_t PORTA;
    int cliente_socket;
    char mensagem_cliente[BUFFER_SIZE];
    struct sockaddr_in cfg_cliente;
} Cliente;

Cliente clientes[MAX_CONEXOES];

int total_conexoes = 0;

void enviar_resposta(Cliente *, char *);

void* func_thread_servidor(void *);

void inicializar_clientes(Cliente *);

void registrar_cliente(Cliente *);

void listar_clientes(Cliente *);

char dados_cliente(Cliente *);

void enviar_mensagem_publica(Cliente *, char *);

void limpar_buffer_mensagem(char *, int);

void limpar_buffer_cliente(int);

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
        printf("Erro ao configurar o servidor\n");
        exit(EXIT_FAILURE);
    }

    printf("Servidor rodando na porta %d\n", PORTA);

    listen(servidor->servidor_socket, SERVIDOR_BACKLOG);
}

void enviar_resposta(Cliente *cliente, char *mensagem) {
    send(cliente->cliente_socket, mensagem, strlen(mensagem), 0);
}

void* func_thread_servidor(void *argumento) {
    Cliente cliente = *(Cliente*) argumento;

    printf("Conexão bem sucedida - IP: %s PORTA: %d\n", cliente.IP, cliente.PORTA);

    int addr_len = sizeof(struct sockaddr_in);
    int len;

    char *listar = ":listar\r\n";

    while (1) {
        len = recv(cliente.cliente_socket, cliente.mensagem_cliente, sizeof(cliente.mensagem_cliente), 0);

        if (len == 0) {
            printf("Cliente desconectado - IP: %s PORTA: %d\n", cliente.IP, cliente.PORTA);
            close(cliente.cliente_socket);
            limpar_buffer_cliente(cliente.cliente_socket);
            return NULL;
        } else if (len == -1) {
            printf("Falha ao receber mensagem\n");
        }

        if (strcmp(cliente.mensagem_cliente, listar) == 0) {
            listar_clientes(&cliente);
        } else {
            printf("[%s:%d] %s", cliente.IP, cliente.PORTA, cliente.mensagem_cliente);
            enviar_mensagem_publica(&cliente, cliente.mensagem_cliente);
        }

        limpar_buffer_mensagem(cliente.mensagem_cliente, sizeof(cliente.mensagem_cliente));
    }

    return NULL;
}

void aceitar_conexoes(Servidor *servidor) {
    inicializar_clientes(clientes);

    Cliente cliente = *(Cliente *) calloc(1, sizeof(Cliente));

    pthread_t server_thread;
    Cliente *p_cliente = malloc(sizeof(Cliente));

    int addr_len = sizeof(struct sockaddr_in);
    
    while (1) {

        cliente.cliente_socket = accept(servidor->servidor_socket, (struct sockaddr *) &cliente.cfg_cliente, (socklen_t *) &addr_len);
        
        if (cliente.cliente_socket < 0) {
            perror("Erro ao aceitar conexões");
            exit(EXIT_FAILURE);
        }
        
        cliente.IP = inet_ntoa(cliente.cfg_cliente.sin_addr);
        cliente.PORTA = ntohs(cliente.cfg_cliente.sin_port);

        if (total_conexoes < MAX_CONEXOES) {
            registrar_cliente(&cliente);
        } else {
            printf("[Máx.Clientes] Conexão rejeitada - IP: %s PORTA: %d\n", cliente.IP, cliente.PORTA);
            char *mensagem_status = "Máximo de clientes alcaçado\n";
            enviar_resposta(&cliente, mensagem_status);
            close(cliente.cliente_socket);
            continue;
        }

        *p_cliente = cliente;

        pthread_create(&server_thread, NULL, func_thread_servidor, (void *) p_cliente);
    }
}

void inicializar_clientes(Cliente *clientes) {
    for (int i = 0; i < MAX_CONEXOES; i++) {
        clientes[i] = *(Cliente *) calloc(1, sizeof(Cliente));
    }
}

void registrar_cliente(Cliente *novo_cliente) {
    for (int i = 0; i < MAX_CONEXOES; i++) {
        if (clientes[i].cliente_socket <= 0) {
            clientes[i] = *novo_cliente;
            total_conexoes++;
            break;
        }
    }
}

void listar_clientes(Cliente *cliente) {
    char mensagem_enviada[BUFFER_SIZE];
    char porta[6];

    if (total_conexoes == 1) {
        char *ninguem_online = "Não há usuários online no momemto.\n";
        enviar_resposta(cliente, ninguem_online);
    } else {
        for (int i = 0; i < MAX_CONEXOES; i++) {
            if (clientes[i].cliente_socket != cliente->cliente_socket && clientes[i].cliente_socket > 0) {
                strncat(mensagem_enviada, "[", sizeof("["));
                strncat(mensagem_enviada, clientes[i].IP, strlen(clientes[i].IP));
                strncat(mensagem_enviada, ":", sizeof(":"));
                sprintf(porta, "%d",clientes[i].PORTA);
                strncat(mensagem_enviada, porta, strlen(porta));
                limpar_buffer_mensagem(porta, sizeof(porta));
                strncat(mensagem_enviada, "] ", sizeof("] "));
                strncat(mensagem_enviada, "nome", sizeof("nome"));
                strncat(mensagem_enviada, "\r\n", sizeof("\r\n"));
            }
        }

        enviar_resposta(cliente, mensagem_enviada);

        limpar_buffer_mensagem(mensagem_enviada, sizeof(mensagem_enviada));
    }
}

void enviar_mensagem_publica(Cliente *cliente, char *mensagem) {
    char mensagem_enviada[BUFFER_SIZE];
    char porta[6];

    strncat(mensagem_enviada, "[", sizeof("["));
    strncat(mensagem_enviada, cliente->IP, strlen(cliente->IP));
    strncat(mensagem_enviada, ":", sizeof(":"));
    sprintf(porta, "%d",clientes->PORTA);
    strncat(mensagem_enviada, porta, strlen(porta));
    strncat(mensagem_enviada, "] ", sizeof("] "));
    strncat(mensagem_enviada, "nome", sizeof("nome"));
    strncat(mensagem_enviada, " >>> ", sizeof(" >>> "));
    strncat(mensagem_enviada, mensagem, strlen(mensagem));

    for (int i = 0; i < MAX_CONEXOES; i++) {
        if (clientes[i].cliente_socket != cliente->cliente_socket && clientes[i].cliente_socket > 0) {
            enviar_resposta(&clientes[i], mensagem_enviada);
        }
    }

    limpar_buffer_mensagem(porta, sizeof(porta));
    limpar_buffer_mensagem(mensagem_enviada, sizeof(mensagem_enviada));
}

void limpar_buffer_mensagem(char *mensagem, int tamanho) {
    bzero(mensagem, tamanho);
}

void limpar_buffer_cliente(int socket_desconectado) {
    for (int i = 0; i < MAX_CONEXOES; i++) {
        if (clientes[i].cliente_socket == socket_desconectado) {
            bzero(&clientes[i].cliente_socket, sizeof(clientes[i].cliente_socket));
            bzero(&clientes[i].IP, sizeof(clientes[i].IP));
            bzero(&clientes[i].PORTA, sizeof(clientes[i].PORTA));
            bzero(clientes[i].mensagem_cliente, sizeof(clientes[i].mensagem_cliente));
            bzero(&clientes[i].cfg_cliente, sizeof(clientes[i].cfg_cliente));
            total_conexoes--;
            break;
        }
    }
}
