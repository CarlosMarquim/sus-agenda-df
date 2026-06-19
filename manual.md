# Manual de Uso — SUS-Agenda DF

Este manual descreve como operar o sistema do ponto de vista do atendente da recepção da UBS. Cada seção indica o caminho exato de menus para chegar à funcionalidade descrita.

## Navegação geral

Ao abrir o sistema, o menu principal exibe quatro categorias e a opção de saída:

```
1. Pacientes
2. Medicos
3. Agenda
4. Agendamentos
0. Sair
```

Digite o número da opção desejada e pressione ENTER. Dentro de cada categoria existe um submenu próprio com a opção "0. Voltar" para retornar ao menu principal. Após cada operação, o sistema exibe "Pressione ENTER para continuar..." antes de voltar ao submenu.

---

## 1. Pacientes

Acesse pelo menu principal, opção **1. Pacientes**. O submenu exibe:

```
1. Cadastrar paciente
2. Buscar paciente por CPF
3. Listar todos os pacientes
0. Voltar
```

### 1.1 Cadastrar paciente

Opção **1. Cadastrar paciente**. O sistema pede os seguintes dados, um por vez:

- **CPF**: somente números, exatamente 11 dígitos (sem pontos ou traços). Se o formato estiver errado ou o CPF já estiver cadastrado, o sistema avisa e não cadastra.
- **Nome completo**.
- **Data de nascimento**: formato DD/MM/AAAA. Datas fora desse formato são rejeitadas.
- **Telefone**.

Se todos os campos forem válidos, o sistema confirma o cadastro e exibe um número de protocolo interno.

### 1.2 Buscar paciente por CPF

Opção **2. Buscar paciente por CPF**. Digite o CPF com 11 dígitos. Se encontrado, o sistema exibe os dados completos do paciente (CPF, nome, data de nascimento, telefone e quantidade de agendamentos no histórico). Se não encontrado, exibe mensagem de erro.

### 1.3 Listar todos os pacientes

Opção **3. Listar todos os pacientes**. Exibe uma tabela com protocolo, CPF, nome e data de nascimento de todos os pacientes cadastrados. Se não houver nenhum, avisa que a base está vazia.

---

## 2. Médicos

Acesse pelo menu principal, opção **2. Medicos**. O submenu exibe:

```
1. Cadastrar medico
2. Listar medicos
3. Buscar medico (CRM ou especialidade)
4. Configurar disponibilidade
0. Voltar
```

### 2.1 Cadastrar médico

Opção **1. Cadastrar medico**. O sistema pede:

- **CRM**: texto livre (até 10 caracteres). Não pode ser vazio nem repetido.
- **Nome completo**.
- **Especialidade**: escolhida por menu numerado, não por texto livre. As opções fixas são:
  ```
  1. Clinica Geral
  2. Pediatria
  3. Ginecologia
  4. Ortopedia
  5. Gastroenterologia
  ```

Após o cadastro, o médico começa com disponibilidade zerada em todos os dias e turnos. Para que ele apareça com horários na grade, é necessário configurar a disponibilidade (opção 4 do submenu).

### 2.2 Listar médicos

Opção **2. Listar medicos**. Exibe uma tabela com protocolo, CRM, nome e especialidade de todos os médicos.

### 2.3 Buscar médico

Opção **3. Buscar medico (CRM ou especialidade)**. O sistema pergunta o tipo de busca:

- **Por CRM**: digite o CRM e o sistema exibe os dados do médico, se encontrado.
- **Por especialidade**: digite o nome exato da especialidade (por exemplo, "Clinica Geral") e o sistema lista todos os médicos daquela especialidade.

### 2.4 Configurar disponibilidade

Opção **4. Configurar disponibilidade**. Informe o CRM do médico. O sistema exibe uma tabela com os 7 dias da semana (Domingo a Sabado) e dois turnos (Manha e Tarde), mostrando "S" (disponível) ou "N" (indisponível) em cada célula.

Para alternar o status de um dia/turno, digite o número do dia (0 = Domingo, 1 = Segunda, ..., 6 = Sabado) e depois o turno (0 = Manha, 1 = Tarde). O sistema inverte o valor: se estava "N" vira "S", e vice-versa. Repita quantas vezes quiser e digite **-1** para finalizar. A tabela final é exibida na tela.

---

## 3. Agendamentos

Acesse pelo menu principal, opção **4. Agendamentos**. O submenu exibe:

```
1. Criar novo agendamento
2. Cancelar agendamento
0. Voltar
```

### 3.1 Criar novo agendamento

Opção **1. Criar novo agendamento**. O fluxo é conduzido passo a passo:

1. **CPF do paciente**: digite os 11 dígitos. Se o paciente não estiver cadastrado, o sistema pergunta se deseja cadastrá-lo agora mesmo (opção 1 = Sim). Se escolher cadastrar, o fluxo de cadastro de paciente acontece ali mesmo, e depois o agendamento continua. Se escolher não (opção 0), a operação é cancelada.

2. **Especialidade**: o sistema exibe o menu numerado com as 5 especialidades. Escolha o número correspondente.

