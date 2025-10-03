#include "../include/server.h"
#include "../src/libtslog/libtslog.h"
#include <iostream>
#include <stdexcept>
#include <string>
#include <csignal>

// Define server port
const int PORT = 8080;

// Flag for clean shutdown on signal
volatile std::sig_atomic_t running = true;

// Signal handler for graceful shutdown
void signal_handler(int signal) {
    running = false;
    std::cout << "\nShutdown signal received. Stopping server..." << std::endl;
}

int main() {
    // Initialize signal handling
    std::signal(SIGINT, signal_handler);  // Handle Ctrl+C
    std::signal(SIGTERM, signal_handler); // Handle termination signal
    
    // Initialize logger
    init_logger("server.log");
    
    try {
        std::cout << "Starting chat server on port " << PORT << "..." << std::endl;
        
        // Create and start the server
        ChatServer server(PORT);
        server.start();
        
        std::cout << "Server is running. Enter 'stop' to shut down." << std::endl;
        
        // Keep the server running until signal or command is received
        std::string command;
        while (running) {
            // Check for "stop" command from console
            if (std::getline(std::cin, command) && command == "stop") {
                break;
            }
        }
        
        // Stop the server gracefully
        std::cout << "Stopping server..." << std::endl;
        server.stop();
        std::cout << "Server stopped." << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        close_logger();
        return 1;
    }
    
    close_logger();
    return 0;
}