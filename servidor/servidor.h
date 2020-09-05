#ifndef _SERVIDOR_H_
#define _SERVIDOR_H_

#include <stdint.h>
#include <sys/socket.h>
#include <arpa/inet.h>

typedef struct servidor {
    int servidor_socket;
    struct sockaddr_in cfg_servidor;
} Servidor;

void criar_socket(Servidor *);

void configurar_servidor(Servidor *, uint16_t);

void aceitar_conexoes(Servidor *);

#endif