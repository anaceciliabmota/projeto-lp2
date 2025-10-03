#include "../include/client.h"
#include "../include/message.h"
#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <stdexcept>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <fstream>
#include <chrono>
#include <ctime>

// Define server connection parameters
const std::string SERVER_IP = "127.0.0.1";
const int SERVER_PORT = 8080;

// Flag to control message receiving thread
std::atomic<bool> running(true);


void client_log(const std::string& message, bool print_to_console = false) {
    // Apenas imprimir no console quando explicitamente solicitado
    if (print_to_console) {
        std::cout << "[INFO] " << message << std::endl;
    }
}

// Function to handle receiving messages from the server
void receive_messages(int socket_fd, int& client_id) {
    const int BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE];
    
    while (running) {
        // Clear buffer before receiving
        memset(buffer, 0, BUFFER_SIZE);
        
        // Receive message from server
        ssize_t bytes_received = recv(socket_fd, buffer, BUFFER_SIZE - 1, 0);
        
        // Exemplo de como usar o novo client_log em casos de erro:
        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                std::cout << "Server disconnected." << std::endl;
                // client_log não cria mais arquivos
            } else {
                std::cerr << "Error receiving data: " << strerror(errno) << std::endl;
                // client_log não cria mais arquivos
            }
            running = false;
            break;
        }
        
        // Ensure null termination
        buffer[bytes_received] = '\0';
        
        try {
            // Verifique se temos múltiplas mensagens concatenadas
            std::string full_buffer(buffer);
            size_t welcome_pos = full_buffer.find("Welcome to the chat server!");
            size_t serialized_pos = full_buffer.find("0:Server:");
            
            // Se encontrarmos tanto a mensagem de boas-vindas quanto uma mensagem serializada
            if (welcome_pos != std::string::npos && serialized_pos != std::string::npos) {
                // Processe primeiro a mensagem de boas-vindas
                std::string welcome = full_buffer.substr(0, serialized_pos);
                std::cout << welcome << std::endl;
                client_log(welcome);
                
                // Extraia o ID do cliente da mensagem de boas-vindas
                size_t id_pos = welcome.find("Your ID is: ");
                if (id_pos != std::string::npos) {
                    std::string id_str = welcome.substr(id_pos + 12);
                    try {
                        // Extraia apenas os dígitos até o primeiro caractere não-numérico
                        std::string id_digits;
                        for (char c : id_str) {
                            if (std::isdigit(c)) {
                                id_digits += c;
                            } else {
                                break;
                            }
                        }
                        client_id = std::stoi(id_digits);
                        std::cout << "Extracted client ID: " << client_id << std::endl;
                        client_log("Extracted client ID: " + std::to_string(client_id));
                    } catch (...) {
                        std::cerr << "Could not extract client ID from: " << id_str << std::endl;
                        client_log("Could not extract client ID from: " + id_str);
                    }
                }
                
                // Processe a parte da mensagem serializada
                std::string serialized = full_buffer.substr(serialized_pos);
                Message msg = Message::deserialize(serialized);
                std::cout << "[" << msg.getSenderName() << "]: " << msg.getContent() << std::endl;
                client_log("[" + msg.getSenderName() + "]: " + msg.getContent());
            }
            // Caso contrário, continue com o processamento normal
            else {
                // Código existente para contar os dois pontos e decidir como tratar
                int colon_count = 0;
                for (char c : std::string(buffer)) {
                    if (c == ':') colon_count++;
                }
                
                if (colon_count >= 3) {
                    // This looks like a proper serialized Message
                    Message msg = Message::deserialize(buffer);
                    std::cout << "[" << msg.getSenderName() << "]: " << msg.getContent() << std::endl;
                    client_log("[" + msg.getSenderName() + "]: " + msg.getContent());
                } else {
                    // This is probably a plain text message from the server
                    std::cout << buffer << std::endl;
                    client_log(buffer);
                    
                    // If this contains the client ID info, extract it
                    std::string id_str = buffer;
                    size_t id_pos = id_str.find("Your ID is: ");
                    if (id_pos != std::string::npos) {
                        id_str = id_str.substr(id_pos + 12); // "Your ID is: " has 12 chars
                        try {
                            // Extraia apenas os dígitos até o primeiro caractere não-numérico
                            std::string id_digits;
                            for (char c : id_str) {
                                if (std::isdigit(c)) {
                                    id_digits += c;
                                } else {
                                    break;
                                }
                            }
                            client_id = std::stoi(id_digits);
                            std::cout << "Extracted client ID: " << client_id << std::endl;
                            client_log("Extracted client ID: " + std::to_string(client_id));
                        } catch (...) {
                            std::cerr << "Could not extract client ID from: " << id_str << std::endl;
                            client_log("Could not extract client ID from: " + id_str);
                        }
                    }
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Error processing message: " << e.what() << std::endl;
            client_log("Error processing message: " + std::string(e.what()));
            // Print the raw message for debugging
            std::cout << "Raw message: " << buffer << std::endl;
            client_log("Raw message: " + std::string(buffer));
        }
    }
}

