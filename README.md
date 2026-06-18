# SUS-Agenda DF

Sistema de gestão de agendamentos para Unidades Básicas de Saúde (UBS) do Distrito Federal.

Projeto final da disciplina de Algoritmos e Lógica de Programação — IDP, semestre 1/2026.
Professor: Murillo Edson de Carvalho Souza.

## Sobre o projeto

O SUS-Agenda DF é um sistema operado pelo atendente da recepção de uma UBS para gerenciar todo o ciclo de um agendamento de consulta: identificação do paciente, registro da queixa principal, seleção de médico e horário, confirmação com número de protocolo e cancelamento.

O núcleo do sistema é desenvolvido em linguagem C, executado em terminal, atendendo aos requisitos técnicos da disciplina (structs, modularização em arquivos `.c`/`.h`, lógica de controle de fluxo). Uma interface gráfica web está planejada como evolução do projeto, documentada no plano de UX/UI deste repositório.

O problema que o sistema resolve é real: atendentes de UBS hoje marcam consultas sem registrar o motivo da visita, e o médico chega ao atendimento sem contexto prévio. Não existe, na maior parte dos fluxos atuais, histórico consolidado do paciente acessível à recepção. O SUS-Agenda DF adiciona esses dois elementos — queixa principal obrigatória no agendamento e histórico por CPF — tornando o processo mais simples e organizado para a equipe de saúde.

## Estrutura do repositório

```
sus-agenda-df/
├── src/                          # código-fonte em C
│   ├── main.c
│   ├── paciente.c
│   ├── medico.c
│   ├── slot.c                    # grade de slots por dia (Fase 2)
│   ├── menu.c
│   ├── agendamento.c             # [futuro — Fase 3]
│   └── agenda.c                  # [futuro — Fase 4]
├── include/                      # headers de interface
│   ├── paciente.h
│   ├── medico.h
│   ├── slot.h
│   ├── agendamento.h
│   ├── agenda.h                  # [futuro — Fase 4]
│   └── menu.h
├── docs/                         # documentação do projeto
│   ├── elicitacao.md             # 5W2H, público-alvo, stakeholders, custo computacional
│   ├── arquitetura.md            # arquitetura do sistema
│   ├── product-management.md     # MVP, roadmap, backlog, métricas de produto
│   ├── seguranca-informacao.md   # plano de segurança da informação
│   └── ux-ui.md                  # plano de UX/UI e design system
├── imagens/                      # prints e capturas de tela do sistema
├── executavel/                   # binário compilado
├── Makefile
└── manual.md                     # manual de uso do sistema
```

## Documentação

| Documento | Conteúdo |
|---|---|
| [docs/elicitacao.md](docs/elicitacao.md) | Levantamento de requisitos: 5W2H, público-alvo, stakeholders, custo computacional |
| [docs/arquitetura.md](docs/arquitetura.md) | Módulos, estruturas de dados, fluxo de camadas, estrutura de arquivos |
| [docs/product-management.md](docs/product-management.md) | Problema de negócio, MVP, backlog priorizado, roadmap, métricas de sucesso |
| [docs/seguranca-informacao.md](docs/seguranca-informacao.md) | Controle de acesso, dados sensíveis, LGPD, auditoria, plano de resposta |
| [docs/ux-ui.md](docs/ux-ui.md) | Personas, fluxos de usuário, design system, wireframes da futura interface gráfica |

## Como compilar e executar

```bash
make
./sus-agenda
```

Requer apenas `gcc` e a biblioteca padrão de C. Sem dependências externas.

## Status do desenvolvimento

| Fase | Entregas | Status |
|---|---|---|
| 1 — Base e cadastro de pacientes | Structs definidas, menu de navegação, cadastro e busca por CPF | Concluída |
| 2 — Médicos e grade de slots | Cadastro de médicos, disponibilidade por turno, grade de slots | Concluída |
| 3 — Agendamento completo | Registro de queixa, validação de conflito, protocolo, cancelamento | Em andamento |
| 4 — Agenda e documentação | Visões de agenda, ajustes finais, documentação | Pendente |

Prazo de entrega: 25 de junho de 2026.

## Equipe

Desenvolvimento técnico: Carlos.
Demais responsabilidades do grupo: ver `docs/product-management.md`.
