# Servidor de Chat Multiusuário - Etapa 1

Este projeto implementa um servidor de chat multiusuário com foco em programação concorrente e comunicação em rede. A primeira etapa consiste na implementação de uma biblioteca de logging thread-safe e na definição da arquitetura básica.

## Compilação e Execução

### Comandos

```bash
# Compilar o projeto
make

# Executar o teste CLI
./log_test
```

## Funcionalidades Implementadas (Etapa 1)

### 1. Biblioteca de Logging Thread-Safe (libtslog)
A biblioteca libtslog implementa um sistema de logging thread-safe com uma API clara e concisa:

[libtslog.h](src/libtslog/libtslog.h): Interface pública da biblioteca
 - init_logger(filename): Inicializa o logger com um arquivo específico
 - log_message(message): Registra uma mensagem de forma thread-safe
 - close_logger(): Finaliza o logger e libera recursos
libtslog.cpp: Implementação usando mutex para exclusão mútua

Utiliza pthread_mutex_t para garantir acesso seguro em ambiente concorrente
Gerencia o arquivo de log de forma apropriada

### 2. Teste CLI de Múltiplas Threads
O arquivo [main.cpp](src/main.cpp) implementa um teste que simula múltiplos usuários enviando mensagens:
 - Cria 10 threads simultâneas representando usuários diferentes
 - Cada thread gera 5 mensagens sequenciais
 - Demonstra a thread-safety da biblioteca de logging
 - Verifica a correta formatação e serialização das mensagens

### 3. Arquitetura Inicial
Os headers principais foram criados para definir a estrutura do sistema:

[message.h](include/message.h) e [message.cpp](src/message.cpp):Define a estrutura de mensagens
- Armazena metadados como remetente, conteúdo e timestamp
- Implementa serialização e deserialização para transmissão futura

[client.h](include/client.h): Define a interface para representação de clientes conectados
- Encapsula informações de identificação e conexão

[server.h](include/server.h): Define a interface para o servidor de chat
- Gerenciamento de conexões e broadcast de mensagens

[thread_safe_client_list.h](include/thread_safe_client_list.h): Implementa um monitor
 - Gerencia a lista de clientes de forma thread-safe

### 4. Documentação da Arquitetura
 [funcionamento.md](docs/funcionamento.md): Descreve o fluxo de execução e a arquitetura geral, explica os componentes principais e suas interações e detalha os mecanismos de concorrência utilizados 

[relatorio-llm-1.md](docs/relatorio-llm-1.md): Análise do desenvolvimento com IA
