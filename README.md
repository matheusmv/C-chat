# chat-client-server-c-programming
Implementação de uma aplicação de chat utilizando sockets na linguagem de programação C.

## Executando o programa

- [gcc](https://gcc.gnu.org/)
- [make](https://www.gnu.org/software/make/)

### Cliente
      - make client
      - ./cliente/cliente <IP> <PORTA> <USUARIO>

### Servidor
      - make server
      - ./servidor/servidor <PORTA>

## Progresso (Arquitetura do Sistema)

- [x] Implementar estrutura/operações básicas do cliente.
- [x] Implementar estrutura/operações básicas do servidor.
- [x] Connect persistente (Cliente).
- [x] Accept persistente (Servidor).
- [x] Implementar gerenciamento de multiplas conexões (Servidor).
      
      Obs.: solução ineficiente, pode gerar alto consumo de processamento/memória.

- [ ] Implementar etapa de identificação/autenticação cliente -> servidor (Cliente/Servidor).
- [x] Implementar gerenciamento de informações dos clientes (Servidor).
- [ ] Implementar direcionamento de mensagens cliente -> servidor -> cliente (Servidor).

## Progresso (Operações exigidas na especificação do projeto)

- [ ] conectar
  - [ ] conectar 'login' 'senha' 'servidor' 'porta'
  - [x] Conecta-se ao servidor, vinculando 'login' ao IP e porta do cliente origem.
  - [x] Informar em caso de nome já existente ou de servidor indisponível.
- [x] listar
  - [x] listar
  - [x] Lista todos os participantes online no servidor mostrando, IP, porta e nome.
  - [x] Informar em caso de nenhum participante online.
- [ ] enviar
  - [ ] enviar 'msg' 'participante'
  - [x] Caso o participante não seja informado, a mensagem é enviada de forma pública.
  - [ ] Informar em caso do participante destino não estar online.
