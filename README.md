# chat-client-server-c-programming
Implementação de uma aplicação de chat utilizando sockets na linguagem de programação C para Linux/Unix.

## Progresso (Arquitetura do Sistema)

- [x] Implementar estrutura/operações básicas do cliente.
- [x] Implementar estrutura/operações básicas do servidor.
- [ ] Connect persistente (Cliente).
- [x] Accept persistente (Servidor).
- [ ] Implementar gerenciamento de multiplas conexões (Servidor).
- [ ] Implementar etapa de identificação/autenticação cliente -> servidor (Cliente/Servidor).
- [ ] Implementar gerenciamento de informações dos clientes (Servidor).
- [ ] Implementar direcionamento de mensagens cliente -> servidor -> cliente (Servidor).

## Progresso (Operações exigidas na especificação do projeto)

- [ ] conectar
  - conectar 'login' 'senha' 'servidor' 'porta'
  - Conecta-se ao servidor, vinculando 'login' ao IP e porta do cliente origem.
  - Informar em caso de nome já existente ou de servidor indisponível.
- [ ] listar
  - listar
  - Lista todos os participantes online no servidor mostrando, IP, porta e nome.
  - Informar em caso de nenhum participante online.
- [ ] enviar
  - enviar 'msg' 'participante'
  - Envia a mensagem 'msg' ao participante 'participante' de forma privada. Caso o participante não seja informado, a mensagem é enviada de forma pública.
  - Informar em caso do participante destino não estar online.
