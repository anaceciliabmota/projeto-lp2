# Requisitos Atendidos no Sistema de Chat Multiusuário

## 1. Threads: uso de std::thread ou pthreads para concorrer clientes/conexões
O sistema utiliza std::thread para gerenciar múltiplas conexões de clientes simultaneamente. O trecho de codigo a seguir pode ser encontrado no arquivo [server.cpp](../src/server.cpp)

```
 // Cria uma thread desvinculada para tratar este cliente
    std::thread client_handler(&ChatServer::handleClient, this, client);
    client_handler.detach();
```
*Explicação*: O servidor cria uma thread dedicada para cada cliente conectado, permitindo atendimento concorrente a múltiplos usuários. Além disso, mantém threads separadas para aceitar novas conexões.

## 2. Exclusão mútua
O sistema implementa exclusão mútua em vários componentes para evitar condições de corrida. Um deles é o uso de mutex para garantir exclusão mútua na escrita do arquivo de log. O trecho apresentado a seguir pode ser encontrado no arquivo [libtslog.cpp](../src/libtslog/libtslog.cpp) 
```
// Em libtslog.cpp - mutex para proteção do arquivo de log
pthread_mutex_t log_mutex;
pthread_mutex_lock(&log_mutex);
// Operações no arquivo de log
pthread_mutex_unlock(&log_mutex);

```

*Explicação*: Mutexes são usados para proteger recursos compartilhados como o arquivo de log.

## 3. Monitores: encapsular sincronização em classes
O sistema implementa o padrão Monitor para ter uma lista de clientes thread safe. A implementação em detalhes pode ser observada no arquivo [thread_safe_client_list](../src/thread_safe_client_list.cpp)

```

// Add a client to the list in a thread-safe way
void ThreadSafeClientList::add(std::shared_ptr<Client> client) {
    std::lock_guard<std::mutex> lock(mutex);
    clients.push_back(client);
}

// Remove a client from the list in a thread-safe way
void ThreadSafeClientList::remove(int client_id) {
    std::lock_guard<std::mutex> lock(mutex);
    clients.erase(
        std::remove_if(clients.begin(), clients.end(),
            [client_id](const std::shared_ptr<Client>& c) {
                return c->getId() == client_id;
            }),
        clients.end()
    );
}

```

*Explicação*: A classe ThreadSafeClientList implementa o padrão Monitor, encapsulando a estrutura de dados compartilhada (lista de clientes) e os mecanismos de sincronização (mutex) em uma única classe, garantindo que todas as operações na lista sejam thread-safe.

## 4. Sockets
O sistema implementa comunicação via sockets TCP. O trecho de codigo a seguir está presente no aruqivo [client.cpp](../src/client.cpp)
```
/ Em client.cpp - linha ~30
ssize_t bytes_sent = send(socket, serialized.c_str(), serialized.length(), 0);

// Em client.cpp - linha ~58
ssize_t bytes_received = recv(socket, buffer, BUFFER_SIZE - 1, 0);
```

*Explicação*: O sistema implementa comunicação em rede através de sockets TCP, permitindo conexões cliente-servidor e troca de mensagens entre usuários remotos. São usadas as funções padrão de socket como socket(), bind(), listen(), accept(), send() e recv().

## 5. Gerenciamento de recursos
O sistema gerencia recursos como  sockets:

```
// Em client.cpp - linhas ~16-21
Client::~Client() {
    // Fecha apenas se o socket for válido
    if (socket >= 0) {
        close(socket);
        socket = -1;
    }
}
```

*Explicação*: O sistema implementa o padrão RAII (Resource Acquisition Is Initialization) para garantir a liberação adequada de recursos. No destrutor da classe Client, os sockets são automaticamente fechados quando o objeto é destruído, evitando vazamentos de recursos. A verificação de validade do socket antes do fechamento (socket >= 0) previne erros de fechamento em descritores inválidos. 

## 6. Tratamento de erros: exceções e mensagens amigáveis no CLI
O sistema implementa tratamento de erros, um exemplo está no arquivo [client.cpp](../src/client.cpp):

```
/// Em client.cpp - linhas ~29-39
try {
    // Serializa a mensagem para formato de string
    std::string serialized = msg.serialize();
    
    // Envia a mensagem através do socket
    ssize_t bytes_sent = send(socket, serialized.c_str(), serialized.length(), 0);
    
    // Verifica se o envio foi bem-sucedido
    if (bytes_sent < 0) {
        std::cerr << "Erro ao enviar mensagem: " << strerror(errno) << std::endl;
        return false;
    }
```

*Explicação*: 

## 7. Logging concorrente: uso obrigatório da biblioteca libtslog
O sistema utiliza a biblioteca libtslog para logging thread-safe:
```
/ Em libtslog/libtslog.cpp ou similar
void log_message(const std::string& message) {
    std::lock_guard<std::mutex> lock(log_mutex);
    // Escrita no arquivo
}
```
*Explicação*: O sistema utiliza a biblioteca libtslog para registrar eventos de forma thread-safe, com proteção por mutex para garantir que operações de escrita no arquivo de log sejam atômicas, mesmo quando realizadas por múltiplas threads concorrentes.

## 8. Documentação
 [funcionamento.md](docs/funcionamento.md): Descreve o fluxo de execução e a arquitetura geral, explica os componentes principais e suas interações e detalha os mecanismos de concorrência utilizados 

Presente documento: Verificação dos requisitos atendidos

Este diagrama ilustra o fluxo de comunicação entre os clientes e o servidor de chat multiusuário, com ênfase na concorrência e no uso da lista de clientes thread-safe.
![Diagrama de Sequência do Chat](docs/diagrama_sequencia.png)

## 9. Makefile
[makefile](../makefile)

## 10. Uso de IA

[relatorio-final-llm.md](relatorio-llm-1.md]): Análise do desenvolvimento com IA Geral da etapa 1

[relatorio-final-llm.md](relatorio-llm2.md]): Análise do desenvolvimento com IA Geral da etapa 2

[relatorio-final-llm.md](relatorio-final-llm.md): Análise do desenvolvimento com IA Geral de todas as etapas

