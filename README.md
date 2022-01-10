# C-chat

    Projeto desenvolvido durante a disciplina de sistemas distribuidos.

## Executando o programa

 - [gcc](https://gcc.gnu.org/)
 - [make](https://www.gnu.org/software/make/)

### Compilando

 - Makefile:

       git clone https://github.com/matheusmv/C-chat.git && cd C-chat && make

 - Terminal:

       git clone https://github.com/matheusmv/C-chat.git && cd C-chat

       gcc src/includes/*.c src/client/*.c -o client -lpthread

       gcc src/includes/*.c src/server/*.c -o server -lpthread

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
