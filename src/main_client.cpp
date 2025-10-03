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

// Define parâmetros de conexão com o servidor
const std::string SERVER_IP = "127.0.0.1";
const int SERVER_PORT = 8080;

// Flag para controlar o loop de recepção
std::atomic<bool> running(true);


void client_log(const std::string& message, bool print_to_console = false) {
    // Apenas imprimir no console quando explicitamente solicitado
    if (print_to_console) {
        std::cout << "[INFO] " << message << std::endl;
    }
}

// Função para receber mensagens do servidor
// Também extrai o ID do cliente da mensagem de boas-vindas
void receive_messages(int socket_fd, int& client_id) {
    const int BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE];
    
    while (running) {
        // Limpa o buffer antes de receber
        memset(buffer, 0, BUFFER_SIZE);
        
        // Recebe dados do socket
        ssize_t bytes_received = recv(socket_fd, buffer, BUFFER_SIZE - 1, 0);
        
        // Exemplo de como usar o novo client_log em casos de erro:
        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                std::cout << "Servidor desconectado." << std::endl;
                // client_log não cria mais arquivos
            } else {
                std::cerr << "Erro ao receber dados: " << strerror(errno) << std::endl;
                // client_log não cria mais arquivos
            }
            running = false;
            break;
        }
        
        // Assegura terminação nula
        buffer[bytes_received] = '\0';
        
        try {
            // Verifica se temos múltiplas mensagens concatenadas
            std::string full_buffer(buffer);
            size_t welcome_pos = full_buffer.find("Welcome to the chat server!");
            size_t serialized_pos = full_buffer.find("0:Server:");
            
            // Se encontrarmos tanto a mensagem de boas-vindas quanto uma mensagem serializada
            if (welcome_pos != std::string::npos && serialized_pos != std::string::npos) {
                // Processa primeiro a mensagem de boas-vindas
                std::string welcome = full_buffer.substr(0, serialized_pos);
                std::cout << welcome << std::endl;
                client_log(welcome);
                
                // Extrai o ID do cliente da mensagem de boas-vindas
                size_t id_pos = welcome.find("Your ID is: ");
                if (id_pos != std::string::npos) {
                    std::string id_str = welcome.substr(id_pos + 12);
                    try {
                        // Extrai apenas os dígitos até o primeiro caractere não-numérico
                        std::string id_digits;
                        for (char c : id_str) {
                            if (std::isdigit(c)) {
                                id_digits += c;
                            } else {
                                break;
                            }
                        }
                        client_id = std::stoi(id_digits);
                        std::cout << "ID do cliente extraído: " << client_id << std::endl;
                        client_log("ID do cliente extraído: " + std::to_string(client_id));
                    } catch (...) {
                        std::cerr << "Não foi possível extrair o ID do cliente de: " << id_str << std::endl;
                        client_log("Não foi possível extrair o ID do cliente de: " + id_str);
                    }
                }
                
                // Processa a parte da mensagem serializada
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
                    // Provavelmente uma mensagem serializada
                    Message msg = Message::deserialize(buffer);
                    std::cout << "[" << msg.getSenderName() << "]: " << msg.getContent() << std::endl;
                    client_log("[" + msg.getSenderName() + "]: " + msg.getContent());
                } else {
                    // Provavelmente uma mensagem de sistema ou erro
                    std::cout << buffer << std::endl;
                    client_log(buffer);
                    
                    // Tenta extrair o ID do cliente se a mensagem for de boas-vindas
                    std::string id_str = buffer;
                    size_t id_pos = id_str.find("Your ID is: ");
                    if (id_pos != std::string::npos) {
                        id_str = id_str.substr(id_pos + 12); 
                        try {
                            // Extrai apenas os dígitos até o primeiro caractere não-numérico
                            std::string id_digits;
                            for (char c : id_str) {
                                if (std::isdigit(c)) {
                                    id_digits += c;
                                } else {
                                    break;
                                }
                            }
                            client_id = std::stoi(id_digits);
                            std::cout << "ID do cliente extraído: " << client_id << std::endl;
                            client_log("ID do cliente extraído: " + std::to_string(client_id));
                        } catch (...) {
                            std::cerr << "Não foi possível extrair o ID do cliente de: " << id_str << std::endl;
                            client_log("Não foi possível extrair o ID do cliente de: " + id_str);
                        }
                    }
                }
            }
        } catch (const std::exception& e) {
            //std::cerr << "Erro ao processar mensagem: " << e.what() << std::endl;
            //client_log("Erro ao processar mensagem: " + std::string(e.what()));
            // Imprime a mensagem bruta para depuração
            //std::cout << "Mensagem bruta: " << buffer << std::endl;
            //client_log("Mensagem bruta: " + std::string(buffer));
        }
    }
}

