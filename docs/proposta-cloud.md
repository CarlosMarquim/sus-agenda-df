# Proposta de Arquitetura Cloud — SUS-Agenda DF

## 1. Contexto

O SUS-Agenda DF, em sua versão atual, opera integralmente em memória local: os dados vivem no processo C em execução no terminal ou no módulo WebAssembly carregado no navegador, sem persistência entre sessões. Essa arquitetura atende ao escopo do trabalho acadêmico, mas apresenta uma limitação funcional clara para uso real em UBS: os agendamentos, pacientes e médicos cadastrados se perdem ao fechar o sistema.

Esta proposta descreve uma evolução arquitetural possível do sistema para um ambiente de nuvem, mantendo o princípio de que nenhuma regra de negócio seria reimplementada em camada externa — apenas a persistência e o acesso multiusuário seriam adicionados.

---

## 2. Problema que a Cloud resolveria

| Limitação atual | Impacto em ambiente real de UBS |
|---|---|
| Dados em memória, sem persistência | Todos os registros se perdem ao reiniciar o sistema |
| Execução local por terminal único | Apenas um atendente acessa o sistema por vez, em um único computador |
| Frontend WebAssembly sem backend | Interface web não persiste dados entre sessões ou entre usuários |
| Sem separação de ambientes | Qualquer erro em produção afeta diretamente os dados reais |

---

## 3. Arquitetura proposta

### 3.1 Visão geral

```
[Atendente / Coordenador]
        │
        ▼
  [Frontend Web]          ← HTML/CSS/JS já implementado
  (SPA em CDN)
        │
        ▼
  [API REST]              ← nova camada, substituindo ccall direto ao WASM
  (Node.js ou Go)
        │
        ├── [Banco de dados relacional]    ← PostgreSQL (pacientes, médicos, agendamentos)
        │
        └── [Autenticação]                ← JWT por sessão de atendente
```

### 3.2 Componentes e responsabilidades

| Componente | Tecnologia sugerida | Função |
|---|---|---|
| Frontend | HTML/CSS/JS (já implementado) em CDN | Interface de interação do atendente |
| API REST | Node.js com Express, ou Go | Recebe requisições do frontend, aplica regras de negócio, persiste dados |
| Banco de dados | PostgreSQL (gerenciado, ex: Amazon RDS ou Supabase) | Persistência de Paciente, Médico, Agendamento |
| Autenticação | JWT com expiração de sessão | Controla acesso por atendente identificado |
| Infraestrutura | AWS, GCP ou Azure (camada gratuita suficiente para escala de UBS) | Hospedagem da API e banco |
| CDN | Vercel ou Netlify | Entrega do frontend estático |

---

## 4. Modelo de dados em banco relacional

As structs C já definem o esquema com clareza. A migração para SQL seria direta:

```sql
-- Paciente
CREATE TABLE paciente (
  id         SERIAL PRIMARY KEY,
  nome       VARCHAR(100) NOT NULL,
  cpf        CHAR(11) UNIQUE NOT NULL,
  nascimento DATE NOT NULL,
  telefone   VARCHAR(16)
);

-- Médico
CREATE TABLE medico (
  id           SERIAL PRIMARY KEY,
  nome         VARCHAR(100) NOT NULL,
  crm          VARCHAR(15) UNIQUE NOT NULL,
  especialidade VARCHAR(50) NOT NULL,
  disponibilidade JSONB   -- equivale ao int disponibilidade[7][2]
);

-- Agendamento
CREATE TABLE agendamento (
  protocolo   SERIAL PRIMARY KEY,
  id_paciente INTEGER REFERENCES paciente(id),
  id_medico   INTEGER REFERENCES medico(id),
  slot        SMALLINT NOT NULL,
  data        DATE NOT NULL,
  queixa      VARCHAR(200) NOT NULL,
  cancelado   BOOLEAN DEFAULT FALSE
);
```

---

## 5. Regras de negócio na Cloud

Nenhuma regra de negócio mudaria. A API seria responsável por aplicar as mesmas validações já implementadas em C:

| Regra atual (C) | Equivalente na API |
|---|---|
| CPF único por paciente | UNIQUE constraint no banco + verificação antes de INSERT |
| Um agendamento por paciente por dia | Query de conflito antes de confirmar |
| Slot deve estar livre | Verificação de sobreposição de slot + data + médico |
| Queixa obrigatória | Validação de campo não nulo no payload da requisição |
| Protocolo sequencial | SERIAL / autoincrement do banco |
| Cancelamento libera slot imediatamente | UPDATE cancelado = TRUE + resposta imediata ao frontend |

---

## 6. Segurança em ambiente Cloud

| Aspecto | Implementação |
|---|---|
| Autenticação | JWT com tempo de expiração (ex: 8h, duração de um turno de UBS) |
| Criptografia em trânsito | HTTPS obrigatório em todos os endpoints |
| Criptografia em repouso | Dados do banco criptografados via recurso nativo do provedor (RDS, Supabase) |
| LGPD | CPF, data de nascimento e queixa clínica classificados como dados sensíveis de saúde; acesso restrito por papel (atendente vs. coordenador); logs de acesso ativados |
| Backups | Backup automático diário do banco, retenção mínima de 30 dias |
| Ambiente de produção separado | Ambientes de desenvolvimento e produção isolados; dados reais nunca acessados em desenvolvimento |

---

## 7. Escala esperada para uma UBS do DF

Uma UBS de porte médio opera com 2 a 4 médicos por turno, 18 slots por médico por dia (9 manhã + 9 tarde) e em torno de 30 a 80 agendamentos diários. O volume de dados e de requisições é muito baixo para qualquer infraestrutura cloud moderna.

| Métrica | Estimativa |
|---|---|
| Agendamentos por dia | 30–80 |
| Registros de pacientes (acumulado em 1 ano) | 500–2.000 |
| Requisições à API por turno | < 500 |
| Tamanho do banco após 1 ano | < 50 MB |

Essa escala é atendida confortavelmente pela camada gratuita de provedores como Supabase (banco PostgreSQL gerenciado) e Vercel (frontend). O custo mensal de infraestrutura para uma UBS única seria próximo de zero na fase inicial.

---

## 8. Roadmap de evolução (pós-MVP acadêmico)

| Etapa | Entrega | Pré-requisito |
|---|---|---|
| **v1.1** | Persistência via API REST + PostgreSQL | Implementar API com os endpoints de CRUD já mapeados pelas funções C |
| **v1.2** | Autenticação por atendente (login/sessão) | API com módulo de autenticação JWT |
| **v1.3** | Deploy em nuvem (frontend em CDN, API e banco gerenciados) | v1.1 e v1.2 concluídas |
| **v2.0** | Multi-UBS (uma instância por unidade, isolamento de dados) | Modelo de tenant por schema no banco |
| **v2.1** | Notificação de lembrete de consulta por SMS ou e-mail | Integração com serviço de mensageria (ex: Twilio, Amazon SNS) |
| **v3.0** | Integração com sistemas do GDF (SIGAF, e-SUS) | Depende de API pública do GDF — fora do controle do projeto |

---

*SUS-Agenda DF | IDP | ALP 2026.1*
