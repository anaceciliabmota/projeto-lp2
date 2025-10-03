#include "../include/server.h"
#include "../include/client.h"
#include "../include/message.h"
#include "../src/libtslog/libtslog.h"

#include <iostream>
#include <stdexcept>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h> // Adicione este include para fcntl

static int next_client_id = 1;  // Adicione isso no topo do arquivo se ainda não estiver presente

// Construtor: Inicializa o socket do servidor e configura-o para escutar conexões
ChatServer::ChatServer(int port) : running(true) {
    // Cria socket TCP
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        throw std::runtime_error("Falha ao criar socket do servidor");
    }
    
    // Configura opção de socket para permitir reutilização da porta (útil para reinicializações do servidor)
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        close(server_socket);
        throw std::runtime_error("Falha ao definir opções do socket");
    }
    
    // Configura endereço do servidor
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;         // IPv4
    server_addr.sin_addr.s_addr = INADDR_ANY; // Aceitar conexões em qualquer interface de rede
    server_addr.sin_port = htons(port);       // Converte porta para ordem de bytes da rede
    
    // Vincula o socket ao endereço e porta especificados
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        close(server_socket);
        throw std::runtime_error("Falha ao vincular socket à porta " + std::to_string(port));
    }
    
    // Começa a escutar por conexões de entrada (tamanho da fila = 10)
    if (listen(server_socket, 10) < 0) {
        close(server_socket);
        throw std::runtime_error("Falha ao escutar no socket");
    }
    
    log_message("Servidor inicializado e escutando na porta " + std::to_string(port));
}

// Destrutor: Limpa recursos
ChatServer::~ChatServer() {
    stop();
}

// Inicia o servidor criando uma thread para aceitar clientes
void ChatServer::start() {
    log_message("Iniciando servidor...");
    client_threads.emplace_back(&ChatServer::acceptClients, this);
    client_threads.emplace_back(&ChatServer::logStats, this);  // Adicione esta linha
    log_message("Servidor iniciado com sucesso. Aguardando conexões...");
}

// Para o servidor e limpa recursos
void ChatServer::stop() {
    log_message("Parando servidor...");
    
    // Define a flag running como false para parar loops
    running = false;
    
    // Envia uma mensagem de desligamento do servidor para todos os clientes
    Message shutdown_msg(0, "Servidor", "O servidor está sendo desligado. Até logo!");
    broadcastMessage(shutdown_msg);
    
    // Dá um momento para as threads de cliente processarem a mensagem de desligamento
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Fecha o socket do servidor para desbloquear a chamada accept()
    close(server_socket);
    
    // Espera pela conclusão da thread principal acceptClients
    for (auto& thread : client_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    
    // Fecha todas as conexões de clientes
    auto all_clients = clients.getAll();
    for (const auto& client : all_clients) {
        close(client->getSocket());
    }
    
    log_message("Servidor parado");
}

// Aceita novas conexões de clientes em um loop
void ChatServer::acceptClients() {
    struct sockaddr_in client_addr;
    socklen_t client_addr_size = sizeof(client_addr);
    
    // Configura o socket para modo não-bloqueante
    int flags = fcntl(server_socket, F_GETFL, 0);
    fcntl(server_socket, F_SETFL, flags | O_NONBLOCK);
    
    while (running) {
        // Usa select para aguardar conexões com um timeout
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(server_socket, &read_fds);
        
        // Define um timeout de 1 segundo
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
        int select_result = select(server_socket + 1, &read_fds, NULL, NULL, &timeout);
        
        // Verifica se devemos continuar executando
        if (!running) {
            break;
        }
        
        // Trata erros de select
        if (select_result < 0) {
            if (errno == EINTR) {
                // Interrompido por sinal, apenas tenta novamente
                continue;
            }
            log_message("Erro de select: " + std::string(strerror(errno)));
            continue;
        }
        
        // Verifica se temos uma nova conexão
        if (select_result > 0 && FD_ISSET(server_socket, &read_fds)) {
            int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_size);
            
            if (client_socket < 0) {
                if (errno != EAGAIN && errno != EWOULDBLOCK) {
                    log_message("Erro ao aceitar conexão de cliente: " + std::string(strerror(errno)));
                }
                continue;
            } else {
                // Obtém o endereço IP do cliente como string
                char client_ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
                
                // Log de segurança mais detalhado
                log_message("Nova conexão aceita | Socket: " + std::to_string(client_socket) + 
                            " | IP: " + std::string(client_ip) + 
                            " | Porta: " + std::to_string(ntohs(client_addr.sin_port)));
            }
            
            // Obtém o endereço IP do cliente como string
            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
            log_message("Nova conexão de " + std::string(client_ip));
            
            // Cria um novo objeto Cliente
            auto client = std::make_shared<Client>(client_socket, next_client_id++);
            
            // Adiciona cliente à lista thread-safe
            clients.add(client);
            
            // Cria uma thread desvinculada para tratar este cliente
            std::thread client_handler(&ChatServer::handleClient, this, client);
            client_handler.detach();
        }
    }
    
    log_message("Loop de aceitação terminado");
}

