#include "../include/message.h"
#include <sstream>

// Construtor padrão: inicializa com ID de remetente -1 e timestamp atual
Message::Message() : sender_id(-1), timestamp(std::time(nullptr)) {
}

// Construtor com parâmetros: configura ID do remetente, nome, conteúdo e timestamp atual
Message::Message(int sender, const std::string& name, const std::string& text)
    : sender_id(sender), sender_name(name), content(text), timestamp(std::time(nullptr)) {
}

// Retorna o ID do remetente da mensagem
int Message::getSenderId() const {
    return sender_id;
}

// Retorna o nome do remetente da mensagem
std::string Message::getSenderName() const {
    return sender_name;
}

// Retorna o conteúdo da mensagem
std::string Message::getContent() const {
    return content;
}

// Retorna o timestamp de quando a mensagem foi criada
std::time_t Message::getTimestamp() const {
    return timestamp;
}

// Serializa a mensagem em formato de string para transmissão pela rede
// Formato: "sender_id:sender_name:timestamp:content"
std::string Message::serialize() const {
    std::stringstream ss;
    ss << sender_id << ":" << sender_name << ":" << timestamp << ":" << content;
    return ss.str();
}

// Desserializa uma string para criar um objeto Message
// Espera o formato: "sender_id:sender_name:timestamp:content"
Message Message::deserialize(const std::string& data) {
    std::stringstream ss(data);
    std::string id_str, name, timestamp_str, content;
    
    // Separa os campos usando ':' como delimitador
    std::getline(ss, id_str, ':');
    std::getline(ss, name, ':');
    std::getline(ss, timestamp_str, ':');
    std::getline(ss, content);
    
    // Cria e configura o objeto Message
    Message msg;
    msg.sender_id = std::stoi(id_str);        // Converte ID para inteiro
    msg.sender_name = name;                   // Define o nome do remetente
    msg.content = content;                    // Define o conteúdo da mensagem
    msg.timestamp = std::stoll(timestamp_str); // Converte timestamp para long long
    
    return msg;
}