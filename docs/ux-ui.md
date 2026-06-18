# Plano de UX/UI e Design System — SUS-Agenda DF

## 1. Objetivo deste Documento

Este documento especifica o design da futura interface gráfica web do SUS-Agenda DF, que substituirá o terminal como camada de apresentação do sistema. A lógica de negócio (validações, regras de conflito, estruturas de dados) permanece a mesma descrita em `docs/arquitetura.md`; o que muda é exclusivamente a forma como o atendente interage com o sistema.

O documento cobre perfil de usuário, fluxos de tela, design system (paleta, tipografia, componentes) e wireframes descritos o suficiente para implementação direta.

## 2. Perfil Completo do Usuário

### 2.1 Persona primária — Ana, Atendente de Recepção

| Campo | Descrição |
|---|---|
| Idade e contexto | 34 anos, atendente administrativa em UBS do Distrito Federal |
| Rotina de trabalho | Atende presencialmente e por telefone, lida com fila de espera, intercala atendimento de pacientes com tarefas administrativas |
| Familiaridade digital | Usa computador no trabalho diariamente, mas não é usuária avançada; aprendeu sistemas anteriores por repetição, não por leitura de manual |
| Ambiente de uso | Tela de computador de mesa, mouse e teclado, ambiente com interrupções frequentes (paciente chegando, telefone tocando) |
| Objetivo no sistema | Completar um agendamento o mais rápido possível, sem erros, mantendo a fila organizada |
| Frustrações conhecidas | Sistemas com muitos cliques para uma tarefa simples; telas com informação demais ao mesmo tempo; mensagens de erro confusas ou genéricas |
| Necessidade de design | Fluxo linear e previsível, poucos cliques por tarefa, texto grande e legível à distância de leitura normal, feedback imediato e claro de cada ação |

### 2.2 Persona secundária — Dr. Carlos, Médico de Família

| Campo | Descrição |
|---|---|
| Idade e contexto | 42 anos, clínico geral da equipe de Saúde da Família |
| Relação com o sistema | Não opera o cadastro ou agendamento; consulta a agenda do dia antes ou durante o atendimento |
| Ambiente de uso | Pode consultar em tablet ou computador do consultório, por períodos curtos entre pacientes |
| Necessidade de design | Visualização rápida e escaneável da agenda, com destaque visual para a queixa do paciente, sem necessidade de navegação complexa |

### 2.3 Implicações de design derivadas do perfil

- Poucos cliques por tarefa: o fluxo de agendamento completo (identificar paciente, registrar queixa, escolher slot, confirmar) deve caber em uma sequência curta de telas, sem etapas redundantes.
- Tolerância a interrupção: o estado do formulário não deve se perder se a atendente precisar pausar; campos preenchidos permanecem visíveis ao retornar à tela.
- Hierarquia visual clara: a ação primária de cada tela (avançar, confirmar) deve ser visualmente dominante, evitando que a atendente clique no botão errado sob pressão de tempo.
- Texto e contraste: tipografia legível a distância de braço, contraste alto entre texto e fundo, dado o uso prolongado ao longo do turno de trabalho.
- Feedback imediato: toda ação (cadastro, agendamento, cancelamento) confirma visualmente o resultado de forma inequívoca antes de a atendente avançar.

## 3. Fluxos de Usuário

### 3.1 Fluxo principal — Novo Agendamento

```
[Tela inicial] 
    → Buscar paciente por CPF
        → [Paciente encontrado] → Tela de perfil do paciente (dados + histórico)
        → [Paciente não encontrado] → Tela de cadastro rápido
    → Tela de Nova Consulta
        → Selecionar especialidade (cards clicáveis, não dropdown)
        → Selecionar médico (lista filtrada pela especialidade)
        → Tela de Grade de Horários
            → Selecionar slot disponível
    → Tela de Confirmação (resumo: paciente, médico, data/horário, campo de queixa)
        → Preencher queixa principal (obrigatório)
        → Confirmar
    → Tela de Sucesso (número de protocolo em destaque)
```

### 3.2 Fluxo secundário — Cancelamento

```
[Tela inicial]
    → Buscar por protocolo ou CPF
    → Lista de consultas futuras vinculadas
        → Selecionar consulta
    → Tela de Confirmação de Cancelamento
        → Confirmar
    → Tela de Sucesso (slot liberado)
```

### 3.3 Fluxo terciário — Consulta de Agenda

```
[Tela inicial]
    → Agenda do Dia (visão consolidada, todos os médicos)
        → Filtrar por médico (opcional)
        → Clicar em slot ocupado → ver detalhe (paciente + queixa)
```

