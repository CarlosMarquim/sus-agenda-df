# Plano de UX/UI, Frontend e Integração — SUS-Agenda DF (versão completa)

## 1. Objetivo deste documento

Este documento substitui e expande `docs/ux-ui.md`. Cobre, em um único lugar: quem usa o sistema e em que condições, todas as telas necessárias para paridade funcional completa com o terminal, o design system, os critérios de acessibilidade e usabilidade específicos do contexto de UBS do Distrito Federal, e a arquitetura técnica exata de integração entre a interface web e o código C real, via WebAssembly. Serve como base única para a implementação do frontend, sem necessidade de reescrever nenhuma regra de negócio.

A lógica de negócio (validações, regras de conflito, geração de protocolo, estruturas de dados) é a mesma já implementada e verificada em C, descrita em `docs/arquitetura.md`. Nada neste documento propõe nova regra de negócio; tudo aqui é camada de apresentação e integração.

## 2. Usuários e contexto de uso

### 2.1 Persona 1 — Ana, Atendente de Recepção

| Campo | Descrição |
|---|---|
| Idade e contexto | 34 anos, atendente administrativa em UBS do Distrito Federal |
| Rotina de trabalho | Atende presencialmente e por telefone, lida com fila de espera, intercala atendimento de pacientes com tarefas administrativas |
| Familiaridade digital | Usa computador no trabalho diariamente, mas não é usuária avançada; aprendeu sistemas anteriores por repetição, não por leitura de manual |
| Ambiente de uso | Computador de mesa, mouse e teclado, ambiente com interrupções frequentes |
| Objetivo no sistema | Completar um agendamento o mais rápido possível, sem erros, mantendo a fila organizada |
| Frustrações conhecidas | Sistemas com muitos cliques para uma tarefa simples; telas com informação demais ao mesmo tempo; mensagens de erro confusas ou genéricas |
| Necessidade de design | Fluxo linear e previsível, poucos cliques por tarefa, texto grande e legível, feedback imediato e claro de cada ação |

### 2.2 Persona 2 — Dr. Carlos, Médico de Família

| Campo | Descrição |
|---|---|
| Idade e contexto | 42 anos, clínico geral da equipe de Saúde da Família |
| Relação com o sistema | Não opera cadastro ou agendamento; consulta a agenda do dia antes ou durante o atendimento |
| Ambiente de uso | Tablet ou computador do consultório, por períodos curtos entre pacientes |
| Necessidade de design | Visualização rápida e escaneável da agenda, com destaque visual para a queixa do paciente, sem navegação complexa |

### 2.3 Persona 3 — Coordenador da UBS (nova)

| Campo | Descrição |
|---|---|
| Idade e contexto | Gestor administrativo da unidade, responsável por acompanhar o funcionamento geral da UBS, não atende pacientes diretamente |
| Relação com o sistema | Consulta a agenda consolidada do dia (todos os médicos) e, ocasionalmente, o histórico de um paciente específico em caso de reclamação ou auditoria interna |
| Padrão de uso | Sessões curtas e esporádicas, geralmente uma vez por turno ou por dia, não uso contínuo como a Ana |
| Objetivo no sistema | Ter uma visão agregada e rápida: quantos atendimentos no dia, quais médicos estão com a agenda cheia, se há algum problema visível (cancelamentos em excesso, médico sem disponibilidade configurada) |
| Necessidade de design | Painel de visão geral com números e indicadores antes de qualquer lista detalhada; nenhuma tarefa de cadastro ou edição nessa visão, apenas leitura |

Esta persona já está nomeada na especificação original do projeto ("Coordenador da UBS — consulta agenda consolidada do dia") e na regra RF16 (agenda do dia consolidada). A tela de Dashboard descrita na seção 4.1 atende diretamente a essa persona.

### 2.4 Dificuldades contextuais do ambiente de UBS-DF

Estas são inferências plausíveis a partir do contexto de saúde pública, não fatos levantados em campo sobre uma UBS específica. Servem para justificar decisões de design concretas, não como dado de pesquisa formal.

