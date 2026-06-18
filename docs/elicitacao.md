# Documento de Elicitação de Requisitos — SUS-Agenda DF

## 1. Visão Geral

Este documento consolida o levantamento de requisitos do SUS-Agenda DF através da técnica 5W2H, complementado por público-alvo, stakeholders e estimativa de custo computacional. Serve como base de alinhamento entre o grupo e o professor antes da implementação das fases finais do sistema.

## 2. 5W2H

| Pergunta | Resposta |
|---|---|
| **What** (O quê) | Sistema de terminal em linguagem C para gerenciar o ciclo completo de agendamento de consultas em Unidades Básicas de Saúde (UBS) do Distrito Federal: cadastro de pacientes, cadastro de médicos, agendamento com registro de queixa principal, cancelamento e consultas de agenda. |
| **Why** (Por quê) | Atendentes de UBS marcam consultas hoje sem registrar o motivo da visita, e o médico chega ao atendimento sem contexto clínico prévio. Não há histórico consolidado do paciente acessível na recepção. O sistema resolve essa lacuna de gestão interna, tornando o fluxo de atendimento mais organizado e eficiente. |
| **Who** (Quem) | Usuário operador: atendente da recepção da UBS. Beneficiários indiretos: médico e enfermeiro da equipe de Saúde da Família, coordenador da UBS, paciente. Desenvolvimento: aluno responsável pela implementação técnica, com apoio do grupo nas demais entregas do trabalho. |
| **Where** (Onde) | Unidades Básicas de Saúde do Distrito Federal, operando localmente no terminal do computador da recepção. Não depende de conexão com internet ou infraestrutura externa na versão atual. |
| **When** (Quando) | Desenvolvimento entre maio e 25 de junho de 2026, dividido em quatro fases incrementais. Em operação, o sistema é utilizado durante o horário de funcionamento da UBS (07h00 às 17h30). |
| **How** (Como) | Implementação em C, com código organizado em módulos por domínio (paciente, médico, agendamento, agenda, menu), cada um com arquivo de implementação `.c` e interface `.h`. Entidades se relacionam por índices inteiros. Interface gráfica web planejada como evolução futura, documentada separadamente. |
| **How much** (Quanto custa) | Ver seção 5 (Custo Computacional). Em termos de esforço de desenvolvimento, o projeto é de baixo custo: não exige hardware especializado, banco de dados externo, ou licenças de software. O custo real é o tempo de desenvolvimento dentro do cronograma acadêmico. |

## 3. Público-Alvo

| Perfil | Descrição | Relação com o sistema |
|---|---|---|
| Atendente de recepção | Profissional administrativo da UBS, responsável por organizar a fila de atendimento e operar o agendamento | Usuário direto e único operador do sistema |
| Médico de família | Profissional da equipe de Saúde da Família (eSF), atende consultas eletivas da UBS | Beneficiário indireto: recebe a agenda do dia com a queixa do paciente já registrada |
| Enfermeiro | Membro da eSF, participa do cuidado e acompanhamento do paciente | Beneficiário indireto, mesmo benefício do médico |
| Coordenador da UBS | Responsável pela gestão administrativa da unidade | Consulta a agenda consolidada e o histórico de pacientes para fins de gestão |
| Paciente | Usuário final do serviço de saúde, não interage diretamente com o sistema | Beneficiário indireto: recebe atendimento mais preparado e tem seu histórico preservado |

## 4. Stakeholders

| Tipo | Participante | Papel no projeto |
|---|---|---|
| Usuário final | Atendente da recepção | Opera o sistema no dia a dia: cadastra pacientes, registra queixas, marca e cancela consultas |
| Beneficiário indireto | Médico e enfermeiro (eSF) | Recebem a agenda do dia com queixa registrada por paciente |
| Beneficiário indireto | Paciente da UBS | Tem atendimento mais preparado e histórico preservado |
| Gestor | Coordenador da UBS | Consulta agenda consolidada e histórico por paciente |
| Patrocinador acadêmico | Prof. Murillo Edson de Carvalho Souza | Define requisitos acadêmicos da disciplina e avalia o projeto |
| Equipe de desenvolvimento | Aluno responsável pela implementação técnica | Lidera o desenvolvimento do sistema em C e a documentação técnica |
| Equipe do grupo | Demais integrantes do grupo de trabalho | Responsáveis por entregas não técnicas do trabalho (apresentação, planos complementares) |

## 5. Custo Computacional

O SUS-Agenda DF é um sistema de baixa complexidade computacional, compatível com o escopo de um trabalho acadêmico em C executado localmente.

### 5.1 Estruturas de dados e complexidade

| Operação | Estrutura usada | Complexidade |
|---|---|---|
| Cadastro de paciente | Inserção em array (busca de duplicidade por CPF antes) | O(n), onde n é o número de pacientes cadastrados |
| Busca de paciente por CPF | Busca linear em array | O(n) |
| Cadastro de médico | Inserção em array (busca de duplicidade por CRM) | O(n) |
| Geração da grade de slots | Preenchimento de array fixo (18 posições por médico/dia) | O(1), tamanho constante por grade |
| Verificação de conflito de horário | Checagem direta de posição no array de slots | O(1) |
| Verificação de agendamento duplicado no mesmo dia | Busca linear nos agendamentos do paciente | O(m), onde m é o número de agendamentos do paciente |
| Listagem de agenda do dia | Iteração sobre médicos x slots | O(d × 18), onde d é o número de médicos |

Nenhuma operação do sistema excede complexidade linear em relação ao volume de dados esperado (uma UBS individual, com dezenas de médicos e até algumas centenas de pacientes ativos). Não há necessidade de estruturas de dados mais sofisticadas (árvores, tabelas hash) para o volume de uma unidade isolada.

### 5.2 Uso de memória

Todas as entidades (pacientes, médicos, agendamentos) são mantidas em arrays estáticos de tamanho fixo, definidos em tempo de compilação (`MAX_PACIENTES`, `MAX_MEDICOS`, `MAX_AGENDAMENTOS`). O consumo de memória é previsível e baixo, da ordem de poucos megabytes mesmo no limite máximo de capacidade, compatível com qualquer computador usado em recepção de UBS.

### 5.3 Requisitos de hardware e ambiente

| Recurso | Requisito |
|---|---|
| Processador | Qualquer CPU moderna, sem exigência de processamento paralelo ou GPU |
| Memória RAM | Inferior a 50 MB em uso típico |
| Armazenamento | Inferior a 5 MB para o executável e dados persistidos em arquivo |
| Sistema operacional | Linux, Windows ou macOS, com compilador C compatível (gcc) |
| Conectividade | Não exigida na versão em C/terminal. A versão futura com interface web exigirá rede local ou acesso à internet, conforme hospedagem escolhida |

### 5.4 Evolução futura

Caso o sistema evolua para atender múltiplas UBS simultaneamente (fora do escopo atual), o custo computacional mudaria de ordem: seria necessário um banco de dados centralizado e replicação de carga entre unidades, o que sai do escopo deste trabalho acadêmico mas é mencionado aqui para contextualizar os limites da solução atual.
