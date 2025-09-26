#include "message.h"
#include <sstream>

Message::Message() : sender_id(-1), timestamp(std::time(nullptr)) {
}

Message::Message(int sender, const std::string& name, const std::string& text)
    : sender_id(sender), sender_name(name), content(text), timestamp(std::time(nullptr)) {
}

int Message::getSenderId() const {
    return sender_id;
}

std::string Message::getSenderName() const {
    return sender_name;
}

std::string Message::getContent() const {
    return content;
}

std::time_t Message::getTimestamp() const {
    return timestamp;
}

std::string Message::serialize() const {
    std::stringstream ss;
    ss << sender_id << ":" << sender_name << ":" << timestamp << ":" << content;
    return ss.str();
}

Message Message::deserialize(const std::string& data) {
    std::stringstream ss(data);
    std::string id_str, name, timestamp_str, content;
    
    std::getline(ss, id_str, ':');
    std::getline(ss, name, ':');
    std::getline(ss, timestamp_str, ':');
    std::getline(ss, content);
    
    Message msg;
    msg.sender_id = std::stoi(id_str);
    msg.sender_name = name;
    msg.content = content;
    msg.timestamp = std::stoll(timestamp_str);
    
    return msg;
}