#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

/* Types */

#define MAX_CONEXOES 5
#define BUFFER_SIZE 4096
#define SERVIDOR_BACKLOG 5

typedef struct servidor {
    uint16_t servidor_socket;
    struct sockaddr_in cfg_servidor;
} Servidor;

typedef struct cliente {
    char usuario[50];
    char *IP;
    uint16_t PORTA;
    int cliente_socket;
    char mensagem_cliente[BUFFER_SIZE];
    struct sockaddr_in cfg_cliente;
} Cliente;

/* Function declarations */

void criar_socket(Servidor *);

void configurar_servidor(Servidor *, uint16_t);

void aceitar_conexoes(Servidor *);

void enviar_resposta(Cliente *, char *);

void *func_thread_servidor(void *);

void inicializar_clientes(Cliente *);

int auth_cliente(Cliente *);

void registrar_cliente(Cliente *);

void listar_clientes(Cliente *);

void construir_mensagem(Cliente *, char *);

void enviar_mensagem_publica(Cliente *);

void enviar_mensagem_privada(Cliente *);

void limpar_buffer_mensagem(char *, int);

void limpar_buffer_cliente(int);

/* Global variables */

Cliente clientes[MAX_CONEXOES];

int total_conexoes = 0;

/* Function definitions */

void criar_socket(Servidor *servidor)
{
    servidor->servidor_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (servidor->servidor_socket == -1) {
        printf("Erro ao criar o socket\n");
        exit(EXIT_FAILURE);
    }

    printf("Socket criado\n");
}

void configurar_servidor(Servidor *servidor, uint16_t PORTA)
{
    servidor->cfg_servidor.sin_family = AF_INET;
    servidor->cfg_servidor.sin_addr.s_addr = INADDR_ANY;
    servidor->cfg_servidor.sin_port = htons(PORTA);

    if (bind(servidor->servidor_socket, (struct sockaddr *)&servidor->cfg_servidor, sizeof(servidor->cfg_servidor)) < 0) {
        printf("Erro ao configurar o servidor\n");
        exit(EXIT_FAILURE);
    }

    printf("Servidor rodando na porta %d\n", PORTA);

    listen(servidor->servidor_socket, SERVIDOR_BACKLOG);
}

void aceitar_conexoes(Servidor *servidor)
{
    inicializar_clientes(clientes);

    Cliente cliente = *(Cliente *)calloc(1, sizeof(Cliente));

    pthread_t server_thread;
    Cliente *p_cliente = malloc(sizeof(Cliente));

    int addr_len = sizeof(struct sockaddr_in);

    while (1) {

        cliente.cliente_socket = accept(servidor->servidor_socket, (struct sockaddr *)&cliente.cfg_cliente, (socklen_t *)&addr_len);

        if (cliente.cliente_socket < 0) {
            perror("Erro ao aceitar conexões");
            exit(EXIT_FAILURE);
        }

        cliente.IP = inet_ntoa(cliente.cfg_cliente.sin_addr);
        cliente.PORTA = ntohs(cliente.cfg_cliente.sin_port);

        if (total_conexoes < MAX_CONEXOES && auth_cliente(&cliente) == 1) {
            registrar_cliente(&cliente);
            *p_cliente = cliente;
            pthread_create(&server_thread, NULL, func_thread_servidor, (void *)p_cliente);
        }
        else {
            printf("Conexão rejeitada - IP: %s PORTA: %d\n", cliente.IP, cliente.PORTA);
            char *mensagem_status = "Máximo de clientes alcaçado\n";
            enviar_resposta(&cliente, mensagem_status);
            close(cliente.cliente_socket);
        }
    }
}

void enviar_resposta(Cliente *cliente, char *mensagem)
{
    send(cliente->cliente_socket, mensagem, strlen(mensagem), 0);
}

