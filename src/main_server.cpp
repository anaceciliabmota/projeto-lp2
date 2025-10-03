#include "../include/server.h"
#include "../src/libtslog/libtslog.h"
#include <iostream>
#include <stdexcept>
#include <string>
#include <csignal>

// Define a porta do servidor
const int PORT = 8080;

// Flag para desligamento limpo ao receber sinal
volatile std::sig_atomic_t running = true;

// Manipulador de sinais para desligamento gracioso
void signal_handler(int signal) {
    running = false;
    std::cout << "\nSinal de desligamento recebido. Parando servidor..." << std::endl;
}

int main() {
    // Inicializa tratamento de sinais
    std::signal(SIGINT, signal_handler);  // Trata Ctrl+C
    std::signal(SIGTERM, signal_handler); // Trata sinal de terminação
    
    // Inicializa o logger
    init_logger("server.log");
    
    try {
        std::cout << "Iniciando servidor de chat na porta " << PORT << "..." << std::endl;
        
        // Cria e inicia o servidor
        ChatServer server(PORT);
        server.start();
        
        std::cout << "Servidor em execução. Digite 'stop' para desligar." << std::endl;
        
        // Mantém o servidor rodando até que um sinal ou comando seja recebido
        std::string command;
        while (running) {
            // Verifica se o comando "stop" foi digitado no console
            if (std::getline(std::cin, command) && command == "stop") {
                break;
            }
        }
        
        // Para o servidor de forma graciosa
        std::cout << "Parando servidor..." << std::endl;
        server.stop();
        std::cout << "Servidor parado." << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Erro: " << e.what() << std::endl;
        close_logger();
        return 1;
    }
    
    close_logger();
    return 0;
}