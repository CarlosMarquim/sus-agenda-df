# Planejamento de Product Management — SUS-Agenda DF

## 1. Problema de Negócio

Atendentes de UBS realizam agendamentos sem registrar o motivo da consulta. O médico chega ao atendimento sem saber previamente o que levou o paciente até a unidade. Paralelamente, não há histórico consolidado acessível na recepção, tornando impossível identificar padrões de retorno ou sobreposição de agendamentos do mesmo paciente.

## 2. Proposta de Valor

Um sistema de terminal em C que centraliza o ciclo completo de agendamento, adicionando dois elementos ausentes nos fluxos atuais: o registro da queixa principal no ato do agendamento e o histórico de consultas do paciente acessível por CPF. Com isso, a equipe de saúde chega ao atendimento com contexto, e a recepção opera com informação organizada.

## 3. Público-Alvo e Setor

Setor: saúde pública do Distrito Federal, especificamente Unidades Básicas de Saúde (UBS).

| Perfil | Papel |
|---|---|
| Atendente da recepção | Usuário principal, opera o sistema diretamente |
| Médico e enfermeiro (eSF) | Beneficiários indiretos, recebem agenda com queixa registrada |
| Coordenador da UBS | Consulta agenda consolidada do dia |

## 4. MVP (Minimum Viable Product)

| Funcionalidade | Descrição | No MVP |
|---|---|---|
| Cadastro de paciente | Nome, CPF, data de nascimento, telefone | Sim |
| Busca de paciente por CPF | Identificação antes de qualquer agendamento | Sim |
| Histórico do paciente | Todas as consultas anteriores acessíveis por CPF | Sim |
| Cadastro de médico | Nome, CRM, especialidade | Sim |
| Disponibilidade por turno | Configuração de dias e turnos disponíveis por médico | Sim |
| Registro de queixa principal | Campo obrigatório no ato do agendamento | Sim |
| Agendamento com validação de conflito | Impede sobreposição de horários e agendamento duplo no mesmo dia | Sim |
| Geração de número de protocolo | Identificador sequencial automático por agendamento | Sim |
| Cancelamento de consulta | Por protocolo ou CPF, com liberação automática do slot | Sim |
| Agenda do dia consolidada | Todos os médicos, slots e queixas em uma única tela | Sim |
| Agenda por médico e data | Visão específica de um médico num dia selecionado | Sim |
| Interface gráfica web | Camada visual substituindo o terminal | Fora do MVP, planejada como evolução |
| Notificação de paciente (SMS/app) | Aviso automático de confirmação ou lembrete | Fora do escopo |
| Múltiplas UBS / multiusuário | Centralização entre unidades | Fora do escopo |

## 5. Backlog Priorizado

A priorização segue o critério de dependência técnica: itens que bloqueiam outros vêm primeiro, independente de quão "visível" são para quem avalia.

| Prioridade | Item | Justificativa |
|---|---|---|
| 1 | Cadastro e busca de paciente por CPF | Pré-requisito de qualquer agendamento |
| 2 | Cadastro de médico e grade de disponibilidade | Pré-requisito para exibir slots |
| 3 | Criação de agendamento com validação de conflito | Núcleo funcional do sistema |
| 4 | Registro obrigatório da queixa principal | Diferencial central, depende do agendamento já existir |
| 5 | Geração de protocolo e confirmação | Fecha o ciclo de criação |
| 6 | Cancelamento por protocolo ou CPF | Depende de agendamentos já existentes |
| 7 | Agenda do dia consolidada | Depende de múltiplos agendamentos para ser útil |
| 8 | Histórico do paciente por CPF | Depende de múltiplos agendamentos acumulados |
| 9 | Persistência em arquivo | Incremental, não bloqueia demonstração em memória |
| 10 | Interface gráfica web | Pós-MVP, contingente a tempo disponível |

## 6. Roadmap de Desenvolvimento

Prazo total: até 25 de junho de 2026, dividido em 4 fases.

| Fase | Período | Entregas | Status |
|---|---|---|---|
| 1 | Até 14/06 | Structs definidas, menu de navegação, cadastro e busca de paciente por CPF | Concluída |
| 2 | Até 18/06 | Cadastro de médicos, disponibilidade por turno, exibição de slots livres e ocupados | Concluída |
| 3 | Até 22/06 | Agendamento completo com registro de queixa, validação de conflito, protocolo, cancelamento | Em andamento |
| 4 | Até 25/06 | Visões de agenda, ajustes finais, documentação, commits organizados | Pendente |
| Contingência | Após 25/06 ou em paralelo se houver tempo | Interface gráfica web substituindo o terminal | Planejada, não bloqueia entrega |

## 7. Regras de Negócio (resumo de produto)

| Regra | Descrição |
|---|---|
| Identificador único | O CPF é o identificador único do paciente. Não podem existir dois cadastros com o mesmo CPF |
| Validação prévia | Nenhum agendamento é gravado sem validação prévia de disponibilidade do slot |
| Um agendamento por dia | Um paciente não pode ter mais de um agendamento no mesmo dia, independente do médico |
| Queixa obrigatória | Não é possível confirmar agendamento sem queixa principal registrada |
| Protocolo imutável | O número de protocolo é gerado sequencialmente e não pode ser editado manualmente |
| Cancelamento imediato | O cancelamento libera o slot imediatamente, sem estado pendente |
| Slots fixos | Slots são fixos de 30 minutos; não é possível criar horários fora da grade pré-definida |

## 8. Métricas de Sucesso

### 8.1 Métricas de produto

- Executável funcional sem crashes em todos os fluxos principais.
- Zero casos de conflito de agendamento aceitos pelo sistema.
- Queixa exibida corretamente em 100% dos slots ocupados na agenda do dia.
- Tempo de resposta de qualquer operação do menu inferior a 1 segundo.

### 8.2 Métricas de avaliação acadêmica

- Funcionamento completo do executável durante a apresentação.
- Código organizado em módulos com funções bem definidas, compilando sem warnings.
- Documentação completa no GitHub (README, docs/, manual, prints do sistema em execução).
- Commits evolutivos demonstrando progresso ao longo do desenvolvimento, não um único commit final.
- Apresentação clara do problema, da solução e da lógica implementada, dentro do limite de 20 minutos.

## 9. Riscos e Mitigação

| Risco | Impacto | Mitigação |
|---|---|---|
| Atraso na Fase 3 (núcleo de agendamento) | Alto — compromete toda a Fase 4 | Priorizar conflito de horário e protocolo antes de qualquer refinamento visual do terminal |
| Tempo insuficiente para interface gráfica | Baixo — não é requisito da disciplina | Tratada como contingência desde o início, documentada mas não bloqueante |
| Mudança de orientação do professor sobre persistência | Médio — pode exigir refatoração | Camada de persistência isolada desde a Fase 1, módulos de domínio não dependem da implementação |
| Divergência entre membros do grupo sobre responsabilidades não técnicas | Médio — atrasa entregas de documentação | Responsabilidades de documentação distribuídas e revisadas neste documento |

## 10. Divisão de Responsabilidades no Grupo

| Frente | Responsável |
|---|---|
| Desenvolvimento técnico (código C, arquitetura, documentação técnica) | Carlos |
| Apresentação e simulação do sistema | A definir com o grupo |
| Revisão de documentos não técnicos | A definir com o grupo |
