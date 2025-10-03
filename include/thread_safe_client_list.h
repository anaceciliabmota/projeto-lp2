#pragma once

#include <vector>      // Para armazenar a lista de clientes
#include <memory>      // Para shared_ptr
#include <mutex>       // Para exclusão mútua
#include "client.h"    // Definição da classe Client
#include <algorithm>

// Implementa um monitor para gerenciar a lista de clientes de forma thread-safe
class ThreadSafeClientList {
private:
    std::vector<std::shared_ptr<Client>> clients; // Armazena ponteiros para objetos Client
    mutable std::mutex mutex;                     // Mutex para proteger acesso concorrente
    
public:
    void add(std::shared_ptr<Client> client);           // Adiciona um cliente à lista de forma segura
    void remove(int client_id);                         // Remove um cliente pelo ID de forma segura
    std::shared_ptr<Client> get(int client_id);         // Obtém um cliente pelo ID
    std::vector<std::shared_ptr<Client>> getAll() const; // Obtém todos os clientes (para broadcast)
    size_t size() const;                                // Retorna o número de clientes conectados
};