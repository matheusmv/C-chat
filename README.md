# C-chat

chat utilizando sockets na linguagem de programação C.

    Projeto desenvolvido durante a disciplina de sistemas distribuidos.

## Compilando

 - [gcc](https://gcc.gnu.org/)
 - [make](https://www.gnu.org/software/make/)

 - Makefile:

       git clone https://github.com/matheusmv/C-chat.git && cd C-chat && make

 - Terminal:

       git clone https://github.com/matheusmv/C-chat.git && cd C-chat

       gcc -O2 src/includes/*.c src/client/*.c -o client -lpthread -DLCOLOR

       gcc -O2 src/includes/*.c src/server/*.c -o server -lpthread -DLCOLOR

## Executando o programa

### Servidor

      ./server -p <port>

### Cliente

      ./client -a <IP-address> -p <port> -u <your-username>

 - Comandos:

      - desconectar:

            :exit:

      - listar usuários online:

            :list:

      - enviar mensagem privada:

            :send: -u <username> -m <message>

      - mensagem pública:

            qualquer texto diferente dos comandos acima será enviado publicamente.
