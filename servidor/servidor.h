#ifndef _SERVIDOR_H_
#define _SERVIDOR_H_

#include <stdint.h>
#include <sys/socket.h>
#include <arpa/inet.h>

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

void criar_socket(Servidor *);

void configurar_servidor(Servidor *, uint16_t);

void enviar_resposta(Cliente *);

void* funcao_servidor(void *);

void aceitar_conexoes(Servidor *);

#endif