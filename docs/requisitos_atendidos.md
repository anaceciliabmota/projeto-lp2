# Requisitos Atendidos no Sistema de Chat Multiusuário

## 1. Threads: uso de std::thread ou pthreads para concorrer clientes/conexões
O sistema utiliza std::thread para gerenciar múltiplas conexões de clientes simultaneamente:

```// Em ChatServer::start() - threads para funções principais
client_threads.emplace_back(&ChatServer::acceptClients, this);
client_threads.emplace_back(&ChatServer::logStats, this);

// Em ChatServer::acceptClients() - thread separada para cada cliente
std::thread client_handler(&ChatServer::handleClient, this, client);
client_handler.detach();
```
*Explicação*: O servidor cria uma thread dedicada para cada cliente conectado, permitindo atendimento concorrente a múltiplos usuários. Além disso, mantém threads separadas para aceitar novas conexões e para logging de estatísticas.

## 2. Exclusão mútua
O sistema implementa exclusão mútua em vários componentes para evitar condições de corrida:
```
// Em libtslog.cpp - mutex para proteção do arquivo de log
pthread_mutex_t log_mutex;
pthread_mutex_lock(&log_mutex);
// Operações no arquivo de log
pthread_mutex_unlock(&log_mutex);

// Em ThreadSafeClientList - mutex para proteção da lista de clientes
mutable std::mutex mutex;
std::lock_guard<std::mutex> lock(mutex); // Usado em todos os métodos que acessam 'clients'
```

*Explicação*: Mutexes são usados para proteger recursos compartilhados como o arquivo de log e a lista de clientes, garantindo que apenas uma thread possa modificá-los por vez, evitando race conditions.

## 3. Semáforos e condvars: controle de filas, slots, sincronização

O sistema utiliza variáveis de condição para sincronização entre threads:

```
// Em MessageQueue.h - implementação de fila de mensagens com condvar
std::mutex mutex;
std::condition_variable cv;

void push(const Message& msg) {
    std::unique_lock<std::mutex> lock(mutex);
    queue.push(msg);
    cv.notify_one();  // Notifica thread consumidora
}

Message pop() {
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait(lock, [this]{ return !queue.empty(); });  // Espera até ter mensagens
    Message msg = queue.front();
    queue.pop();
    return msg;
}
```

*Explicação*: Variáveis de condição são usadas para implementar o padrão produtor-consumidor na fila de mensagens, permitindo que threads consumidoras aguardem a chegada de novas mensagens e sejam notificadas quando disponíveis.

## 4. Monitores: encapsular sincronização em classes
O sistema implementa o padrão Monitor em várias classes:

```
// Em ThreadSafeClientList.h
class ThreadSafeClientList {
private:
    std::vector<std::shared_ptr<Client>> clients;
    mutable std::mutex mutex;
    
public:
    void add(std::shared_ptr<Client> client) {
        std::lock_guard<std::mutex> lock(mutex);
        clients.push_back(client);
    }
    
    void remove(int client_id) {
        std::lock_guard<std::mutex> lock(mutex);
        clients.erase(std::remove_if(clients.begin(), clients.end(),
            [client_id](const auto& c) { return c->getId() == client_id; }),
            clients.end());
    }
    
    std::vector<std::shared_ptr<Client>> getAll() const {
        std::lock_guard<std::mutex> lock(mutex);
        return clients;  // Retorna cópia da lista
    }
};
```

*Explicação*: A classe ThreadSafeClientList implementa o padrão Monitor, encapsulando a estrutura de dados compartilhada (lista de clientes) e os mecanismos de sincronização (mutex) em uma única classe, garantindo que todas as operações na lista sejam thread-safe.

## 5. Sockets
O sistema implementa comunicação via sockets TCP:

```
// Em ChatServer.cpp - criação e configuração de socket
server_socket = socket(AF_INET, SOCK_STREAM, 0);
if (server_socket == -1) {
    throw std::runtime_error("Falha ao criar socket");
}

// Configuração do endereço
struct sockaddr_in server_addr;
server_addr.sin_family = AF_INET;
server_addr.sin_addr.s_addr = INADDR_ANY;
server_addr.sin_port = htons(port);

// Bind e listen
bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
listen(server_socket, 10);

// Em handleClient - comunicação através de socket
char buffer[BUFFER_SIZE];
int bytes_received = recv(client->getSocket(), buffer, BUFFER_SIZE - 1, 0);
buffer[bytes_received] = '\0';

// Envio de mensagem para cliente
send(client->getSocket(), response.c_str(), response.length(), 0);
```

*Explicação*: O sistema implementa comunicação em rede através de sockets TCP, permitindo conexões cliente-servidor e troca de mensagens entre usuários remotos. São usadas as funções padrão de socket como socket(), bind(), listen(), accept(), send() e recv().

## 6. Gerenciamento de recursos
O sistema gerencia recursos como memória, sockets e threads:

```
// Smart pointers para gerenciamento automático de memória
auto client = std::make_shared<Client>(client_socket, next_client_id++);
clients.add(client);

// RAII para gerenciamento de mutex
{
    std::lock_guard<std::mutex> lock(mutex);
    // Código protegido
} // Mutex liberado automaticamente ao sair do escopo

// Fechamento adequado de sockets no destrutor
ChatServer::~ChatServer() {
    running = false;
    for (auto& thread : client_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    if (server_socket >= 0) {
        close(server_socket);
    }
}

// Fechamento de recursos do cliente
Client::~Client() {
    if (socket >= 0) {
        close(socket);
        socket = -1;
    }
}
```

*Explicação*: O sistema gerencia recursos como sockets, arquivos e memória usando técnicas como RAII (Resource Acquisition Is Initialization), smart pointers, fechamento explícito de recursos em destrutores e join de threads para garantir que todos os recursos sejam liberados adequadamente.

## 7. Tratamento de erros: exceções e mensagens amigáveis no CLI
O sistema implementa tratamento de erros robusto:

```
// Em main_server.cpp - tratamento de exceções de alto nível
try {
    ChatServer server(PORT);
    server.start();
} catch (const std::exception& e) {
    std::cerr << "Erro fatal: " << e.what() << std::endl;
    log_error("Erro fatal: " + std::string(e.what()));
    return 1;
}

// Em handleClient - tratamento de erros de socket
int bytes_received = recv(client->getSocket(), buffer, BUFFER_SIZE - 1, 0);
if (bytes_received < 0) {
    log_error("Erro ao receber dados do cliente " + std::to_string(client->getId()));
    return;
} else if (bytes_received == 0) {
    log_info("Cliente " + std::to_string(client->getId()) + " desconectou");
    return;
}

// Em parser.cpp - validação de entrada e feedback amigável
try {
    Message msg = Message::deserialize(input);
    // Processamento normal
} catch (const std::invalid_argument& e) {
    std::string error_msg = "Formato de mensagem inválido: " + std::string(e.what());
    send(client->getSocket(), error_msg.c_str(), error_msg.length(), 0);
    log_warning(error_msg);
}
```

*Explicação*: O sistema implementa tratamento de exceções em níveis críticos e fornece mensagens de erro informativas, tanto para desenvolvedores (logs) quanto para usuários finais na interface de linha de comando. Erros de rede, formatação de mensagens e outros problemas são detectados e tratados adequadamente.

## 8. Logging concorrente: uso obrigatório da biblioteca libtslog
O sistema utiliza a biblioteca libtslog para logging thread-safe:
```
// Inicialização do logger thread-safe
init_logger("server.log");

// Uso do logger em múltiplas threads concorrentes
log_info("Servidor inicializado na porta " + std::to_string(port));
log_info("Nova conexão de cliente: " + std::string(client_ip));
log_warning("Tentativa de acesso inválida de " + std::string(ip_address));
log_error("Falha ao processar mensagem: " + error_description);

// Implementação thread-safe em libtslog.cpp
void log_message(LogLevel level, const std::string& message) {
    pthread_mutex_lock(&log_mutex);
    if (log_file.is_open()) {
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::system_clock::to_time_t(now);
        log_file << std::put_time(std::localtime(&timestamp), "%Y-%m-%d %H:%M:%S") 
                << " [" << log_level_to_string(level) << "] " 
                << message << std::endl;
    }
    pthread_mutex_unlock(&log_mutex);
}
```
*Explicação*: O sistema utiliza a biblioteca libtslog para registrar eventos de forma thread-safe, com proteção por mutex para garantir que operações de escrita no arquivo de log sejam atômicas, mesmo quando realizadas por múltiplas threads concorrentes.

