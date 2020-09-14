#ifndef _CLIENTE_H_
#define _CLIENTE_H_

#include <stdint.h>
#include <sys/socket.h>
#include <arpa/inet.h>

typedef struct cliente {
    int cliente_socket;
} Cliente;

void criar_socket(Cliente *);

void conectar(Cliente *, char *, uint16_t, char *);

void iniciar_chat(Cliente *);

void fechar_conexao(Cliente *);

#endif