int main() {
    try {
        // Abrir arquivo de log do cliente

        std::cout << "Conectando ao servidor de chat em " << SERVER_IP << ":" << SERVER_PORT << "..." << std::endl;
        client_log("Conectando ao servidor de chat em " + SERVER_IP + ":" + std::to_string(SERVER_PORT) + "...");
        
        // Cria socket
        int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (socket_fd < 0) {
            throw std::runtime_error("Falha ao criar socket");
        }
        
        // Configura endereço do servidor
        struct sockaddr_in server_addr;
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(SERVER_PORT);
        
        // Converte o endereço IP de string para forma binária
        if (inet_pton(AF_INET, SERVER_IP.c_str(), &server_addr.sin_addr) <= 0) {
            close(socket_fd);
            throw std::runtime_error("Endereço inválido ou não suportado");
        }
        
        // Conecta ao servidor
        if (connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            close(socket_fd);
            throw std::runtime_error("Conexão falhou");
        }
        
        std::cout << "Conectado ao servidor. Digite suas mensagens ou 'EXIT' para sair." << std::endl;
        client_log("Conectado ao servidor.");
        
        // Solicita nome de usuário
        std::string username;
        std::cout << "Digite seu nome de usuário: ";
        std::getline(std::cin, username);

        // Envia o nome de usuário como a primeira mensagem
        // Usamos um ID temporário, já que o ID real será atribuído pelo servidor
        Message username_msg(0, "NewUser", username);
        std::string serialized = username_msg.serialize();
        send(socket_fd, serialized.c_str(), serialized.length(), 0);
        client_log("Nome de usuário enviado: " + username);

        // Continua com o código existente para criar a thread de recebimento
        int client_id = 0;
        std::thread receive_thread(receive_messages, socket_fd, std::ref(client_id));
        
        // Loop principal para enviar mensagens
        std::string input;
        
        while (running) {
            // Lê uma linha de entrada do usuário
            std::getline(std::cin, input);
            
            // Verifica se o usuário deseja sair
            if (input == "EXIT") {
                running = false;
                break;
            }
            
            // Cria e envia mensagem
            Message msg(client_id, username, input);
            std::string serialized = msg.serialize();
            
            if (send(socket_fd, serialized.c_str(), serialized.length(), 0) < 0) {
                std::cerr << "Falha ao enviar mensagem: " << strerror(errno) << std::endl;
                client_log("Falha ao enviar mensagem: " + std::string(strerror(errno)));
                running = false;
                break;
            }
            client_log("Mensagem enviada: " + input);
        }
        
        // Fecha o socket para fazer a thread de recepção sair se estiver bloqueada em recv()
        close(socket_fd);
        
        // Aguarda a thread de recepção terminar
        if (receive_thread.joinable()) {
            receive_thread.join();
        }
        
        std::cout << "Desconectado do servidor." << std::endl;
        client_log("Desconectado do servidor.");
        
    } catch (const std::exception& e) {
        std::cerr << "Erro: " << e.what() << std::endl;
        client_log("Erro: " + std::string(e.what()));
        return 1;
    }
    
    return 0;
}