// Trata comunicação com um cliente específico
void ChatServer::handleClient(std::shared_ptr<Client> client) {
    const int BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;
    
    // Envia mensagem de boas-vindas para o cliente
    std::string welcome_msg = "Bem-vindo ao servidor de chat! Seu ID é: " + std::to_string(client->getId());
    send(client->getSocket(), welcome_msg.c_str(), welcome_msg.length(), 0);
    
    // Aguarda pela primeira mensagem que deve ser o nome de usuário
    memset(buffer, 0, BUFFER_SIZE);
    bytes_received = recv(client->getSocket(), buffer, BUFFER_SIZE - 1, 0);
    
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        
        try {
            // Tenta analisar como uma mensagem
            Message msg = Message::deserialize(buffer);
            
            // Define o nome de usuário do cliente para o conteúdo desta primeira mensagem
            client->setUsername(msg.getContent());
            log_message("Cliente " + std::to_string(client->getId()) + " definiu nome de usuário para: " + client->getUsername());
            
            // Agora envia a notificação de entrada com o nome de usuário real
            Message join_msg(0, "Servidor", "Usuário " + client->getUsername() + " entrou no chat");
            broadcastMessage(join_msg);
            
        } catch (const std::exception& e) {
            // Se a análise falhar, usa o nome de usuário padrão
            log_message("Erro ao definir nome de usuário: " + std::string(e.what()));
            
            // Envia notificação de entrada com nome de usuário padrão
            Message join_msg(0, "Servidor", "Usuário " + client->getUsername() + " entrou no chat");
            broadcastMessage(join_msg);
        }
    } else {
        // Se não pudermos receber uma mensagem, usa nome de usuário padrão
        Message join_msg(0, "Servidor", "Usuário " + client->getUsername() + " entrou no chat");
        broadcastMessage(join_msg);
    }
    
    // Processa mensagens do cliente até desconexão ou desligamento do servidor
    while (running) {
        // Limpa o buffer antes de receber
        memset(buffer, 0, BUFFER_SIZE);
        
        // Usa select com timeout para tornar a operação de recebimento interruptível
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(client->getSocket(), &read_fds);
        
        struct timeval timeout;
        timeout.tv_sec = 1;  // timeout de 1 segundo
        timeout.tv_usec = 0;
        
        int select_result = select(client->getSocket() + 1, &read_fds, NULL, NULL, &timeout);
        
        // Verifica se o servidor ainda está em execução
        if (!running) {
            break;
        }
        
        // Trata erros de select ou timeout
        if (select_result <= 0) {
            if (select_result < 0 && errno != EINTR) {
                log_message("Erro de select no tratador de cliente: " + std::string(strerror(errno)));
            }
            // Em timeout ou interrupção, apenas continua o loop
            continue;
        }
        
        // Recebe dados se disponíveis
        if (FD_ISSET(client->getSocket(), &read_fds)) {
            bytes_received = recv(client->getSocket(), buffer, BUFFER_SIZE - 1, 0);
            
            // Verifica desconexão ou erro
            if (bytes_received <= 0) {
                if (bytes_received == 0) {
                    log_message("Cliente " + std::to_string(client->getId()) + " desconectado");
                } else {
                    log_message("Erro ao receber dados: " + std::string(strerror(errno)));
                }
                break;
            }
            
            // Garante terminação nula
            buffer[bytes_received] = '\0';
            
            try {
                // Analisa mensagem recebida
                Message msg = Message::deserialize(buffer);
                
                // Registra a mensagem recebida
                log_message("Mensagem do cliente " + std::to_string(client->getId()) + ": " + msg.getContent());
                
                // Adiciona log de cada mensagem recebida com timestamp
                log_message("Recebida mensagem de " + client->getUsername() + 
                            " (ID: " + std::to_string(client->getId()) + 
                            "): " + msg.getContent().substr(0, 50) + 
                            (msg.getContent().length() > 50 ? "..." : ""));
                
                // Transmite mensagem para todos os outros clientes
                broadcastMessage(msg, client->getId());
            } catch (const std::exception& e) {
                log_message("Erro ao processar mensagem: " + std::string(e.what()));
                
                // Registra tentativas de envio malformadas
                log_message("Cliente " + std::to_string(client->getId()) + 
                           " enviou formato de mensagem inválido: " + std::string(e.what()) + 
                           " | Dados brutos: " + std::string(buffer).substr(0, 100));
            }
        }
    }
    
    // Envia mensagem de desconexão se o servidor ainda estiver em execução
    if (running) {
        Message disconnect_msg(0, "Servidor", "Usuário " + client->getUsername() + " saiu do chat");
        broadcastMessage(disconnect_msg, client->getId());
    }
    
    // Cliente desconectado, remove da lista
    clients.remove(client->getId());
    log_message("Thread do manipulador do cliente " + std::to_string(client->getId()) + " encerrada");
}

