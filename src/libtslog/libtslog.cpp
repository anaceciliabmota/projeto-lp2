#include "libtslog.h"
#include <fstream>
#include <iostream>
#include <pthread.h>

// Objeto de arquivo de log
ofstream log_file;
// Mutex para proteger o acesso ao arquivo de log
pthread_mutex_t log_mutex;

void init_logger(const std::string& filename) {
    // Inicializa o mutex
    if (pthread_mutex_init(&log_mutex, NULL) != 0) {
        cerr << "Erro: A inicializacao do mutex falhou." << endl;
        return;
    }
    // Abre o arquivo de log para escrita no final (append)
    log_file.open(filename, std::ios_base::app);
    if (!log_file.is_open()) {
        cerr << "Erro: Nao foi possivel abrir o arquivo de log." << endl;
    }
}

void log_message(const std::string& message) {
    // Adquire o bloqueio do mutex
    pthread_mutex_lock(&log_mutex);
    if (log_file.is_open()) {
        log_file << message << std::endl;
    }
    // Libera o bloqueio do mutex
    pthread_mutex_unlock(&log_mutex);
}

void close_logger() {
    // Fecha o arquivo de log
    if (log_file.is_open()) {
        log_file.close();
    }
    // Destroi o mutex
    pthread_mutex_destroy(&log_mutex);
}