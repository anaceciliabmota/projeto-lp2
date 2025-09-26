#pragma once

#include <vector>      // Para armazenar a lista de threads de clientes
#include <thread>      // Para criar threads para cada cliente
#include <memory>      // Para shared_ptr
#include <string>      // Para manipulação de strings
#include <atomic>      // Para flag de controle thread-safe (running)
#include <sys/socket.h> // API de sockets
#include <netinet/in.h> // Estruturas de endereço de rede
#include "thread_safe_client_list.h" // Lista thread-safe de clientes

// Forward declarations para evitar inclusão circular
class Client;
class Message;
class ThreadSafeClientList;

// Classe principal do servidor que gerencia conexões e mensagens
class ChatServer {
private:
    int server_socket;                  // Socket de escuta do servidor
    ThreadSafeClientList clients;       // Lista thread-safe de clientes conectados
    std::vector<std::thread> client_threads; // Threads para lidar com cada cliente
    std::atomic<bool> running;          // Flag thread-safe para controlar o loop principal
    
public:
    ChatServer(int port);               // Construtor que configura o socket na porta especificada
    ~ChatServer();                      // Destrutor que libera recursos e encerra threads
    
    void start();                       // Inicia o servidor (loop de aceitação)
    void stop();                        // Para o servidor de forma segura
    void broadcastMessage(const Message& msg, int sender_id = -1); // Envia msg para todos os clientes exceto o remetente
    
private:
    void acceptClients();               // Loop que aceita novas conexões de clientes
    void handleClient(std::shared_ptr<Client> client); // Gerencia comunicação com um cliente específico
};