## 4. Design System

### 4.1 Princípios de design

- Clareza acima de densidade: cada tela mostra o necessário para a tarefa atual, não tudo que o sistema sabe.
- Ação primária sempre visível: botões de avançar/confirmar usam a cor de destaque e ficam fixos na parte inferior da tela em fluxos longos.
- Estados explícitos: livre, ocupado, cancelado e confirmado têm cores e ícones distintos e consistentes em toda a interface.

### 4.2 Paleta de Cores

| Token | Cor | Uso |
|---|---|---|
| `--color-primary` | `#1F5C99` (azul institucional) | Botões primários, cabeçalhos, elementos de navegação ativa |
| `--color-primary-hover` | `#174A7A` | Estado de hover/pressed do botão primário |
| `--color-secondary` | `#2E9E6B` (verde) | Indicadores de disponibilidade, confirmações de sucesso |
| `--color-danger` | `#C0392B` (vermelho) | Cancelamento, slots ocupados, mensagens de erro |
| `--color-warning` | `#D69E2E` (amarelo/âmbar) | Avisos não bloqueantes (ex: campo obrigatório não preenchido) |
| `--color-background` | `#F7F9FB` | Fundo geral das telas |
| `--color-surface` | `#FFFFFF` | Cartões, formulários, áreas de conteúdo |
| `--color-text-primary` | `#1A1A1A` | Texto principal |
| `--color-text-secondary` | `#5C6770` | Texto auxiliar, labels, metadados |
| `--color-border` | `#D9E1E7` | Bordas de inputs e divisores |

A paleta segue o padrão de aplicações de saúde pública: azul como cor institucional confiável, verde para disponibilidade/sucesso, vermelho reservado exclusivamente para ações destrutivas ou bloqueios, evitando uso de vermelho como cor decorativa.

### 4.3 Tipografia

| Token | Especificação | Uso |
|---|---|---|
| `--font-family` | `'Inter', system-ui, sans-serif` | Fonte única em toda a interface, para consistência e legibilidade em tela |
| `--font-size-display` | 28px / bold | Título de tela (ex: "Nova Consulta") |
| `--font-size-heading` | 20px / semibold | Subtítulos de seção dentro de uma tela |
| `--font-size-body` | 16px / regular | Texto padrão de formulários e listas |
| `--font-size-label` | 14px / medium | Labels de campos de formulário |
| `--font-size-caption` | 12px / regular | Metadados secundários (ex: "cadastrado em 10/06/2026") |

Tamanho mínimo de 16px para texto de corpo, considerando uso prolongado em jornada de trabalho e leitura a distância de tela de mesa.

### 4.4 Espaçamento e Grid

| Token | Valor |
|---|---|
| `--space-xs` | 4px |
| `--space-sm` | 8px |
| `--space-md` | 16px |
| `--space-lg` | 24px |
| `--space-xl` | 40px |

Grid de 8px como unidade base para todo espaçamento entre elementos, garantindo consistência visual.

### 4.5 Componentes

#### Botão primário
- Fundo `--color-primary`, texto branco, altura mínima de 48px (área de toque confortável mesmo em uso com mouse sob pressão de tempo).
- Cantos arredondados de 8px.
- Estado de hover muda para `--color-primary-hover`; estado desabilitado usa opacidade 40%.

#### Botão secundário (ações como "voltar" ou "cancelar fluxo")
- Fundo transparente, borda `--color-border`, texto `--color-text-primary`.

#### Card de slot (grade de horários)
- Slot livre: fundo `--color-surface`, borda `--color-secondary`, texto do horário em destaque, clicável.
- Slot ocupado: fundo `#F0F0F0`, borda `--color-border`, exibe nome do paciente truncado, não clicável (cursor `not-allowed`).
- Slot selecionado (após clique): fundo `--color-primary`, texto branco.

#### Campo de formulário (input de texto)
- Altura mínima 44px, borda `--color-border`, borda muda para `--color-primary` em foco.
- Label sempre visível acima do campo (não usar apenas placeholder como label, para evitar perda de contexto ao preencher).
- Campo de queixa principal: `textarea` com contador de caracteres visível (0/200), já que é o campo central do diferencial do sistema.

#### Badge de status
- Badge "Disponível": fundo `--color-secondary` com 15% de opacidade, texto `--color-secondary`.
- Badge "Ocupado": fundo `--color-text-secondary` com 15% de opacidade, texto `--color-text-secondary`.
- Badge "Cancelado": fundo `--color-danger` com 15% de opacidade, texto `--color-danger`.