3. **Médico**: o sistema lista todos os médicos da especialidade escolhida, mostrando CRM e nome. Digite o CRM do médico desejado. Se o CRM não pertencer à especialidade selecionada, a operação é cancelada.

4. **Queixa principal**: texto livre de até 200 caracteres descrevendo o motivo da consulta. Não pode ser vazio.

5. **Data da consulta**: formato DD/MM/AAAA. O sistema verifica se o médico tem disponibilidade naquele dia da semana.

6. **Escolha do horário**: o sistema exibe a grade do médico para a data informada, com todos os slots numerados e identificados como LIVRE ou OCUPADO. Digite o número do slot livre desejado, ou 0 para cancelar.

7. **Resumo e confirmação**: antes de gravar qualquer coisa, o sistema exibe um resumo completo com nome e CPF do paciente, médico, especialidade, queixa, data, horário e um número de protocolo provisório. Digite 1 para confirmar ou 0 para desistir. Somente após a confirmação o agendamento é registrado e o protocolo passa a valer.

O sistema impede que o mesmo paciente tenha dois agendamentos ativos no mesmo dia, independente de serem médicos ou horários diferentes. Se isso for tentado, a mensagem "este paciente ja possui agendamento ativo nessa data" é exibida após a confirmação.

### 3.2 Cancelar agendamento

Opção **2. Cancelar agendamento**. O sistema oferece dois caminhos de busca:

**Por protocolo** (opção 1): digite o número de protocolo. O sistema localiza o agendamento e exibe todos os dados (protocolo, paciente, médico, especialidade, data, horário, queixa e status). Se estiver ativo, pede confirmação (1 = Sim, 0 = Não). Ao confirmar, o agendamento é marcado como cancelado e o slot é liberado imediatamente na grade do médico.

**Por CPF do paciente** (opção 2): digite o CPF. O sistema lista todos os agendamentos ativos daquele paciente, mostrando protocolo, CRM do médico, data, horário e queixa. Digite o número do protocolo que deseja cancelar. O sistema exibe os dados completos e pede confirmação antes de cancelar.

Se o protocolo não existir ou o agendamento já estiver cancelado, o sistema exibe uma mensagem de erro específica.

---

## 4. Visões de agenda

Acesse pelo menu principal, opção **3. Agenda**. O submenu exibe:

```
1. Exibir grade do dia
2. Agenda do dia (todos os medicos)
3. Historico do paciente
0. Voltar
```

### 4.1 Grade de um médico (opção 1)

Opção **1. Exibir grade do dia**. Informe o CRM do médico e a data (DD/MM/AAAA). O sistema exibe a grade daquele médico naquele dia, com cada slot numerado e identificado:

- Slots livres aparecem como `LIVRE`.
- Slots ocupados aparecem como `OCUPADO` seguido do nome do paciente entre parênteses.

Se o médico não tiver disponibilidade naquele dia da semana, a grade é exibida sem nenhum slot.

### 4.2 Agenda do dia — todos os médicos (opção 2)

Opção **2. Agenda do dia (todos os medicos)**. Informe apenas a data. O sistema percorre todos os médicos cadastrados e exibe, para cada um:

- Cabeçalho com nome, CRM e especialidade.
- Cada slot do dia: se livre, exibe horário e `LIVRE`; se ocupado, exibe horário, nome do paciente e a queixa principal registrada no agendamento.
- Se o médico não tiver disponibilidade naquele dia da semana, exibe "Sem disponibilidade nesse dia da semana" e segue para o próximo.

Essa visão permite ao coordenador da UBS ter uma visão completa de todos os atendimentos programados para o dia.

### 4.3 Histórico do paciente (opção 3)

Opção **3. Historico do paciente**. Informe o CPF do paciente. O sistema exibe uma tabela com todos os agendamentos daquele paciente, incluindo os cancelados:

- Protocolo, CRM do médico, especialidade, data, horário, status (ATIVO ou CANCELADO) e queixa.

Se o paciente não tiver nenhum agendamento, a mensagem "Nenhum agendamento no historico desse paciente" é exibida.

---

## 5. Observações importantes

- **Persistência**: nesta versão, os dados existem apenas em memória enquanto o programa está em execução. Ao encerrar o sistema (opção 0 no menu principal), todos os cadastros, agendamentos e grades são perdidos. Uma versão futura poderá adicionar gravação em arquivo.

- **Especialidades fixas**: as 5 especialidades disponíveis são definidas no sistema e não podem ser alteradas pelo atendente. Tanto o cadastro de médico quanto a seleção no agendamento usam o mesmo menu numerado, garantindo correspondência exata.

- **Conflito por dia**: o sistema permite que um paciente tenha agendamentos em datas diferentes sem restrição. A limitação é apenas de um agendamento ativo por dia. Se o agendamento daquele dia for cancelado, o paciente pode agendar novamente na mesma data.

- **Protocolo sequencial**: o número de protocolo é gerado automaticamente a partir de 1 e avança apenas quando um agendamento é confirmado. Se o atendente desiste na tela de resumo, o número não é consumido, evitando buracos na sequência.
