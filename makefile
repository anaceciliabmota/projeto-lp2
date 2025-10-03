# Variaveis para o compilador e flags
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17
LDFLAGS = -pthread

# Diretorios
SRC_DIR = src
INCLUDE_DIR = include
LIBTSLOG_DIR = src/libtslog
BUILD_DIR = build

# Flags de inclusão
INCLUDES = -I$(INCLUDE_DIR) -I$(LIBTSLOG_DIR)

# Nomes dos executáveis
SERVER_TARGET = chat_server
CLIENT_TARGET = chat_client

# Arquivos fonte principais
SERVER_MAIN = $(SRC_DIR)/main_server.cpp
CLIENT_MAIN = $(SRC_DIR)/main_client.cpp

# Arquivos fonte comuns e específicos
LIBTSLOG_SRC = $(SRC_DIR)/libtslog/libtslog.cpp
MESSAGE_SRC = $(SRC_DIR)/message.cpp
CLIENT_SRC = $(SRC_DIR)/client.cpp
SERVER_SRC = $(SRC_DIR)/server.cpp
THREAD_SAFE_LIST_SRC = $(SRC_DIR)/thread_safe_client_list.cpp

# Lista de arquivos para o servidor
SERVER_SRCS = $(SERVER_MAIN) $(SERVER_SRC) $(CLIENT_SRC) $(MESSAGE_SRC) $(THREAD_SAFE_LIST_SRC) $(LIBTSLOG_SRC)
SERVER_OBJS = $(addprefix $(BUILD_DIR)/server/, $(notdir $(SERVER_SRCS:.cpp=.o)))

# Lista de arquivos para o cliente
CLIENT_SRCS = $(CLIENT_MAIN) $(CLIENT_SRC) $(MESSAGE_SRC)
CLIENT_OBJS = $(addprefix $(BUILD_DIR)/client/, $(notdir $(CLIENT_SRCS:.cpp=.o)))

# Variavel para o comando mkdir
MKDIR_P = mkdir -p

# Regra principal para construir ambos os executáveis
all: $(SERVER_TARGET) $(CLIENT_TARGET)

# Regra para construir o servidor
$(SERVER_TARGET): $(SERVER_OBJS)
	@echo "Linkando $(SERVER_TARGET)..."
	$(CXX) $(SERVER_OBJS) -o $@ $(LDFLAGS)

# Regra para construir o cliente
$(CLIENT_TARGET): $(CLIENT_OBJS)
	@echo "Linkando $(CLIENT_TARGET)..."
	$(CXX) $(CLIENT_OBJS) -o $@ $(LDFLAGS)

# Regras de compilação para os arquivos do servidor
$(BUILD_DIR)/server/%.o: $(SRC_DIR)/%.cpp
	@echo "Compilando $<..."
	@$(MKDIR_P) $(BUILD_DIR)/server
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/server/%.o: $(SRC_DIR)/libtslog/%.cpp
	@echo "Compilando $<..."
	@$(MKDIR_P) $(BUILD_DIR)/server
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Regras de compilação para os arquivos do cliente
$(BUILD_DIR)/client/%.o: $(SRC_DIR)/%.cpp
	@echo "Compilando $<..."
	@$(MKDIR_P) $(BUILD_DIR)/client
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Limpeza: remove os arquivos de build e executáveis
clean:
	@echo "Removendo arquivos de build e executáveis..."
	@rm -rf $(BUILD_DIR) $(SERVER_TARGET) $(CLIENT_TARGET)

# Regra para executar o servidor
run-server: $(SERVER_TARGET)
	./$(SERVER_TARGET)

# Regra para executar o cliente
run-client: $(CLIENT_TARGET)
	./$(CLIENT_TARGET)

.PHONY: all clean run-server run-client