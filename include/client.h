#pragma once

#include <string>      // Para armazenar nome de usuário
#include <memory>      // Para smart pointers
#include <sys/socket.h> // API de sockets

// Forward declaration
class Message;

// Representa um cliente conectado ao servidor
class Client {
private:
    int socket;            // Socket de conexão com este cliente
    int id;                // ID único para identificar o cliente
    std::string username;  // Nome de usuário do cliente
    
public:
    Client(int socket_fd, int client_id); // Construtor que configura socket e ID
    ~Client();                            // Destrutor que fecha o socket
    
    bool sendMessage(const Message& msg);   // Envia uma mensagem para este cliente
    bool receiveMessage(Message& msg);      // Recebe uma mensagem deste cliente
    int getId() const;                      // Retorna o ID único do cliente
    std::string getUsername() const;        // Retorna o nome de usuário
    void setUsername(const std::string& name); // Define o nome de usuário
    int getSocket() const;                // Retorna o descritor do socket
};