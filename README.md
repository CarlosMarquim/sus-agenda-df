# SUS-Agenda DF

Sistema de gestão de agendamentos para Unidades Básicas de Saúde (UBS) do Distrito Federal.

Projeto final da disciplina de Algoritmos e Lógica de Programação — IDP, semestre 1/2026.
Professor: Murillo Edson de Carvalho Souza.

## Sobre o projeto

O SUS-Agenda DF gerencia o ciclo completo de agendamento de consultas em UBS do DF: identificação do paciente por CPF, registro obrigatório da queixa principal, seleção de médico e horário, confirmação com número de protocolo e cancelamento.

O sistema resolve um problema real: atendentes de UBS marcam consultas sem registrar o motivo da visita, e o médico chega ao atendimento sem contexto clínico prévio. O SUS-Agenda DF adiciona dois elementos ausentes nos fluxos atuais — queixa principal obrigatória e histórico de consultas por CPF.

O núcleo do sistema é desenvolvido em linguagem C, atendendo aos requisitos técnicos da disciplina (structs, modularização em arquivos `.c`/`.h`, lógica de controle de fluxo). O mesmo core C é compilado para WebAssembly via Emscripten e executado diretamente no navegador, sem backend e sem reimplementação de lógica em JavaScript.

## Como executar

### Terminal (C nativo)

```bash
make
./sus-agenda
```

Requer apenas `gcc` e a biblioteca padrão de C. Sem dependências externas.

### Frontend web (WebAssembly)

```bash
make wasm
python -m http.server 8080 --directory web
```

Abrir `http://localhost:8080` no navegador. Requer Emscripten instalado para o build; os arquivos `.wasm` e `.js` já estão incluídos no repositório para execução direta sem recompilar.

Após o carregamento inicial, o sistema roda completamente offline. Os dados vivem em memória do módulo WebAssembly — comportamento equivalente ao terminal.

## Estrutura do repositório

```
sus-agenda-df/
├── web/                          # frontend web
│   ├── index.html                # página única (SPA)
│   ├── style.css                 # design system completo
│   ├── app.js                    # navegação, renderização, chamadas ccall
│   ├── sus-agenda.js             # glue code gerado pelo Emscripten
│   └── sus-agenda.wasm           # core C compilado para WebAssembly
├── src/                          # código-fonte em C
│   ├── main.c                    # entry point do terminal
│   ├── menu.c                    # navegação do terminal
│   ├── paciente.c
│   ├── medico.c
│   ├── slot.c
│   ├── agendamento.c
│   ├── agenda.c
│   └── bridge_wasm.c             # ponte C→JSON para o frontend WebAssembly
├── include/                      # headers de interface
│   └── *.h
├── docs/                         # documentação do projeto
│   ├── elicitacao.md             # 5W2H, público-alvo, stakeholders, custo computacional
│   ├── arquitetura.md            # arquitetura do sistema e integração WebAssembly
│   ├── product-management.md     # MVP, roadmap, backlog, métricas
│   ├── seguranca-informacao.md   # segurança da informação e LGPD
│   └── ux-ui.md                  # design system e fluxos de usuário
├── imagens/                      # prints do sistema
├── executavel/                   # binário compilado (terminal)
├── Makefile
├── manual.md
└── README.md
```

## Documentação

| Documento | Conteúdo |
|---|---|
| [docs/elicitacao.md](docs/elicitacao.md) | 5W2H, público-alvo, stakeholders, custo computacional |
| [docs/arquitetura.md](docs/arquitetura.md) | Módulos, estruturas de dados, WebAssembly, fluxo de camadas |
| [docs/product-management.md](docs/product-management.md) | Problema de negócio, MVP, backlog, roadmap, métricas |
| [docs/seguranca-informacao.md](docs/seguranca-informacao.md) | Controle de acesso, dados sensíveis, LGPD |
| [docs/ux-ui.md](docs/ux-ui.md) | Personas, fluxos de usuário, design system implementado |

## Status do desenvolvimento

| Fase | Entregas | Status |
|---|---|---|
| 1 — Base e cadastro de pacientes | Structs definidas, menu de navegação, cadastro e busca por CPF | Concluída |
| 2 — Médicos e grade de slots | Cadastro de médicos, disponibilidade por turno, grade de slots | Concluída |
| 3 — Agendamento completo | Registro de queixa, validação de conflito, protocolo, cancelamento | Concluída |
| 4 — Agenda e documentação | Visões de agenda, ajustes finais, documentação | Concluída |
| 5 — Frontend WebAssembly | Interface web com core C compilado para WASM via Emscripten | Concluída |

## Equipe

Desenvolvimento técnico (core C e frontend WebAssembly): Carlos.
Demais responsabilidades do grupo: ver `docs/product-management.md`.
