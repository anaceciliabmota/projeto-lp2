#include "../include/thread_safe_client_list.h"

// Add a client to the list in a thread-safe way
void ThreadSafeClientList::add(std::shared_ptr<Client> client) {
    std::lock_guard<std::mutex> lock(mutex);
    clients.push_back(client);
}

// Remove a client from the list in a thread-safe way
void ThreadSafeClientList::remove(int client_id) {
    std::lock_guard<std::mutex> lock(mutex);
    clients.erase(
        std::remove_if(clients.begin(), clients.end(),
            [client_id](const std::shared_ptr<Client>& c) {
                return c->getId() == client_id;
            }),
        clients.end()
    );
}

// Get a client by ID in a thread-safe way
std::shared_ptr<Client> ThreadSafeClientList::get(int client_id) {
    std::lock_guard<std::mutex> lock(mutex);
    for (const auto& client : clients) {
        if (client->getId() == client_id) {
            return client;
        }
    }
    return nullptr;
}

// Get all clients in a thread-safe way
std::vector<std::shared_ptr<Client>> ThreadSafeClientList::getAll() const {
    std::lock_guard<std::mutex> lock(mutex);
    return clients;
}

// Get the number of connected clients in a thread-safe way
size_t ThreadSafeClientList::size() const {
    std::lock_guard<std::mutex> lock(mutex);
    return clients.size();
}