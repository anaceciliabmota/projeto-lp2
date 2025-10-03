# Funcionamento do Sistema de Chat Multiusuário

## 1. Arquitetura Geral

O sistema de chat multiusuário é composto por quatro componentes principais:

- **ChatServer**: Gerenciador central de conexões e distribuição de mensagens
- **Client**: Representação de um usuário conectado ao sistema
- **Message**: Estrutura de dados para transmissão de mensagens
- **ThreadSafeClientList**: Monitor para gerenciar a lista de clientes de forma concorrente

## 2. Fluxo de Execução
OBS: não implementado ainda, apenas a ideia central para a criação dos headers

### Inicialização do Servidor
1. O servidor é iniciado na função `main()`
2. Um objeto `ChatServer` é criado com uma porta específica
3. O método `start()` é chamado para iniciar o loop principal

### Aceitação de Conexões
1. No método `acceptClients()`, o servidor fica em loop aguardando conexões
2. Quando uma conexão é aceita, cria-se um objeto `Client`
3. O cliente é adicionado à lista thread-safe
4. Uma nova thread é criada para gerenciar a comunicação com este cliente

### Processamento de Mensagens
1. Cada thread cliente executa o método `handleClient()`
2. A thread aguarda mensagens do cliente via socket
3. Quando uma mensagem é recebida, ela é:
   - Deserializada em um objeto `Message`
   - Distribuída para outros clientes via `broadcastMessage()`
   - Registrada no sistema de log usando `libtslog`

### Encerramento de Conexão
1. Quando um cliente desconecta, sua thread detecta e remove-o da lista
2. O socket é fechado e os recursos são liberados
3. A thread termina sua execução

## 3. Mecanismos de Concorrência

### Threads
- Cada cliente é gerenciado por uma thread dedicada
- Uma thread principal aceita novas conexões
- A biblioteca `pthread` (via std::thread) gerencia o ciclo de vida das threads

### Exclusão Mútua
- Mutexes protegem o acesso à lista de clientes
- O mutex da biblioteca `libtslog` protege operações de escrita no log

### Monitor
- A classe `ThreadSafeClientList` implementa o padrão Monitor
- Encapsula a lista de clientes e operações de acesso
- Protege operações de adição, remoção e consulta com mutex

## 4. Comunicação via Sockets

### Protocolo
- Conexão TCP para comunicação confiável
- Mensagens serializadas em formato texto
- Formato: `[ID]:[NOME]:[TIMESTAMP]:[CONTEÚDO]`

### Operações de Socket
- `socket()`: Cria um socket de comunicação
- `bind()`: Associa o socket a uma porta
- `listen()`: Configura o socket para aceitar conexões
- `accept()`: Aceita uma nova conexão de cliente
- `send()/recv()`: Envia e recebe dados pelo socket

## 5. Tratamento de Erros

- Verificação de erros em operações de socket
- Tratamento de desconexões inesperadas
- Logging de erros e eventos usando a biblioteca `libtslog`
- Recuperação de falhas sem comprometer todo o sistema

## 6. Integração com libtslog

- Logging de eventos importantes:
  - Inicialização do servidor
  - Conexões e desconexões de clientes
  - Mensagens recebidas e enviadas
  - Erros e exceções

- Thread-safety garantida pelo uso de mutex na biblioteca