- **Letramento digital heterogêneo.** Atendentes de UBS variam bastante em familiaridade com computadores, e em geral aprendem sistemas por repetição de tarefa, não por leitura de manual. Implicação de design: cada tela deve ser compreensível pelo rótulo e pela posição dos elementos, sem depender de ícones ambíguos ou jargão técnico; ações destrutivas (cancelar) sempre pedem confirmação explícita em texto, nunca só um ícone de "X".
- **Computador compartilhado entre turnos.** É comum mais de um atendente usar o mesmo equipamento em turnos diferentes. Implicação de design: nenhuma tela deve depender de estado "lembrado" de uma sessão anterior (preferências salvas, último paciente buscado); cada acesso à tela inicial começa neutro.
- **Digitação sob pressão de fila.** CPF e data são campos digitados repetidamente, muitas vezes com o paciente esperando ao balcão. O erro mais provável aqui não é dificuldade de visão ou de navegação, é erro de digitação seguido de não notar o erro a tempo. Implicação de design: validação de formato em tempo real (CPF com máscara, data com máscara), e mensagens de erro que aparecem perto do campo errado, não só no topo da tela ou em um alerta genérico.
- **Variação de infraestrutura entre unidades.** UBS do DF têm equipamentos de idades e capacidades diferentes. Implicação de design: a interface não deve depender de conexão de internet contínua para operar (ver seção 7, a lógica roda inteiramente no navegador depois do carregamento inicial), e deve funcionar em resoluções de tela menores sem quebrar layout.

### 2.5 Implicações de design consolidadas

- Poucos cliques por tarefa: o fluxo de agendamento completo deve caber em uma sequência curta de telas, sem etapas redundantes.
- Tolerância a interrupção: o estado de um formulário em andamento não se perde se a atendente for interrompida e voltar à mesma tela.
- Hierarquia visual clara: a ação primária de cada tela é visualmente dominante.
- Texto e contraste altos, dado o uso prolongado ao longo do turno.
- Feedback imediato e inequívoco para toda ação (cadastro, agendamento, cancelamento).
- Validação de formato em tempo real para CPF e data, com erro localizado no campo, não genérico.
- Nenhum estado de sessão anterior interfere na sessão atual (computador compartilhado).
- Visão agregada e somente leitura para o Coordenador, sem misturar com as telas operacionais da Ana.

## 3. Inventário completo de telas

Para que a interface seja 100% funcional, cada opção de menu que existe hoje no terminal precisa de uma tela ou componente equivalente. Esta tabela é a checklist de paridade.

| Área | Funcionalidade (RF) | Tela |
|---|---|---|
| Geral | — | Dashboard / Tela Inicial (contadores + consultas do dia) |
| Pacientes | RF01 | Cadastrar paciente |
| Pacientes | RF02 | Buscar paciente por CPF |
| Pacientes | RF03 | Listar todos os pacientes |
| Médicos | RF04 | Cadastrar médico |
| Médicos | RF05 | Configurar disponibilidade (grid dia × turno) |
| Médicos | RF06 | Listar médicos / buscar por CRM ou especialidade |
| Agendamento | RF07–RF13 | Criar novo agendamento (fluxo multi-etapa) |
| Cancelamento | RF14–RF15 | Cancelar agendamento (por protocolo ou CPF) |
| Agenda | RF09, RF17 | Grade de um médico em uma data |
| Agenda | RF16 | Agenda do dia (todos os médicos) |
| Agenda | RF18, RF03 | Histórico de consultas de um paciente |

Nenhuma tela pode ser puramente decorativa ou "em construção": se uma função existe no terminal, ela precisa existir na interface web chamando a mesma função real em C.

## 4. Fluxos de usuário

### 4.1 Tela Inicial / Dashboard

Cabeçalho fixo com nome do sistema e a UBS. Quatro cartões de indicador no topo: total de pacientes, total de médicos, total de agendamentos, consultas do dia atual. Abaixo, tabela com as próximas consultas do dia (protocolo, paciente, médico, hora, status). Esta tela atende primariamente o Coordenador (visão geral) e serve de ponto de partida para a Ana.

### 4.2 Fluxo — Novo Agendamento

