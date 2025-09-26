# Variaveis para o compilador e flags
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17
LDFLAGS = -pthread

# Diretorios
SRC_DIR = src
INCLUDE_DIR = include
LIBTSLOG_DIR = src/libtslog
BUILD_DIR = build

# Flags de inclusão (agora apontando para a pasta include)
INCLUDES = -I$(INCLUDE_DIR) -I$(LIBTSLOG_DIR)

# Nomes dos arquivos
TARGET = log_test # O nome do executavel no diretorio raiz
MAIN_SRC = $(SRC_DIR)/main.cpp
LIB_SRC = $(SRC_DIR)/libtslog/libtslog.cpp
MSG_SRC = $(SRC_DIR)/message.cpp  # Novo arquivo message.cpp

# Cria uma lista de todos os arquivos .cpp
SRCS = $(MAIN_SRC) $(LIB_SRC) $(MSG_SRC)
OBJS = $(addprefix $(BUILD_DIR)/, $(notdir $(SRCS:.cpp=.o)))

# Variavel para o comando mkdir
MKDIR_P = mkdir -p

# Regra principal para construir o projeto
all: $(TARGET)

# Linka os objetos para criar o executavel no diretorio raiz
$(TARGET): $(OBJS)
	@echo "Linkando..."
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

# Regra de compilacao para main.cpp
$(BUILD_DIR)/main.o: $(MAIN_SRC)
	@echo "Compilando $<..."
	@$(MKDIR_P) $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Regra de compilacao para libtslog.cpp
$(BUILD_DIR)/libtslog.o: $(LIB_SRC)
	@echo "Compilando $<..."
	@$(MKDIR_P) $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Regra de compilacao para message.cpp
$(BUILD_DIR)/message.o: $(MSG_SRC)
	@echo "Compilando $<..."
	@$(MKDIR_P) $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Limpeza: remove os arquivos de build
clean:
	@echo "Removendo arquivos de build e executavel..."
	@rm -rf $(BUILD_DIR) $(TARGET)

# Regra para executar o programa
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
