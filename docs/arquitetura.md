# Arquitetura do Sistema — SUS-Agenda DF

## 1. Visão Geral da Arquitetura

O sistema segue arquitetura modular em camadas, separando interface de terminal, lógica de negócio e persistência de dados. Essa separação garante que mudanças na forma de entrada/saída (por exemplo, a futura interface gráfica) não exijam alterações na lógica de negócio.

```
[ menu.c ] → interface e navegação (camada de apresentação)
    ↓
[ agendamento.c | paciente.c | medico.c | agenda.c ] → lógica de negócio (camada de domínio)
    ↓
[ arrays em memória / arquivos .dat ] → persistência de dados (camada de dados)
```

Cada módulo de domínio expõe funções através de um header (`.h`) e implementa a lógica em um arquivo `.c` correspondente. Nenhum módulo de domínio depende diretamente da camada de apresentação, o que permite, no futuro, substituir o terminal por uma interface gráfica sem reescrever as regras de negócio.

## 2. Módulos e Responsabilidades

| Módulo | Arquivos | Responsabilidade |
|---|---|---|
| Paciente | `paciente.c` / `paciente.h` | Cadastro, busca por CPF, exibição de dados e histórico de consultas |
| Médico | `medico.c` / `medico.h` | Cadastro de médicos, especialidades e disponibilidade por dia/turno |
| Agendamento | `agendamento.c` / `agendamento.h` | Criação, confirmação, cancelamento e validação de conflitos de horário |
| Agenda | `agenda.c` / `agenda.h` | Visões consolidadas: agenda do dia, agenda por médico, histórico por paciente |
| Menu | `menu.c` / `menu.h` | Interface de navegação e fluxo de interação no terminal |
| Main | `main.c` | Ponto de entrada, inicialização das estruturas globais e chamada ao menu principal |

## 3. Estruturas de Dados Principais

Cada entidade é representada por uma `struct` em C. O relacionamento entre entidades é feito por índices inteiros que referenciam posições em arrays globais, evitando ponteiros complexos e mantendo o código simples de auditar.

### 3.1 Paciente

```c
typedef struct {
    char cpf[12];              // identificador único, 11 dígitos + terminador
    char nome[100];
    char dataNascimento[11];   // formato DD/MM/AAAA
    char telefone[16];
    int historicoIds[50];      // índices de agendamentos vinculados a este paciente
    int numHistorico;
} Paciente;
```

### 3.2 Médico

```c
typedef struct {
    char nome[100];
    char crm[15];
    char especialidade[50];
    int disponibilidade[7][2]; // [dia da semana 0-6][turno 0=manhã,1=tarde] = 1 disponível / 0 indisponível
} Medico;
```

### 3.3 Agendamento

```c
typedef struct {
    int protocolo;       // identificador sequencial, gerado automaticamente
    int idPaciente;       // índice no array de pacientes
    int idMedico;          // índice no array de médicos
    int slot;              // índice de 0 a 17 na grade de slots do dia
    char data[11];         // formato DD/MM/AAAA
    char queixa[201];      // texto livre, até 200 caracteres + terminador
    int cancelado;         // 0 = ativo, 1 = cancelado
} Agendamento;
```

## 4. Fluxo de Dados entre Módulos

A criação de um agendamento ilustra como os módulos colaboram sem acoplamento direto:

1. `menu.c` solicita ao atendente o CPF do paciente e chama `paciente_buscar_cpf()`.
2. Se o paciente não existir, `menu.c` chama `paciente_cadastrar()`, que retorna o índice do novo registro.
3. `menu.c` solicita a especialidade e chama `medico_filtrar_por_especialidade()`, que retorna a lista de médicos elegíveis.
4. Com o médico escolhido, `menu.c` chama `agenda_exibir_grade(idMedico, data)`, que consulta `medico.disponibilidade` e os agendamentos já existentes para montar a grade de slots livres e ocupados.
5. O atendente escolhe um slot. `menu.c` chama `agendamento_criar(idPaciente, idMedico, slot, data, queixa)`.
6. `agendamento_criar()` valida: (a) o slot está livre, (b) o paciente não tem outro agendamento no mesmo dia, (c) a queixa não está vazia. Se tudo for válido, grava o agendamento e retorna o número de protocolo.
7. `menu.c` exibe o resumo de confirmação e o protocolo ao atendente.