```
[Buscar paciente por CPF]
    → encontrado → segue
    → não encontrado → [Cadastro rápido de paciente] → segue
[Selecionar especialidade] (cards clicáveis, as 5 opções fixas do sistema)
[Selecionar médico] (lista filtrada pela especialidade, mostrando CRM e nome)
[Informar data] → sistema busca/cria a grade e mostra os slots
[Selecionar slot livre] (grade Manhã/Tarde, mesmo padrão visual da seção 6)
[Preencher queixa principal] (até 200 caracteres, contador visível, obrigatório)
[Tela de Confirmação] (resumo completo + protocolo provisório)
    → Confirmar → [Tela de Sucesso] (protocolo em destaque)
    → Cancelar → volta sem gravar nada
```

Nota de implementação: a ordem aqui (queixa antes da confirmação, depois do slot) é uma escolha de UX, não uma exigência da lógica em C — o core só exige que todos os campos estejam presentes no momento da chamada de `agendamento_criar`. Pode-se mover a queixa para mais cedo no fluxo sem qualquer mudança na integração.

### 4.3 Fluxo — Cancelamento

```
[Escolher tipo de busca: Protocolo ou CPF]
    → Protocolo → [Localiza direto]
    → CPF → [Lista agendamentos ativos do paciente] → [Selecionar um]
[Tela de Confirmação] (dados completos do agendamento)
    → Confirmar → [Tela de Sucesso] (slot liberado)
```

### 4.4 Fluxo — Cadastro e Disponibilidade de Médico

```
[Cadastrar médico] (CRM, nome, especialidade via menu fixo)
    → sucesso → [Configurar Disponibilidade]
[Grid interativo 7 dias × 2 turnos] (clique alterna Disponível/Indisponível, sem necessidade de "salvar" separado — cada clique já persiste em memória)
```

### 4.5 Fluxo — Consulta de Agenda (Ana e Coordenador)

```
[Dashboard]
    → "Ver grade de um médico" → [Informar CRM + data] → [Grade do médico]
    → "Agenda do dia" → [Informar data] → [Lista de todos os médicos com seus slots e queixas]
    → "Histórico de paciente" → [Informar CPF] → [Tabela com todas as consultas, ativas e canceladas]
```

## 5. Design System

### 5.1 Princípios

- Clareza acima de densidade: cada tela mostra o necessário para a tarefa atual.
- Ação primária sempre visível e visualmente dominante.
- Estados explícitos (livre, ocupado, cancelado, confirmado) com cor e texto/ícone juntos, nunca cor isolada.

### 5.2 Paleta de cores

| Token | Cor | Uso |
|---|---|---|
| `--color-primary` | `#1F5C99` | Botões primários, cabeçalhos, navegação ativa |
| `--color-primary-hover` | `#174A7A` | Hover/pressed do botão primário |
| `--color-secondary` | `#2E9E6B` | Disponibilidade, confirmações de sucesso |
| `--color-danger` | `#C0392B` | Cancelamento, slots ocupados, erros |
| `--color-warning` | `#D69E2E` | Avisos não bloqueantes |
| `--color-background` | `#F7F9FB` | Fundo geral |
| `--color-surface` | `#FFFFFF` | Cartões e formulários |
| `--color-text-primary` | `#1A1A1A` | Texto principal |
| `--color-text-secondary` | `#5C6770` | Texto auxiliar |
| `--color-border` | `#D9E1E7` | Bordas e divisores |

Vermelho reservado exclusivamente para ações destrutivas ou erro, nunca decorativo.

### 5.3 Tipografia

| Token | Especificação | Uso |
|---|---|---|
| `--font-family` | `'Inter', system-ui, sans-serif` | Fonte única |
| `--font-size-display` | 28px / bold | Título de tela |
| `--font-size-heading` | 20px / semibold | Subtítulos |
| `--font-size-body` | 16px / regular | Texto padrão (mínimo aceitável para uso prolongado) |
| `--font-size-label` | 14px / medium | Labels de campo |
| `--font-size-caption` | 12px / regular | Metadados |

### 5.4 Espaçamento

Grid de 8px: `--space-xs` 4px, `--space-sm` 8px, `--space-md` 16px, `--space-lg` 24px, `--space-xl` 40px.

### 5.5 Componentes principais

