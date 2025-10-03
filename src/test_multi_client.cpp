#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <chrono>
#include <random>
#include <atomic>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cstring>

// Constantes de configuração
const std::string SERVER_IP = "127.0.0.1";
const int SERVER_PORT = 8080;
const int NUM_CLIENTS = 10;
const int MESSAGES_PER_CLIENT = 5;
const int MAX_DELAY_MS = 1000;

// Mutex para sincronizar a saída do console
std::mutex cout_mutex;

// Contador atômico para acompanhar os clientes conectados
std::atomic<int> connected_clients(0);

// Função para gerar uma mensagem aleatória
std::string generate_random_message() {
    static const std::string messages[] = {
        "Olá, como estão todos?",
        "Este é um teste de múltiplos clientes",
        "O servidor está funcionando bem?",
        "Testando comunicação concorrente",
        "Esta é uma mensagem de teste",
        "Quero ver se todos recebem essa mensagem",
        "Testando o broadcast de mensagens",
        "Alguém está recebendo minhas mensagens?",
        "Teste de carga do servidor",
        "Última mensagem de teste"
    };
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 9);
    
    return messages[dis(gen)];
}

// Função que simula um cliente
void simulate_client(int client_id) {
    try {
        // Criar identificação do cliente
        std::string username = "TestClient_" + std::to_string(client_id);
        
        // Log de início
        {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "[Cliente " << client_id << "] Iniciando..." << std::endl;
        }
        
        // Criar socket
        int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (sock_fd < 0) {
            throw std::runtime_error("Falha ao criar socket");
        }
        
        // Configurar endereço do servidor
        struct sockaddr_in server_addr;
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(SERVER_PORT);
        
        if (inet_pton(AF_INET, SERVER_IP.c_str(), &server_addr.sin_addr) <= 0) {
            close(sock_fd);
            throw std::runtime_error("Endereço inválido");
        }
        
        // Conectar ao servidor
        if (connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            close(sock_fd);
            throw std::runtime_error("Falha na conexão");
        }
        
        {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "[Cliente " << client_id << "] Conectado ao servidor" << std::endl;
        }
        
        // Incrementar contador de clientes conectados
        connected_clients++;
        
        // Enviar nome de usuário
        std::string username_msg = "0:NewUser:" + std::to_string(time(nullptr)) + ":" + username;
        send(sock_fd, username_msg.c_str(), username_msg.length(), 0);
        
        // Esperar um pouco para receber a mensagem de boas-vindas
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        // Thread para receber mensagens
        std::thread receiver([sock_fd, client_id]() {
            const int BUFFER_SIZE = 1024;
            char buffer[BUFFER_SIZE];
            
            while (true) {
                memset(buffer, 0, BUFFER_SIZE);
                ssize_t bytes_received = recv(sock_fd, buffer, BUFFER_SIZE - 1, 0);
                
                if (bytes_received <= 0) {
                    break;
                }
                
                buffer[bytes_received] = '\0';
                
                std::lock_guard<std::mutex> lock(cout_mutex);
                std::cout << "[Cliente " << client_id << " Recebeu] " << buffer << std::endl;
            }
        });
        
        // Desacoplar a thread para permitir que ela continue executando
        receiver.detach();
        
        // Gerador de números aleatórios para delays
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> delay_dist(200, MAX_DELAY_MS);
        
        // Enviar mensagens
        for (int i = 0; i < MESSAGES_PER_CLIENT; ++i) {
            // Delay aleatório entre mensagens para simular interação humana
            std::this_thread::sleep_for(std::chrono::milliseconds(delay_dist(gen)));
            
            // Gerar e enviar mensagem
            std::string content = generate_random_message();
            std::string message = std::to_string(client_id) + ":" + username + ":" + 
                                  std::to_string(time(nullptr)) + ":" + content;
            
            send(sock_fd, message.c_str(), message.length(), 0);
            
            {
                std::lock_guard<std::mutex> lock(cout_mutex);
                std::cout << "[Cliente " << client_id << " Enviou] " << content << std::endl;
            }
        }
        
        // Esperar um pouco para que as mensagens sejam processadas
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        // Desconectar
        {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "[Cliente " << client_id << "] Desconectando..." << std::endl;
        }
        
        // Decrementar contador de clientes conectados
        connected_clients--;
        
        // Fechar socket
        close(sock_fd);
        
    } catch (const std::exception& e) {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cerr << "[Cliente " << client_id << "] Erro: " << e.what() << std::endl;
    }
}

int main() {
    std::cout << "Iniciando simulação de " << NUM_CLIENTS << " clientes..." << std::endl;
    
    // Vetor para armazenar as threads dos clientes
    std::vector<std::thread> client_threads;
    
    // Criar threads para simular clientes
    for (int i = 1; i <= NUM_CLIENTS; ++i) {
        // Pequeno delay entre criação de clientes para evitar sobrecarga
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        
        // Criar thread para simular o cliente
        client_threads.emplace_back(simulate_client, i);
    }
    
    // Aguardar todas as threads terminarem
    for (auto& t : client_threads) {
        if (t.joinable()) {
            t.join();
        }
    }
    
    std::cout << "Simulação concluída. Todos os clientes desconectados." << std::endl;
    
    return 0;
}