void *func_thread_servidor(void *argumento)
{
    Cliente cliente = *(Cliente *)argumento;

    printf("Conexão bem sucedida - IP: %s PORTA: %d\n", cliente.IP, cliente.PORTA);

    int addr_len = sizeof(struct sockaddr_in);
    int len;

    char *listar = ":listar\r\n";
    char *mensagem_privada = ":enviar";

    while (1) {
        len = recv(cliente.cliente_socket, cliente.mensagem_cliente, sizeof(cliente.mensagem_cliente), 0);

        if (len == 0) {
            printf("Cliente desconectado - IP: %s PORTA: %d\n", cliente.IP, cliente.PORTA);
            close(cliente.cliente_socket);
            limpar_buffer_cliente(cliente.cliente_socket);
            return NULL;
        }
        else if (len == -1) {
            printf("Falha ao receber mensagem\n");
        }

        if (strcmp(cliente.mensagem_cliente, listar) == 0) {
            listar_clientes(&cliente);
        }
        else if (strncmp(cliente.mensagem_cliente, mensagem_privada, strlen(mensagem_privada)) == 0) {
            enviar_mensagem_privada(&cliente);
            continue;
        }
        else {
            printf("[%s:%d] %s >>> %s", cliente.IP, cliente.PORTA, cliente.usuario, cliente.mensagem_cliente);
            enviar_mensagem_publica(&cliente);
        }

        limpar_buffer_mensagem(cliente.mensagem_cliente, sizeof(cliente.mensagem_cliente));
    }

    return NULL;
}

void inicializar_clientes(Cliente *clientes)
{
    for (int i = 0; i < MAX_CONEXOES; i++) {
        clientes[i] = *(Cliente *)calloc(1, sizeof(Cliente));
    }
}

int auth_cliente(Cliente *cliente)
{
    int status = 0;

    recv(cliente->cliente_socket, cliente->mensagem_cliente, sizeof(cliente->mensagem_cliente), 0);
    strncpy(cliente->usuario, cliente->mensagem_cliente, sizeof(cliente->usuario));
    limpar_buffer_mensagem(cliente->mensagem_cliente, sizeof(cliente->mensagem_cliente));

    for (int i = 0; i < MAX_CONEXOES; i++) {
        if (strcmp(clientes[i].usuario, cliente->usuario) == 0) {
            char *resposta = "usuário já cadastrado, tente novamente.\r\n";
            enviar_resposta(cliente, resposta);
            close(cliente->cliente_socket);
            limpar_buffer_cliente(cliente->cliente_socket);
            return status;
        }
    }

    char *resposta = "sucesso\r\n";
    enviar_resposta(cliente, resposta);
    status = 1;

    return status;
}

void registrar_cliente(Cliente *novo_cliente)
{
    for (int i = 0; i < MAX_CONEXOES; i++) {
        if (clientes[i].cliente_socket <= 0) {
            clientes[i] = *novo_cliente;
            total_conexoes++;
            break;
        }
    }
}

void listar_clientes(Cliente *cliente)
{
    char mensagem_enviada[BUFFER_SIZE];
    char porta[6];

    if (total_conexoes == 1) {
        char *ninguem_online = "Não há usuários online no momemto.\n";
        enviar_resposta(cliente, ninguem_online);
    }
    else {
        for (int i = 0; i < MAX_CONEXOES; i++) {
            if (clientes[i].cliente_socket != cliente->cliente_socket && clientes[i].cliente_socket > 0) {
                strncat(mensagem_enviada, "[", sizeof("["));
                strncat(mensagem_enviada, clientes[i].IP, strlen(clientes[i].IP));
                strncat(mensagem_enviada, ":", sizeof(":"));
                sprintf(porta, "%d", clientes[i].PORTA);
                strncat(mensagem_enviada, porta, strlen(porta));
                strncat(mensagem_enviada, "] ", sizeof("] "));
                strncat(mensagem_enviada, clientes[i].usuario, strlen(clientes[i].usuario));
                strncat(mensagem_enviada, "\r\n", sizeof("\r\n"));
                limpar_buffer_mensagem(porta, sizeof(porta));
            }
        }

        enviar_resposta(cliente, mensagem_enviada);

        limpar_buffer_mensagem(mensagem_enviada, sizeof(mensagem_enviada));
    }
}

