#pragma once

#include <string>      // Para armazenamento de texto
#include <ctime>       // Para timestamp da mensagem

// Representa uma mensagem no sistema de chat
class Message {
private:
    int sender_id;             // ID do cliente que enviou a mensagem
    std::string sender_name;   // Nome do remetente
    std::string content;       // Conteúdo da mensagem
    std::time_t timestamp;     // Momento em que a mensagem foi criada
    
public:
    Message();                 // Construtor padrão
    Message(int sender, const std::string& name, const std::string& text); // Construtor com parâmetros
    
    int getSenderId() const;           // Obtém ID do remetente
    std::string getSenderName() const; // Obtém nome do remetente
    std::string getContent() const;    // Obtém conteúdo da mensagem
    std::time_t getTimestamp() const;  // Obtém timestamp
    
    // Métodos para conversão da mensagem para transmissão via socket
    std::string serialize() const;                // Converte mensagem em string para envio
    static Message deserialize(const std::string& data); // Converte string recebida em objeto Message
};