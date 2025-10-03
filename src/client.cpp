#include "../include/client.h"
#include "../include/message.h"
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <unistd.h>

// Static counter for generating unique client IDs
static int next_client_id = 1;

// Constructor: Initialize socket and client ID
Client::Client(int socket_fd, int client_id) : socket(socket_fd), id(client_id) {
    // Initialize with default username
    username = "Anonymous_" + std::to_string(id);
}

// Destructor: Close socket and free resources
Client::~Client() {
    // Only close if socket is valid
    if (socket >= 0) {
        close(socket);
        socket = -1;
    }
}

// Send a message to this client
bool Client::sendMessage(const Message& msg) {
    try {
        // Serialize message to string format
        std::string serialized = msg.serialize();
        
        // Send the message through the socket
        ssize_t bytes_sent = send(socket, serialized.c_str(), serialized.length(), 0);
        
        // Check if send was successful
        if (bytes_sent < 0) {
            std::cerr << "Error sending message: " << strerror(errno) << std::endl;
            return false;
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Exception while sending message: " << e.what() << std::endl;
        return false;
    }
}

// Receive a message from this client
bool Client::receiveMessage(Message& msg) {
    const int BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE];
    
    // Clear buffer before receiving
    memset(buffer, 0, BUFFER_SIZE);
    
    // Receive data from socket
    ssize_t bytes_received = recv(socket, buffer, BUFFER_SIZE - 1, 0);
    
    // Check for errors or disconnection
    if (bytes_received <= 0) {
        // Connection closed or error
        return false;
    }
    
    // Null-terminate the received data
    buffer[bytes_received] = '\0';
    
    try {
        // Convert received data to Message object
        msg = Message::deserialize(buffer);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error deserializing message: " << e.what() << std::endl;
        return false;
    }
}

// Return client's unique ID
int Client::getId() const {
    return id;
}

// Return client's username
std::string Client::getUsername() const {
    return username;
}

// Set client's username
void Client::setUsername(const std::string& name) {
    username = name;
}

// Return socket file descriptor (needed for server communication)
int Client::getSocket() const {
    return socket;
}