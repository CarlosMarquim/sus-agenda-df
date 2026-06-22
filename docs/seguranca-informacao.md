# Plano de Segurança da Informação — SUS-Agenda DF

## 1. Objetivo

Este documento descreve as medidas de segurança da informação adotadas no SUS-Agenda DF, considerando que o sistema manipula dados pessoais e dados sensíveis de saúde (CPF, data de nascimento, telefone e queixa clínica), sujeitos aos princípios da Lei Geral de Proteção de Dados (LGPD).

## 2. Classificação dos Dados Tratados

| Dado | Classificação | Justificativa |
|---|---|---|
| Nome completo | Dado pessoal | Identifica diretamente a pessoa |
| CPF | Dado pessoal, identificador único | Permite cruzamento com outras bases |
| Data de nascimento | Dado pessoal | Combinado com CPF, aumenta risco de identificação |
| Telefone | Dado pessoal | Contato direto com o titular |
| Queixa principal | Dado sensível de saúde | Art. 5º, II da LGPD classifica dado de saúde como sensível |
| Histórico de consultas | Dado sensível de saúde | Revela padrão de saúde do paciente ao longo do tempo |

## 3. Controle de Acesso

O sistema é operado exclusivamente via terminal local da recepção da UBS, sem exposição em rede na versão atual. Isso reduz a superfície de ataque a praticamente zero em termos de acesso remoto: um agente externo só acessaria os dados com acesso físico ao computador da recepção.

| Aspecto | Implementação prevista |
|---|---|
| Acesso ao sistema | Restrito ao terminal local da recepção, sem login de usuário na versão atual (escopo acadêmico) |
| Acesso aos dados | Apenas através das funções do sistema, sem edição direta de arquivos de dados por usuários finais |
| Interface web (implementada) | A SPA roda inteiramente no cliente via WebAssembly — sem servidor backend e sem transmissão de dados pela rede. Os dados vivem em memória do módulo WASM e são perdidos ao fechar a aba, comportamento equivalente ao terminal. Uma versão com persistência em servidor exigiria autenticação, HTTPS e controle de sessão antes de qualquer exposição em rede. |

## 4. Integridade dos Dados

| Mecanismo | Descrição |
|---|---|
| Validação de CPF | Verificação de formato (11 dígitos numéricos) antes de aceitar cadastro ou busca |
| Validação de data | Verificação de formato DD/MM/AAAA em todas as entradas de data |
| Identificador único | CPF é validado como único antes de permitir novo cadastro, prevenindo duplicidade e inconsistência |
| Protocolo imutável | Número de protocolo gerado sequencialmente pelo sistema, sem possibilidade de edição manual, garantindo rastreabilidade |
| Validação de conflito | Nenhum agendamento é gravado sem verificação prévia de disponibilidade do slot, prevenindo sobrescrita de dados |

## 5. Auditoria e Rastreabilidade

O número de protocolo sequencial funciona como mecanismo básico de auditoria: cada agendamento criado no sistema é identificável de forma única e ordenada no tempo, permitindo reconstruir a sequência de operações caso necessário.

Na versão de terminal, não há log de eventos separado do registro de dados em si, dado o escopo acadêmico do projeto. A evolução natural deste mecanismo, fora do escopo atual, seria um log de auditoria separado, registrando quem realizou cada operação (relevante quando o sistema passar a ter múltiplos atendentes autenticados).

## 6. Cancelamento Seguro

O cancelamento de uma consulta exige que o atendente busque explicitamente pelo número de protocolo ou CPF do paciente, visualize a consulta antes de confirmar, e confirme a ação antes de qualquer dado ser alterado. Não existe cancelamento automático ou silencioso. Essa etapa de confirmação evita perda acidental de dados de agendamento.

## 7. Conformidade com a LGPD

O sistema observa os seguintes princípios da LGPD, mesmo operando em escopo acadêmico:

| Princípio (LGPD) | Aplicação no sistema |
|---|---|
| Finalidade | Os dados coletados (nome, CPF, data de nascimento, telefone, queixa) têm finalidade exclusiva de viabilizar o agendamento e o atendimento clínico, sem uso secundário |
| Necessidade | Apenas os campos estritamente necessários ao agendamento são coletados; não há coleta de dados adicionais sem propósito definido |
| Minimização de exposição | O sistema opera localmente, sem transmissão de dados pela rede na versão em C/terminal |
| Segurança | Validações de integridade e controle de acesso restrito ao terminal local, conforme detalhado nas seções 3 e 4 |
| Transparência | O paciente é informado do número de protocolo gerado, dando a ele um meio de rastrear e, se necessário, solicitar o cancelamento de seu próprio agendamento |

## 8. Riscos de Segurança e Mitigação

| Risco | Probabilidade | Impacto | Mitigação |
|---|---|---|---|
| Acesso físico não autorizado ao terminal | Baixa (ambiente controlado de recepção) | Alto (exposição de dados de saúde) | Restringir acesso físico ao computador da recepção; fora do escopo de código, é controle administrativo da UBS |
| Perda de dados por encerramento do programa (modelo em memória) | Alta, se a persistência não for implementada | Médio (perda de histórico entre sessões) | Implementar persistência em arquivo conforme orientação do professor (ver `docs/arquitetura.md`, seção 5) |
| Corrupção de arquivo de persistência | Baixa | Médio | Validação de integridade ao carregar dados na inicialização, com tratamento de erro caso o arquivo esteja corrompido |
| Exposição de dados na interface web atual | Baixa — SPA roda no cliente via WASM, sem backend | Baixo | Dados em memória do módulo WASM, sem transmissão de rede. Risco aumenta apenas se uma versão com servidor for implementada no futuro. |

## 9. Limitações do Escopo Atual

Este plano de segurança reflete o escopo de um sistema acadêmico de terminal, de uso local e single-user. Ele não cobre, porque não se aplica à versão atual, requisitos como: autenticação multifator, criptografia de dados em repouso, segregação de ambientes, ou conformidade com certificações de segurança da informação (ISO 27001 ou similares), que seriam exigidos caso o sistema evoluísse para uso real em produção em múltiplas UBS.
