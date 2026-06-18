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
│   ├── agendamento.c       # [futuro — Fase 3]
│   └── agenda.c            # [futuro — Fase 4]
├── include/
│   ├── paciente.h
│   ├── medico.h
│   ├── slot.h
│   ├── agendamento.h
│   ├── agenda.h            # [futuro — Fase 4]
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

## 7. Evolução para Interface Gráfica

A arquitetura modular permite duas estratégias de evolução para uma interface gráfica web, sem reescrever a lógica de negócio em C:

**Estratégia A — API intermediária.** Um pequeno servidor (Python/Flask ou Node/Express) expõe endpoints HTTP que chamam o executável C via processo externo, ou leem/escrevem o mesmo arquivo de persistência usado pelo terminal. O frontend web consome esses endpoints.

**Estratégia B — Reimplementação da lógica.** A lógica de negócio (validações, regras de conflito) é portada para a linguagem do backend escolhido, mantendo a versão em C como o artefato avaliado na disciplina. Essa é a estratégia mais simples de implementar dado o prazo do projeto, e é a adotada no plano de UX/UI deste repositório.

Detalhes de paleta, componentes e wireframes da interface gráfica estão documentados em `docs/ux-ui.md`.
