#include "../include/server.h"
#include "../include/client.h"
#include "../include/message.h"
#include "../src/libtslog/libtslog.h"

#include <iostream>
#include <stdexcept>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h> // Add this include for fcntl

static int next_client_id = 1;  // Add this at the top of the file if not already present

// Constructor: Initialize server socket and configure it for listening
ChatServer::ChatServer(int port) : running(true) {
    // Create TCP socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        throw std::runtime_error("Failed to create server socket");
    }
    
    // Set socket option to allow port reuse (useful for server restarts)
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        close(server_socket);
        throw std::runtime_error("Failed to set socket options");
    }
    
    // Configure server address
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;         // IPv4
    server_addr.sin_addr.s_addr = INADDR_ANY; // Accept connections on any network interface
    server_addr.sin_port = htons(port);       // Convert port to network byte order
    
    // Bind socket to the specified address and port
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        close(server_socket);
        throw std::runtime_error("Failed to bind socket to port " + std::to_string(port));
    }
    
    // Start listening for incoming connections (queue size = 10)
    if (listen(server_socket, 10) < 0) {
        close(server_socket);
        throw std::runtime_error("Failed to listen on socket");
    }
    
    log_message("Server initialized and listening on port " + std::to_string(port));
}

// Destructor: Clean up resources
ChatServer::~ChatServer() {
    stop();
}

// Start the server by creating a thread to accept clients
void ChatServer::start() {
    log_message("Starting server...");
    client_threads.emplace_back(&ChatServer::acceptClients, this);
    client_threads.emplace_back(&ChatServer::logStats, this);  // Adicione esta linha
    log_message("Server started successfully. Awaiting connections...");
}

// Stop the server and clean up
void ChatServer::stop() {
    log_message("Stopping server...");
    
    // Set running flag to false to stop loops
    running = false;
    
    // Send a server shutdown message to all clients
    Message shutdown_msg(0, "Server", "Server is shutting down. Goodbye!");
    broadcastMessage(shutdown_msg);
    
    // Give client threads a moment to process the shutdown message
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Close the server socket to unblock the accept() call
    close(server_socket);
    
    // Wait for main acceptClients thread to complete
    for (auto& thread : client_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    
    // Close all client connections
    auto all_clients = clients.getAll();
    for (const auto& client : all_clients) {
        close(client->getSocket());
    }
    
    log_message("Server stopped");
}

// Accept new client connections in a loop
void ChatServer::acceptClients() {
    struct sockaddr_in client_addr;
    socklen_t client_addr_size = sizeof(client_addr);
    
    // Set socket to non-blocking mode
    int flags = fcntl(server_socket, F_GETFL, 0);
    fcntl(server_socket, F_SETFL, flags | O_NONBLOCK);
    
    while (running) {
        // Use select to wait for connections with a timeout
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(server_socket, &read_fds);
        
        // Set a 1-second timeout
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
        int select_result = select(server_socket + 1, &read_fds, NULL, NULL, &timeout);
        
        // Check if we should continue running
        if (!running) {
            break;
        }
        
        // Handle select errors
        if (select_result < 0) {
            if (errno == EINTR) {
                // Interrupted by signal, just retry
                continue;
            }
            log_message("Select error: " + std::string(strerror(errno)));
            continue;
        }
        
        // Check if we have a new connection
        if (select_result > 0 && FD_ISSET(server_socket, &read_fds)) {
            int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_size);
            
            if (client_socket < 0) {
                if (errno != EAGAIN && errno != EWOULDBLOCK) {
                    log_message("Error accepting client connection: " + std::string(strerror(errno)));
                }
                continue;
            } else {
                // Get client IP address as string
                char client_ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
                
                // Log de segurança mais detalhado
                log_message("New connection accepted | Socket: " + std::to_string(client_socket) + 
                            " | IP: " + std::string(client_ip) + 
                            " | Port: " + std::to_string(ntohs(client_addr.sin_port)));
            }
            
            // Get client IP address as string
            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
            log_message("New connection from " + std::string(client_ip));
            
            // Create a new Client object
            auto client = std::make_shared<Client>(client_socket, next_client_id++);
            
            // Add client to thread-safe list
            clients.add(client);
            
            // Create a detached thread to handle this client
            std::thread client_handler(&ChatServer::handleClient, this, client);
            client_handler.detach();
        }
    }
    
    log_message("Accept loop terminated");
}