#### Toast de confirmação
- Aparece no canto superior direito, fundo `--color-secondary` para sucesso ou `--color-danger` para erro, desaparece automaticamente após 4 segundos ou ao clicar.

### 4.6 Ícones

Uso de um único conjunto de ícones em todo o sistema (recomendado: Lucide ou Material Symbols), evitando misturar bibliotecas. Ícones usados com significado consistente:

| Ícone | Significado |
|---|---|
| Lupa | Buscar paciente |
| Usuário | Perfil/cadastro de paciente |
| Calendário | Agenda, data |
| Relógio | Horário, slot |
| Check circular | Confirmação, sucesso |
| X circular | Cancelamento |
| Alerta triangular | Aviso/erro de validação |

## 5. Wireframes Descritos

### 5.1 Tela Inicial

Cabeçalho fixo no topo com o nome do sistema e a data atual. Corpo da tela dividido em três acessos rápidos em formato de cartão grande, lado a lado: "Nova Consulta", "Buscar Paciente", "Agenda do Dia". Cada cartão tem um ícone grande, título e uma linha de descrição curta. Essa tela funciona como ponto de partida único, evitando menu lateral complexo, dado que a atendente realiza poucas tarefas distintas repetidamente.

### 5.2 Tela de Busca de Paciente

Campo de busca por CPF centralizado e em destaque, com máscara de formatação automática (000.000.000-00) enquanto digita. Botão "Buscar" abaixo. Se não encontrado, exibe mensagem clara com botão "Cadastrar novo paciente" logo abaixo, evitando que a atendente precise navegar para outro menu.

### 5.3 Tela de Perfil do Paciente

Cabeçalho com nome e CPF do paciente em destaque. Abaixo, duas seções: "Histórico de Consultas" (lista cronológica reversa, cada item mostrando data, médico, especialidade e queixa registrada) e botão de ação primária "Agendar Nova Consulta", fixo na parte inferior da tela.

### 5.4 Tela de Seleção de Especialidade

Grid de cartões clicáveis (não dropdown), um por especialidade (Clínica Geral, Pediatria, Ginecologia, Ortopedia, Gastroenterologia), cada um com ícone representativo. Cartões clicáveis em vez de menu suspenso reduzem a chance de seleção errada sob pressão de tempo e tornam a opção escolhida visualmente óbvia.

### 5.5 Tela de Grade de Horários

Cabeçalho com nome e especialidade do médico selecionado, e seletor de data (padrão: dia atual). Corpo dividido em duas colunas, "Manhã" e "Tarde", cada uma listando os 9 slots daquele turno como cards conforme especificado na seção 4.5. Slots ocupados mostram o nome do paciente truncado; ao passar o mouse, exibem a queixa em tooltip (útil para o médico consultando a mesma tela).

### 5.6 Tela de Confirmação de Agendamento

Resumo em formato de cartão: nome e CPF do paciente, médico e especialidade, data e horário selecionados. Abaixo do resumo, campo de texto obrigatório para a queixa principal, com contador de caracteres. Botão primário "Confirmar Agendamento" desabilitado até que a queixa seja preenchida, reforçando visualmente a regra de negócio RN04 (queixa obrigatória) sem depender de mensagem de erro após o fato.

### 5.7 Tela de Sucesso

Ícone de confirmação grande e centralizado, número de protocolo em destaque tipográfico maior que o restante da tela (já que é a informação que a atendente vai repassar ao paciente verbalmente), e botão para retornar à tela inicial.

## 6. Acessibilidade

- Contraste mínimo de 4.5:1 entre texto e fundo em todos os componentes, seguindo WCAG AA.
- Todos os botões e campos interativos têm área de toque mínima de 44x44px, mesmo em uso com mouse, para reduzir erro de clique.
- Estados de foco visíveis (outline) em todos os elementos navegáveis por teclado, já que parte do fluxo pode ser operado sem mouse.
- Nenhuma informação crítica é transmitida exclusivamente por cor (ex: status de slot usa cor e texto/ícone simultaneamente).

## 7. Stack de Implementação Planejada

Conforme alinhado no planejamento do projeto, a interface gráfica será implementada como aplicação web, com a lógica de negócio reescrita na linguagem do backend escolhido (Python), mantendo a versão em C como artefato avaliado na disciplina de Algoritmos e Lógica de Programação. Essa interface é tratada como entrega de contingência, condicionada à conclusão das Fases 3 e 4 do núcleo em C, conforme `docs/product-management.md`.
