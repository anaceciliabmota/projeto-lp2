#ifndef LIBTSLOG_H
#define LIBTSLOG_H

#include <string>

using namespace std;

// Inicializa o logger, abrindo o arquivo de log para escrita.
void init_logger(const std::string& filename);

// Escreve uma mensagem de log no arquivo de forma thread-safe.
// Usa um mutex para garantir que apenas uma thread escreva por vez.
void log_message(const std::string& message);

// Fecha o arquivo de log, liberando os recursos.
void close_logger();

#endif // LIBTSLOG_H