// Handle communication with a specific client
void ChatServer::handleClient(std::shared_ptr<Client> client) {
    const int BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;
    
    // Send welcome message to client
    std::string welcome_msg = "Welcome to the chat server! Your ID is: " + std::to_string(client->getId());
    send(client->getSocket(), welcome_msg.c_str(), welcome_msg.length(), 0);
    
    // Wait for the first message which should be the username
    memset(buffer, 0, BUFFER_SIZE);
    bytes_received = recv(client->getSocket(), buffer, BUFFER_SIZE - 1, 0);
    
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        
        try {
            // Try to parse as a Message
            Message msg = Message::deserialize(buffer);
            
            // Set the client's username to the content of this first message
            client->setUsername(msg.getContent());
            log_message("Client " + std::to_string(client->getId()) + " set username to: " + client->getUsername());
            
            // Now send the join notification with the actual username
            Message join_msg(0, "Server", "User " + client->getUsername() + " has joined the chat");
            broadcastMessage(join_msg);
            
        } catch (const std::exception& e) {
            // If parsing failed, use the default username
            log_message("Error setting username: " + std::string(e.what()));
            
            // Send join notification with default username
            Message join_msg(0, "Server", "User " + client->getUsername() + " has joined the chat");
            broadcastMessage(join_msg);
        }
    } else {
        // If we couldn't receive a message, use default username
        Message join_msg(0, "Server", "User " + client->getUsername() + " has joined the chat");
        broadcastMessage(join_msg);
    }
    
    // Process client messages until disconnection or server shutdown
    while (running) {
        // Clear buffer before receiving
        memset(buffer, 0, BUFFER_SIZE);
        
        // Use select with timeout to make the receive operation interruptible
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(client->getSocket(), &read_fds);
        
        struct timeval timeout;
        timeout.tv_sec = 1;  // 1 second timeout
        timeout.tv_usec = 0;
        
        int select_result = select(client->getSocket() + 1, &read_fds, NULL, NULL, &timeout);
        
        // Check if server is still running
        if (!running) {
            break;
        }
        
        // Handle select errors or timeout
        if (select_result <= 0) {
            if (select_result < 0 && errno != EINTR) {
                log_message("Select error in client handler: " + std::string(strerror(errno)));
            }
            // On timeout or interrupt, just continue the loop
            continue;
        }
        
        // Receive data if available
        if (FD_ISSET(client->getSocket(), &read_fds)) {
            bytes_received = recv(client->getSocket(), buffer, BUFFER_SIZE - 1, 0);
            
            // Check for disconnection or error
            if (bytes_received <= 0) {
                if (bytes_received == 0) {
                    log_message("Client " + std::to_string(client->getId()) + " disconnected");
                } else {
                    log_message("Error receiving data: " + std::string(strerror(errno)));
                }
                break;
            }
            
            // Ensure null termination
            buffer[bytes_received] = '\0';
            
            try {
                // Parse received message
                Message msg = Message::deserialize(buffer);
                
                // Log the received message
                log_message("Message from client " + std::to_string(client->getId()) + ": " + msg.getContent());
                
                // Adicione log de cada mensagem recebida com timestamp
                log_message("Received message from " + client->getUsername() + 
                            " (ID: " + std::to_string(client->getId()) + 
                            "): " + msg.getContent().substr(0, 50) + 
                            (msg.getContent().length() > 50 ? "..." : ""));
                
                // Broadcast message to all other clients
                broadcastMessage(msg, client->getId());
            } catch (const std::exception& e) {
                log_message("Error processing message: " + std::string(e.what()));
                
                // Registre tentativas de envio malformadas
                log_message("Client " + std::to_string(client->getId()) + 
                           " sent invalid message format: " + std::string(e.what()) + 
                           " | Raw data: " + std::string(buffer).substr(0, 100));
            }
        }
    }
    
    // Send disconnect message to client if server is still running
    if (running) {
        Message disconnect_msg(0, "Server", "User " + client->getUsername() + " has left the chat");
        broadcastMessage(disconnect_msg, client->getId());
    }
    
    // Client disconnected, remove from list
    clients.remove(client->getId());
    log_message("Client " + std::to_string(client->getId()) + " handler thread terminated");
}

// Na função broadcastMessage, adicione mais detalhes ao log
void ChatServer::broadcastMessage(const Message& msg, int sender_id) {
    // Serialize the message for transmission
    std::string serialized_msg = msg.serialize();
    
    // Get all connected clients in a thread-safe manner
    auto all_clients = clients.getAll();
    int successful_sends = 0;
    
    // Send to each client except the sender
    for (const auto& client : all_clients) {
        // Skip the sender
        if (client->getId() == sender_id) {
            continue;
        }
        
        // Send the message
        if (send(client->getSocket(), serialized_msg.c_str(), serialized_msg.length(), 0) < 0) {
            log_message("Failed to send message to client " + std::to_string(client->getId()));
        } else {
            successful_sends++;
        }
    }
    
    // Log success info com mais detalhes
    if (sender_id > 0) {
        // Encontre o nome do remetente para o log
        std::string sender_name = "Unknown";
        auto sender = clients.get(sender_id);
        if (sender) {
            sender_name = sender->getUsername();
        }
        
        log_message("Message from " + sender_name + " (ID: " + std::to_string(sender_id) + ") broadcast to " + 
                    std::to_string(successful_sends) + " clients");
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