Nenhuma dessas etapas exige que `agendamento.c` conheça detalhes de exibição em terminal, nem que `medico.c` conheça a lógica de validação de agendamento. Essa independência é o que sustenta a possibilidade de trocar a camada de apresentação no futuro.

## 5. Persistência de Dados

A persistência é tratada como uma responsabilidade isolada dentro de cada módulo de domínio, sem vazamento de detalhes de implementação para os demais módulos. O sistema suporta dois modelos sem alteração na lógica de negócio:

| Modelo | Descrição |
|---|---|
| Em memória (sem arquivo) | Arrays inicializados na execução. Dados resetam ao fechar o programa. |
| Arquivos binários (`.dat`) | Funções de salvar e carregar isoladas em cada módulo (`paciente_salvar()`, `paciente_carregar()`, etc.). Os demais módulos não conhecem a implementação de persistência, apenas chamam essas funções na inicialização e no encerramento do programa. |

Entidades persistidas: Paciente, Médico, Agendamento.

## 6. Estrutura de Arquivos do Repositório

```
sus-agenda-df/
├── src/
│   ├── main.c
│   ├── paciente.c
│   ├── medico.c
│   ├── slot.c              # grade de slots por dia (Fase 2)
│   ├── menu.c
│   ├── agendamento.c
│   └── agenda.c
├── include/
│   ├── paciente.h
│   ├── medico.h
│   ├── slot.h
│   ├── agendamento.h
│   ├── agenda.h
│   └── menu.h
├── docs/
│   ├── elicitacao.md
│   ├── arquitetura.md
│   ├── product-management.md
│   ├── seguranca-informacao.md
│   └── ux-ui.md
├── imagens/
├── executavel/
├── Makefile
├── README.md
└── manual.md
```

## 7. Interface Web — WebAssembly via Emscripten

A interface gráfica web foi implementada compilando o core C diretamente para WebAssembly via Emscripten. Nenhuma lógica de negócio foi reimplementada em JavaScript.

### 7.1 Estratégia adotada

O mesmo código C que executa no terminal é compilado para WASM com `make wasm`. O JavaScript atua exclusivamente como camada de apresentação: navega entre telas, renderiza HTML e chama funções C via `Module.ccall()`.

### 7.2 Camada de ponte (bridge_wasm.c)

`src/bridge_wasm.c` expõe funções C com assinaturas simples que o JavaScript pode chamar diretamente. Cada função recebe parâmetros primitivos (strings e inteiros) e retorna JSON formatado com `snprintf`. Exemplo:

```c
// bridge_wasm.c
char* bridge_paciente_cadastrar(const char* nome, const char* cpf,
                                 const char* dataNasc, const char* telefone) {
    // chama paciente_registrar() — mesma função usada pelo terminal
    // retorna: {"sucesso": true, "id": 3}
    //       ou {"sucesso": false, "erro": "CPF já cadastrado"}
}
```

`main.c` e `menu.c` não entram no build WASM — são exclusivos do terminal.

### 7.3 Estrutura do frontend

```
web/
├── index.html        # SPA — uma única página, telas trocadas via JS
├── style.css         # design system completo (paleta, tipografia, componentes)
├── app.js            # navegação entre telas e chamadas Module.ccall()
├── sus-agenda.js     # glue code gerado pelo Emscripten
└── sus-agenda.wasm   # core C compilado para WebAssembly
```

Fontes externas: apenas Google Fonts (Plus Jakarta Sans e IBM Plex Sans) via `<link>`. Zero frameworks, zero bibliotecas de componentes.

### 7.4 Comportamento em execução

- Após o carregamento inicial, o sistema roda completamente offline no cliente
- Dados vivem em memória do módulo WASM — perdem-se ao fechar a aba, comportamento equivalente ao terminal
- Para servir localmente: `python -m http.server 8080 --directory web`

### 7.5 Separação de responsabilidades mantida

A separação de camadas descrita na seção 1 se preserva integralmente: o JavaScript substitui `menu.c` como camada de apresentação, enquanto toda a lógica de negócio permanece nos módulos C originais. Trocar o frontend por outro framework no futuro não exigiria alterar nenhum arquivo `.c` do domínio.
