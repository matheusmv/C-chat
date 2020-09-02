#ifndef _CLIENTE_H_
#define _CLIENTE_H_

#include <stdint.h>
#include <sys/socket.h>
#include <arpa/inet.h>

typedef struct _servidor {
    struct sockaddr_in cfg_servidor;
    char *IP_SERVIDOR;
    uint16_t PORTA;
} Servidor;

typedef struct _cliente {
    int cliente_socket;
} Cliente;

void criar_socket(Cliente *);

void conectar(Cliente *, char *, uint16_t);

void enviar_mensagem(Cliente *);

void receber_mensagem(Cliente *);

void fechar_conexao(Cliente *);

#endif