## 9. Web: servidor responde a GET básico
O sistema implementa um servidor HTTP básico:
```
// Em handleHttpRequest - processamento de requisições HTTP
void handleHttpRequest(int client_socket) {
    char buffer[BUFFER_SIZE] = {0};
    recv(client_socket, buffer, BUFFER_SIZE, 0);
    
    if (strncmp(buffer, "GET", 3) == 0) {
        std::string path = extractPath(buffer);
        if (path == "/" || path == "/index.html") {
            sendHttpResponse(client_socket, "200 OK", "text/html", 
                "<html><body><h1>Chat Server Status</h1><p>Online clients: " + 
                std::to_string(clients.size()) + "</p></body></html>");
        } else {
            sendHttpResponse(client_socket, "404 Not Found", "text/html", 
                "<html><body><h1>404 Not Found</h1></body></html>");
        }
    }
}

void sendHttpResponse(int socket, const std::string& status, const std::string& content_type, 
                     const std::string& body) {
    std::string response = "HTTP/1.1 " + status + "\r\n";
    response += "Content-Type: " + content_type + "\r\n";
    response += "Content-Length: " + std::to_string(body.length()) + "\r\n";
    response += "Connection: close\r\n\r\n";
    response += body;
    
    send(socket, response.c_str(), response.length(), 0);
}
```
*Explicação*: O sistema implementa um servidor HTTP básico que responde a requisições GET, permitindo verificar o status do servidor de chat através de um navegador web. O servidor fornece uma página HTML simples mostrando estatísticas como o número de clientes conectados.

## 10. Agendador: cliente envia job e servidor reconhece
O sistema implementa um agendador simples de tarefas:

```
// Em MessageProcessor.h - implementação de agendador de tarefas
class JobScheduler {
private:
    std::queue<Job> job_queue;
    std::mutex mutex;
    std::condition_variable cv;
    std::atomic<bool> running{true};
    std::thread worker_thread;
    
public:
    JobScheduler() {
        worker_thread = std::thread(&JobScheduler::processJobs, this);
    }
    
    ~JobScheduler() {
        running = false;
        cv.notify_all();
        if (worker_thread.joinable()) {
            worker_thread.join();
        }
    }
    
    void scheduleJob(const Job& job) {
        std::lock_guard<std::mutex> lock(mutex);
        job_queue.push(job);
        cv.notify_one();
    }
    
private:
    void processJobs() {
        while (running) {
            Job job;
            {
                std::unique_lock<std::mutex> lock(mutex);
                cv.wait(lock, [this] { return !job_queue.empty() || !running; });
                if (!running && job_queue.empty()) break;
                job = job_queue.front();
                job_queue.pop();
            }
            
            // Executa o job
            log_info("Executando job: " + job.description);
            job.execute();
        }
    }
};

// Em handleClient - recebimento e agendamento de jobs
if (message.getType() == MessageType::JOB_REQUEST) {
    Job job(message.getContent(), client->getId());
    log_info("Job recebido do cliente " + std::to_string(client->getId()));
    jobScheduler.scheduleJob(job);
    
    // Confirmação para o cliente
    std::string confirmation = "Job agendado com sucesso";
    send(client->getSocket(), confirmation.c_str(), confirmation.length(), 0);
}
```
*Explicação*: O sistema implementa um agendador de tarefas que permite aos clientes enviar jobs para execução assíncrona no servidor. Um thread worker dedicado processa os jobs da fila, e o cliente recebe uma confirmação quando seu job é agendado. O sistema utiliza variáveis de condição para sincronização eficiente entre o thread principal e o worker.