// Na função broadcastMessage, adicione mais detalhes ao log
void ChatServer::broadcastMessage(const Message& msg, int sender_id) {
    // Serializa a mensagem para transmissão
    std::string serialized_msg = msg.serialize();
    
    // Obtém todos os clientes conectados de forma thread-safe
    auto all_clients = clients.getAll();
    int successful_sends = 0;
    
    // Envia para cada cliente exceto o remetente
    for (const auto& client : all_clients) {
        // Pula o remetente
        if (client->getId() == sender_id) {
            continue;
        }
        
        // Envia a mensagem
        if (send(client->getSocket(), serialized_msg.c_str(), serialized_msg.length(), 0) < 0) {
            log_message("Falha ao enviar mensagem para o cliente " + std::to_string(client->getId()));
        } else {
            successful_sends++;
        }
    }
    
    // Log de informações de sucesso com mais detalhes
    if (sender_id > 0) {
        // Encontra o nome do remetente para o log
        std::string sender_name = "Desconhecido";
        auto sender = clients.get(sender_id);
        if (sender) {
            sender_name = sender->getUsername();
        }
        
        log_message("Mensagem de " + sender_name + " (ID: " + std::to_string(sender_id) + ") transmitida para " + 
                    std::to_string(successful_sends) + " clientes");
    }
}

// Adicione esta função ao arquivo:
void ChatServer::logStats() {
    while (running) {
        for (int i = 0; i < 300 && running; ++i) {  
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        
        if (!running) break;
        
        auto all_clients = clients.getAll();
        log_message("---- ESTATÍSTICAS DO SERVIDOR ----");
        log_message("Clientes conectados: " + std::to_string(all_clients.size()));
        
        std::string client_list = "IDs de clientes ativos: ";
        for (const auto& client : all_clients) {
            client_list += std::to_string(client->getId()) + " (" + client->getUsername() + "), ";
        }
        
        if (!all_clients.empty()) {
            client_list = client_list.substr(0, client_list.length() - 2);  // Remove a última vírgula
        }
        
        log_message(client_list);
        log_message("--------------------------------");
    }
}