- **Botão primário**: fundo `--color-primary`, texto branco, altura mínima 48px, cantos arredondados 8px. Justificativa de altura: área de toque confortável mesmo sob pressão de tempo (seção 2.4).
- **Botão secundário**: fundo transparente, borda `--color-border`.
- **Card de slot**: livre = fundo `--color-surface` + borda `--color-secondary`, clicável; ocupado = fundo `#F0F0F0`, nome do paciente truncado, não clicável; selecionado = fundo `--color-primary`, texto branco.
- **Campo de texto**: altura mínima 44px, label sempre visível acima do campo (nunca só placeholder). Campo de CPF e data com máscara de formatação em tempo real e validação inline (erro aparece junto ao campo, conforme seção 2.4). Campo de queixa: `textarea` com contador 0/200 visível.
- **Badge de status**: Disponível (`--color-secondary` 15% opacidade), Ocupado (`--color-text-secondary` 15%), Cancelado (`--color-danger` 15%).
- **Toast de confirmação**: canto superior direito, verde para sucesso, vermelho para erro, desaparece em 4s ou ao clicar.
- **Grid de disponibilidade**: tabela 7×2 com cada célula clicável alternando S/N visualmente (ex.: check verde / traço cinza), sem necessidade de botão "salvar" — cada clique já reflete o estado real em memória.

### 5.6 Ícones

Conjunto único (Lucide ou Material Symbols): lupa (buscar), usuário (paciente), calendário (agenda/data), relógio (horário/slot), check circular (confirmação), X circular (cancelamento), alerta triangular (erro de validação).

## 6. Acessibilidade e usabilidade

- Contraste mínimo 4.5:1 entre texto e fundo (WCAG AA).
- Área de toque mínima 44×44px em todos os elementos interativos.
- Estados de foco visíveis (outline) em navegação por teclado.
- Nenhuma informação crítica transmitida só por cor.
- Validação de formato em tempo real para CPF e data, com mensagem de erro posicionada junto ao campo (não em alerta genérico no topo), conforme a dificuldade contextual de digitação sob pressão (seção 2.4).
- Nenhuma tela depende de estado salvo de uma sessão anterior do navegador (computador compartilhado entre turnos, seção 2.4).
- Layout responsivo a partir de telas pequenas, sem depender de internet contínua após o carregamento inicial (variação de infraestrutura entre UBS, seção 2.4).

## 7. Arquitetura de integração com o sistema em C

### 7.1 Princípio

A interface web não reimplementa nenhuma regra de negócio. Os arquivos `paciente.c`, `medico.c`, `slot.c`, `agendamento.c` e `agenda.c` (camada core, sem I/O de terminal) são compilados para WebAssembly via Emscripten. O frontend em HTML/CSS/JavaScript chama essas funções reais através de uma camada de ponte fina, `bridge_wasm.c`, que apenas traduz parâmetros e formata o retorno como JSON. Nenhuma lógica nova é escrita em JavaScript além de exibição e navegação entre telas.

### 7.2 Modelo de execução

Aplicação de página única (SPA simples): um único HTML carrega o módulo `.wasm` uma vez; trocar de "tela" significa mostrar/esconder seções da mesma página via JavaScript, nunca recarregar a página. Isso preserva o estado em memória do módulo WASM durante toda a sessão, exatamente como o terminal mantém estado em memória durante a execução do programa. Não há backend com lógica própria: um servidor de arquivos estático (ex.: `python -m http.server`) só serve os arquivos para o navegador carregar; toda a lógica roda no cliente.

### 7.3 Contrato de dados

Cada função de ponte recebe parâmetros simples (inteiros, strings) e retorna uma string JSON, escrita em um buffer estático interno ao módulo (sem `malloc`/`free` do lado C; o JavaScript lê a string imediatamente após cada chamada, antes da próxima). O JSON é montado manualmente com `snprintf`, sem biblioteca externa, mantendo a RNF07.

### 7.4 Funções de ponte necessárias

