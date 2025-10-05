
### Relatório Final: Análise Crítica e Detalhada do Uso de IA (LLMs) no Projeto de Programação Concorrente

Este relatório apresenta uma análise aprofundada do uso de Modelos de Linguagem de Grande Escala (LLMs) no desenvolvimento do projeto de Programação Concorrente (Tema A - Servidor de Chat Multiusuário). Ele discute o valor agregado, os desafios enfrentados e as lições aprendidas em cada uma das três etapas do projeto, culminando em uma estratégia robusta para a utilização de IA em documentação e desenvolvimento.

#### 1. Valor Agregado e Aceleração do Desenvolvimento

O uso da IA atuou como um acelerador multifacetado, otimizando o tempo e a qualidade em todas as fases do projeto.

* **Aceleração da Etapa 1 (Arquitetura e Logging)**: A IA forneceu rapidamente um esqueleto de código para a biblioteca de logging **thread-safe** (`libtslog`). Ao gerar protótipos e estruturas iniciais, como a definição de **mutexes** para exclusão mútua, a ferramenta permitiu que a equipe se concentrasse na compreensão dos conceitos de concorrência em vez de na sintaxe básica[cite: 57]. Além disso, a IA gerou um `Makefile` que segue as convenções do setor, garantindo a modularidade e a organização do projeto desde o início.

* **Aceleração da Etapa 2 (Protótipo de Comunicação)**: A IA facilitou a transição da lógica de programação procedural (típica de exemplos de sockets em C) para a **arquitetura orientada a objetos** em C++. Ela traduziu as funções de `bind()`, `listen()`, `accept()`, `send()` e `recv()` para métodos encapsulados dentro das classes `ChatServer` e `Client`, o que acelerou a construção do protótipo funcional e reforçou a compreensão da lógica de **sockets**[cite: 61].

* **Apoio à Documentação e Análise** (Etapa 3): A IA foi utilizada para a criação de um **diagrama de sequência** detalhado do fluxo cliente-servidor, que se tornou uma parte crucial da documentação final. A ferramenta também auxiliou na elaboração do relatório final, ajudando a mapear requisitos e a estruturar a análise crítica, cumprindo um dos requisitos do projeto.

#### 2. Análise Crítica e Riscos de Segurança e Concorrência

Apesar dos benefícios, a IA apresentou falhas significativas que reforçaram a necessidade de validação humana e um olhar crítico sobre suas respostas.

* **Alucinações e Invenção de Código**: O problema mais sério foi a **alucinação de código**. Em um incidente, a IA gerou exemplos de código que não existiam no projeto, fabricando funcionalidades para demonstração de requisitos como monitores e tratamento de erros. A ferramenta chegou a sugerir a existência de um agendador de jobs e um servidor HTTP, funcionalidades que não eram parte do Tema A. Isso destacou que a IA não tem conhecimento real sobre o código-fonte, mas sim a capacidade de gerar código plausível baseado em padrões.

* **Limitações na Análise de Concorrência**: A IA se mostrou limitada na capacidade de analisar problemas de concorrência complexos. Embora tenha explicado a necessidade de um **mutex** para evitar *race conditions* em uma lista de clientes, ela não foi capaz de identificar **deadlocks** ou **starvation** que poderiam surgir da interação entre múltiplos clientes e recursos compartilhados. Sua análise foi restrita a prompts específicos e não ao contexto dinâmico de execução.

* **Inconsistências e Falhas de Código**: A IA ocasionalmente gerou código com falhas, como misturar bibliotecas de C e C++, o que resultaria em erros de compilação. Ela também não garantiu por padrão que o gerenciamento de recursos, como o fechamento de sockets, seria tratado de forma robusta em todas as condições (por exemplo, em caso de falha de thread). Isso exigiu que o programador implementasse manualmente o padrão **RAII** (`Resource Acquisition Is Initialization`) para prevenir **vazamentos de recursos**.

#### 3. Estratégia de Mitigação e Lições Aprendidas

Para mitigar os riscos e maximizar a eficácia do uso da IA, a seguinte estratégia foi adotada:

* **Validação Rigorosa**: Todo o código gerado foi submetido a uma **revisão e validação manual** rigorosa. O código de concorrência foi testado com simulações de múltiplos clientes para garantir a ausência de *race conditions*, um passo que a IA não pode executar.
* **Prompts Iterativos e Específicos**: A abordagem de "um prompt, uma solução" foi evitada. Em vez disso, a IA foi utilizada de forma **iterativa e com prompts específicos** para construir componentes menores, como a lógica do servidor separada da lógica do cliente. Isso permitiu uma validação incremental do código e do design.
* **Foco na Compreensão**: A principal função da IA foi atuar como uma ferramenta de aprendizado. O foco foi em entender o **porquê** de cada solução (por exemplo, a necessidade de uma lista de clientes thread-safe) em vez de apenas copiar e colar o código.

---

#### 4. Conclusão: IA como Assistente, Não como Autoridade

As interações ao longo do projeto demonstram claramente que a IA é uma **ferramenta de apoio poderosa e um acelerador de desenvolvimento**, mas não uma autoridade definitiva. O incidente da **alucinação de código** e a incapacidade de realizar tarefas fora de seu escopo (como gerar uma imagem) reforçaram a necessidade de uma postura crítica.

A lição final é que o conhecimento técnico, a validação humana e a supervisão contínua são indispensáveis para garantir a **robustez e a conformidade do sistema**. O uso da IA deve ser sempre visto como uma colaboração, onde o programador é o responsável final pela qualidade e integridade do código.