#include "../include/client.h"
#include "../include/message.h"
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <unistd.h>

// Contador estático para gerar IDs únicos de cliente
static int next_client_id = 1;

// Construtor: Inicializa o socket e o ID do cliente
Client::Client(int socket_fd, int client_id) : socket(socket_fd), id(client_id) {
    // Inicializa com nome de usuário padrão
    username = "Anonymous_" + std::to_string(id);
}

// Destrutor: Fecha o socket e libera recursos
Client::~Client() {
    // Fecha apenas se o socket for válido
    if (socket >= 0) {
        close(socket);
        socket = -1;
    }
}

// Envia uma mensagem para este cliente
bool Client::sendMessage(const Message& msg) {
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
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Exceção ao enviar mensagem: " << e.what() << std::endl;
        return false;
    }
}

// Recebe uma mensagem deste cliente
bool Client::receiveMessage(Message& msg) {
    const int BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE];
    
    // Limpa o buffer antes de receber
    memset(buffer, 0, BUFFER_SIZE);
    
    // Recebe dados do socket
    ssize_t bytes_received = recv(socket, buffer, BUFFER_SIZE - 1, 0);
    
    // Verifica erros ou desconexão
    if (bytes_received <= 0) {
        // Conexão fechada ou erro
        return false;
    }
    
    // Termina os dados recebidos com null
    buffer[bytes_received] = '\0';
    
    try {
        // Converte os dados recebidos para um objeto Message
        msg = Message::deserialize(buffer);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Erro ao desserializar mensagem: " << e.what() << std::endl;
        return false;
    }
}

// Retorna o ID único do cliente
int Client::getId() const {
    return id;
}

// Retorna o nome de usuário do cliente
std::string Client::getUsername() const {
    return username;
}

// Define o nome de usuário do cliente
void Client::setUsername(const std::string& name) {
    username = name;
}

// Retorna o descritor de arquivo do socket (necessário para comunicação com o servidor)
int Client::getSocket() const {
    return socket;
}