| Função em `bridge_wasm.c` | Core C que chama | Retorno (JSON) |
|---|---|---|
| `bridge_paciente_cadastrar` | `paciente_registrar` | `{ok, idx, erro}` |
| `bridge_paciente_buscar_cpf` | `buscar_paciente_cpf` + `paciente_obter` | dados do paciente ou `{encontrado:false}` |
| `bridge_paciente_listar` | itera `pacientes[]` | array de pacientes |
| `bridge_medico_cadastrar` | `medico_registrar` | `{ok, idx, erro}` |
| `bridge_medico_listar` | itera `medicos[]` | array de médicos |
| `bridge_medico_buscar_crm` | `buscar_medico_crm` + `medico_obter` | dados do médico |
| `bridge_medico_buscar_especialidade` | `buscar_medicos_especialidade` | array de médicos |
| `bridge_medico_disponibilidade_obter` | leitura direta de `medicos[idx].disponibilidade` | tabela 7×2 |
| `bridge_medico_disponibilidade_alternar` | `medico_alternar_disponibilidade` (nova, ver 7.5) | tabela 7×2 atualizada |
| `bridge_grade_obter_ou_criar` | `grade_buscar` / `grade_criar` + `grade_obter` | lista de slots com hora/status/paciente |
| `bridge_agendamento_criar` | `agendamento_criar` | `{ok, protocolo, erro}` |
| `bridge_agendamento_cancelar` | `agendamento_cancelar` | `{ok, erro}` |
| `bridge_agendamento_buscar_protocolo` | `buscar_agendamento_protocolo` + `agendamento_obter` | dados do agendamento |
| `bridge_agendamento_listar_paciente` | `buscar_agendamentos_paciente` + `agendamento_obter` | array de agendamentos |
| `bridge_agenda_dia` | itera médicos + `grade_buscar/criar` + `agendamento_obter` | array de médicos com seus slots e queixas |
| `bridge_especialidades_listar` | constante `ESPECIALIDADES` de `medico.h` | array de 5 strings |
| `bridge_contadores` | `num_pacientes`, `num_medicos`, `num_agendamentos` + contagem de agendamentos do dia atual | `{pacientes, medicos, agendamentos, consultas_hoje}` |

### 7.5 Único ajuste necessário no core C existente

A lógica de alternar disponibilidade (`disponibilidade[dia][turno] = !disponibilidade[dia][turno]`) hoje vive embutida dentro de `medico_configurar_disponibilidade_terminal`, não como função própria. Para o bridge reutilizar exatamente a mesma lógica sem duplicar nem acessar o array global diretamente, extrair uma função nova:

```c
/* Alterna o status de disponibilidade (S/N) de um dia/turno especifico
 * do medico no indice dado. Retorna 1 em sucesso, 0 se os indices
 * forem invalidos. */
int medico_alternar_disponibilidade(int idx_medico, int dia, int turno);
```

`medico_configurar_disponibilidade_terminal` passa a chamar essa função internamente, em vez de alterar o array direto — isso é o único ponto em que o core C ganha uma função nova por causa do frontend, e é estritamente uma extração, não uma reescrita de comportamento.

### 7.6 Build

Comando de compilação para WebAssembly (a incluir como alvo `make wasm` no Makefile, separado do alvo `make` do terminal, que continua existindo e inalterado):

```
emcc src/paciente.c src/medico.c src/slot.c src/agendamento.c src/agenda.c src/bridge_wasm.c \
  -Iinclude -O2 \
  -s EXPORTED_FUNCTIONS="['_bridge_paciente_cadastrar', '_bridge_paciente_buscar_cpf', ...]" \
  -s EXPORTED_RUNTIME_METHODS="['ccall','cwrap']" \
  -s MODULARIZE=1 -s EXPORT_NAME="SusAgendaModule" \
  -o web/sus-agenda.js
```

`main.c` e `menu.c` não entram nessa compilação, porque pertencem à camada de terminal, não ao core.

## 8. Stack de implementação e fora de escopo

A interface gráfica é implementada compilando os arquivos C reais (camada core) para WebAssembly via Emscripten, sem reescrever a lógica de negócio em nenhuma outra linguagem. O frontend roda inteiramente no navegador, sem servidor de backend com lógica própria. A versão em terminal compilada diretamente em C continua sendo o artefato avaliado na disciplina de Algoritmos e Lógica de Programação. Esta interface web é entrega de contingência/bônus, não bloqueia o prazo de entrega acadêmica, e deve ser construída em branch separada do código já entregue.

Fora de escopo desta primeira versão: persistência entre sessões do navegador (o sistema continua perdendo dados ao fechar a aba, exatamente como o terminal perde ao fechar o programa), autenticação de usuário, suporte a múltiplos atendentes simultâneos.