int main() {
    try {
        // Abrir arquivo de log do cliente

        std::cout << "Connecting to chat server at " << SERVER_IP << ":" << SERVER_PORT << "..." << std::endl;
        client_log("Connecting to chat server at " + SERVER_IP + ":" + std::to_string(SERVER_PORT) + "...");
        
        // Create socket
        int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (socket_fd < 0) {
            throw std::runtime_error("Failed to create socket");
        }
        
        // Configure server address
        struct sockaddr_in server_addr;
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(SERVER_PORT);
        
        // Convert IP address from string to binary form
        if (inet_pton(AF_INET, SERVER_IP.c_str(), &server_addr.sin_addr) <= 0) {
            close(socket_fd);
            throw std::runtime_error("Invalid address or address not supported");
        }
        
        // Connect to the server
        if (connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            close(socket_fd);
            throw std::runtime_error("Connection failed");
        }
        
        std::cout << "Connected to the server. Type your messages or 'EXIT' to quit." << std::endl;
        client_log("Connected to the server.");
        
        // Ask for username
        std::string username;
        std::cout << "Enter your username: ";
        std::getline(std::cin, username);

        // Enviar o nome de usuário como a primeira mensagem
        // Usamos um ID temporário, já que o ID real será atribuído pelo servidor
        Message username_msg(0, "NewUser", username);
        std::string serialized = username_msg.serialize();
        send(socket_fd, serialized.c_str(), serialized.length(), 0);
        client_log("Sent username: " + username);

        // Continue com o código existente para criar a thread de recebimento...
        int client_id = 0;
        std::thread receive_thread(receive_messages, socket_fd, std::ref(client_id));
        
        // Main loop to send messages
        std::string input;
        
        while (running) {
            // Read a line of input from the user
            std::getline(std::cin, input);
            
            // Check if user wants to exit
            if (input == "EXIT") {
                running = false;
                break;
            }
            
            // Create and send message
            Message msg(client_id, username, input);
            std::string serialized = msg.serialize();
            
            if (send(socket_fd, serialized.c_str(), serialized.length(), 0) < 0) {
                std::cerr << "Failed to send message: " << strerror(errno) << std::endl;
                client_log("Failed to send message: " + std::string(strerror(errno)));
                running = false;
                break;
            }
            client_log("Sent message: " + input);
        }
        
        // Close socket to trigger receive thread to exit if blocked on recv()
        close(socket_fd);
        
        // Wait for receive thread to finish
        if (receive_thread.joinable()) {
            receive_thread.join();
        }
        
        std::cout << "Disconnected from server." << std::endl;
        client_log("Disconnected from server.");
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        client_log("Error: " + std::string(e.what()));
        return 1;
    }
    
    return 0;
}