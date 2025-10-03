# Servidor de Chat Multiusuário - Etapa 2

Este projeto implementa um servidor de chat multiusuário com foco em programação concorrente e comunicação em rede. A segunda etapa focou na criação de um protótipo funcional para o servidor de chat multiusuário, estabelecendo a comunicação básica entre cliente e servidor.

## 1. Compilação e Execução

### Comandos

```bash
# Compilar o projeto
make all

# Iniciar o servidor (porta padrão: 8080)
./chat_server

# Iniciar o cliente
./chat_client

```

Após a execução, o log é registrado no arquivo [server.log](server.log), através da biblioteca libtslog implementada na primeira etapa. 

## 2. Teste de Múltiplos Clientes
Este projeto inclui um script de teste para simular vários clientes conectando-se simultaneamente ao servidor. O teste verifica a capacidade do servidor de lidar com múltiplas conexões, troca de mensagens e broadcast.

#### Como executar os testes:

1. **Usando o script automatizado**:
   ```bash
   # Torna o script executável
   chmod +x run_tests.sh
   
   # Executa o teste completo (compila, inicia servidor se necessário e executa)
   ./run_tests.sh

## 3. Funcionalidades Implementadas (Etapa 2) 
Funcionalidades Implementadas (Etapa 2)
### 1. Servidor Multithreaded
- Implementação de um servidor que gerencia múltiplas conexões concorrentes
- Utilização de threads para lidar com cada cliente separadamente
- Sistema de identificação única para cada cliente conectado
### 2. Gerenciamento de Clientes
- Lista thread-safe para gerenciamento seguro de clientes conectados
- Detecção automática de desconexões
- Mecanismo de nomes de usuário personalizáveis
### 3. Comunicação em Rede
- Protocolo de comunicação cliente-servidor via sockets TCP/IP
- Formato de mensagem serializado para transmissão confiável
- Sistema de broadcast para enviar mensagens a todos os clientes conectados
### 4. Interface de Cliente
- Cliente de linha de comando para conexão ao servidor
- Sistema de envio e recebimento de mensagens em tempo real
- Suporte a comandos especiais como alteração de nome de usuário


### 4. Documentação da Arquitetura
 [funcionamento.md](docs/funcionamento.md): Descreve o fluxo de execução e a arquitetura geral, explica os componentes principais e suas interações e detalha os mecanismos de concorrência utilizados 

[relatorio-llm2.md](docs/relatorio-llm2.md]): Análise do desenvolvimento com IA para Etapa 2

[requisitos_atendidos.md](docs/requisitos_atendidos.md): Verificação dos requisitos atendidos

### 5. Vídeo de execução

link pro vídeo de demonstração: https://youtu.be/taHNfdk8w1A