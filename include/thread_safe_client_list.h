#pragma once

#include <vector>
#include <memory>
#include <mutex>
#include "client.h"

class ThreadSafeClientList {
private:
    std::vector<std::shared_ptr<Client>> clients;
    mutable std::mutex mutex;
    
public:
    void add(std::shared_ptr<Client> client);
    void remove(int client_id);
    std::shared_ptr<Client> get(int client_id);
    std::vector<std::shared_ptr<Client>> getAll() const;
    size_t size() const;
};