void construir_mensagem(Cliente *cliente_emissor, char *mensagem_buffer)
{
    char porta[6];
    char horario[50];

    limpar_buffer_mensagem(porta, sizeof(porta));
    limpar_buffer_mensagem(horario, sizeof(horario));

    time_t horario_atual;
    time(&horario_atual);
    strncat(horario, ctime(&horario_atual), sizeof(horario));

    strncat(mensagem_buffer, "Msg de ", sizeof("Msg de "));
    strncat(mensagem_buffer, cliente_emissor->usuario, strlen(cliente_emissor->usuario));
    strncat(mensagem_buffer, " [", sizeof(" ["));
    strncat(mensagem_buffer, cliente_emissor->IP, strlen(cliente_emissor->IP));
    strncat(mensagem_buffer, ":", sizeof(":"));
    sprintf(porta, "%d", clientes->PORTA);
    strncat(mensagem_buffer, porta, strlen(porta));
    strncat(mensagem_buffer, "]:", sizeof("]:"));
    strncat(mensagem_buffer, "[", sizeof("["));
    strncat(mensagem_buffer, horario, strlen(horario) - 1);
    strncat(mensagem_buffer, "] ", sizeof("] "));
    strncat(mensagem_buffer, cliente_emissor->mensagem_cliente, strlen(cliente_emissor->mensagem_cliente));
}

void enviar_mensagem_publica(Cliente *cliente)
{
    char mensagem_enviada[BUFFER_SIZE];

    construir_mensagem(cliente, mensagem_enviada);

    for (int i = 0; i < MAX_CONEXOES; i++) {
        if (clientes[i].cliente_socket != cliente->cliente_socket && clientes[i].cliente_socket > 0) {
            enviar_resposta(&clientes[i], mensagem_enviada);
        }
    }

    limpar_buffer_mensagem(mensagem_enviada, sizeof(mensagem_enviada));
}

void enviar_mensagem_privada(Cliente *cliente)
{
    char mensagem_enviada[BUFFER_SIZE];
    char usuario_destino[50];
    int envio_confirmado = 0;

    if (total_conexoes == 1) {
        char *ninguem_online = "Não há usuários online no momemto.\n";
        enviar_resposta(cliente, ninguem_online);
        limpar_buffer_mensagem(cliente->mensagem_cliente, sizeof(cliente->mensagem_cliente));
    }
    else {
        for (int i = 0; i < strlen(cliente->mensagem_cliente); i++) {
            // capturando a mensagem
            if ((int)cliente->mensagem_cliente[i] == 60) {
                i++;
                int aux = 0;

                while ((int)cliente->mensagem_cliente[i] != 62) {
                    mensagem_enviada[aux++] = cliente->mensagem_cliente[i++];
                }

                strncat(mensagem_enviada, "\r\n", sizeof("\r\n"));
            }
            // capturando destinatário
            if ((int)cliente->mensagem_cliente[i] == 62) {
                i += 2;
                int aux = 0;

                while ((int)cliente->mensagem_cliente[i] != 13) {
                    usuario_destino[aux++] = cliente->mensagem_cliente[i++];
                }
            }
        }

        for (int i = 0; i < MAX_CONEXOES; i++) {
            if (strncmp(clientes[i].usuario, usuario_destino, strlen(usuario_destino)) == 0) {
                limpar_buffer_mensagem(cliente->mensagem_cliente, sizeof(cliente->mensagem_cliente));
                strncpy(cliente->mensagem_cliente, mensagem_enviada, sizeof(cliente->mensagem_cliente));
                limpar_buffer_mensagem(mensagem_enviada, sizeof(mensagem_enviada));
                construir_mensagem(cliente, mensagem_enviada);
                enviar_resposta(&clientes[i], mensagem_enviada);
                envio_confirmado = 1;
                break;
            }
        }

        if (envio_confirmado == 0) {
            char *ninguem_online = "Usuário não está online no momemto.\n";
            enviar_resposta(cliente, ninguem_online);
        }
    }

    limpar_buffer_mensagem(cliente->mensagem_cliente, sizeof(cliente->mensagem_cliente));
    limpar_buffer_mensagem(mensagem_enviada, sizeof(mensagem_enviada));
    limpar_buffer_mensagem(usuario_destino, sizeof(usuario_destino));
}

void limpar_buffer_mensagem(char *mensagem, int tamanho)
{
    bzero(mensagem, tamanho);
}

void limpar_buffer_cliente(int socket_desconectado)
{
    for (int i = 0; i < MAX_CONEXOES; i++) {
        if (clientes[i].cliente_socket == socket_desconectado) {
            bzero(&clientes[i].usuario, sizeof(clientes[i].usuario));
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

/* main */

int main(int argc, char *argv[])
{
    Servidor s1;

    if (argc != 2) {
        printf("%s <PORTA_SERVIDOR>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    criar_socket(&s1);

    configurar_servidor(&s1, (uint16_t)atoi(argv[1]));

    aceitar_conexoes(&s1);

    return EXIT_SUCCESS;
}
