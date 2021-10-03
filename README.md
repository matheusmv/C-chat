# C-chat

Implementação de uma aplicação de chat utilizando sockets na linguagem de programação C.

## Executando o programa

- [gcc](https://gcc.gnu.org/)
- [make](https://www.gnu.org/software/make/)

### Cliente
      - make client
      - ./client server_ip server_port username
      - comandos:
            - desconectar -> :exit:
            - listar usuários -> :list:
            - mensagem privada -> :send: <msg> username
            - mensagem pública -> qualquer texto diferente dos comandos acima será enviado publicamente.

### Servidor
      - make server
      - ./server server_port
