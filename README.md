# C-chat

chat utilizando sockets na linguagem de programação C.

## Executando o programa

 - [gcc](https://gcc.gnu.org/)
 - [make](https://www.gnu.org/software/make/)

### Compilando

 - Makefile:

       git clone https://github.com/matheusmv/C-chat.git && cd C-chat && make

 - Terminal:

       git clone https://github.com/matheusmv/C-chat.git && cd C-chat

       gcc utils.c network.c client.c client-app.c -o client -lpthread

       gcc utils.c network.c server.c server-app.c -o server -lpthread

### Cliente

      ./client -a ip -p port -u username

 - Comandos:

      - desconectar:

            :exit:

      - listar usuários online:

            :list:

      - enviar mensagem privada:

            :send: -u username -m message

      - mensagem pública: qualquer texto diferente dos comandos acima será enviado publicamente.

### Servidor

      ./server -p port
