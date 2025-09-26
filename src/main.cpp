#include "libtslog/libtslog.h"
#include "../include/message.h"
#include <iostream>
#include <pthread.h>
#include <string>
#include <ctime>
#include <unistd.h> 

using namespace std;

#define NUM_THREADS 10

// Estrutura para passar dados para a thread
struct ThreadData {
    int id;
    std::string username;
};

// Função que a thread irá executar
void* worker_thread(void* arg) {
    ThreadData* data = static_cast<ThreadData*>(arg);
    
    for (int i = 0; i < 5; ++i) {
        // Cria uma mensagem usando a classe Message
        Message msg(data->id, data->username, "Mensagem " + std::to_string(i));
        
        // Obtém string formatada da mensagem
        std::string formatted_message = "[" + msg.getSenderName() + "] " + msg.getContent();
        
        // Registra a mensagem formatada
        log_message(formatted_message);
        
        // // Demonstração de serialização/deserialização
        // std::string serialized = msg.serialize();
        // log_message("Serializada: " + serialized);
        
        // Pequena pausa para melhor visualização
        usleep(100000); // 100ms
    }
    
    delete data;
    pthread_exit(NULL);
}

int main() {
    // Nome do arquivo de log
    string log_filename = "chat_messages.log";

    // Inicializa o logger
    init_logger(log_filename);
    cout << "Logger inicializado. Arquivo: " << log_filename << std::endl;

    pthread_t threads[NUM_THREADS];
    
    // Nomes de usuário para simular clientes reais
    const std::string usernames[10] = {
        "Alice", "Bob", "Carlos", "Diana", "Eduardo",
        "Fernanda", "Gabriel", "Helena", "Igor", "Julia"
    };

    // Cria e inicia as threads
    for (int i = 0; i < NUM_THREADS; ++i) {
        ThreadData* data = new ThreadData{i, usernames[i]};
        int rc = pthread_create(&threads[i], NULL, worker_thread, (void*)data);
        if (rc) {
            cerr << "Erro: Não foi possível criar a thread " << i << std::endl;
            exit(-1);
        }
    }

    // Espera todas as threads terminarem
    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_join(threads[i], NULL);
    }

    // Fecha o logger
    close_logger();
    cout << "Logger fechado. Teste de chat concluído." << std::endl;

    return 0;
}