#!/bin/bash

# Cores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}==== Script de Teste do Servidor de Chat ====${NC}"

# Compilar o script de teste
echo -e "${BLUE}Compilando o script de teste...${NC}"
g++ -o test_multi_client src/test_multi_client.cpp -std=c++17 -pthread

if [ $? -ne 0 ]; then
    echo -e "${RED}Falha na compilação do script de teste.${NC}"
    exit 1
fi

echo -e "${GREEN}Compilação concluída com sucesso.${NC}"

# Verificar se o servidor está em execução
echo -e "${BLUE}Verificando se o servidor está em execução...${NC}"
if pgrep -x "chat_server" > /dev/null; then
    echo -e "${GREEN}Servidor encontrado em execução.${NC}"
else
    echo -e "${RED}Servidor não está em execução. Iniciando...${NC}"
    ./chat_server &
    SERVER_PID=$!
    echo -e "${BLUE}Aguardando o servidor inicializar...${NC}"
    sleep 2
fi

# Executar o teste
echo -e "${BLUE}Iniciando teste com múltiplos clientes...${NC}"
./test_multi_client

# Finalizar
echo -e "${GREEN}Teste concluído.${NC}"

# Se iniciamos o servidor, vamos encerrá-lo
if [ ! -z "$SERVER_PID" ]; then
    echo -e "${BLUE}Encerrando o servidor iniciado pelo script...${NC}"
    kill $SERVER_PID
fi

echo -e "${BLUE}==== Fim do Teste ====${NC}"