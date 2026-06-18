#ifndef AGENDAMENTO_H
#define AGENDAMENTO_H

/*
 * Modulo Agendamento - Fase 3 do SUS-Agenda DF
 *
 * Segue a mesma convencao de camadas dos modulos anteriores:
 *   1) CORE: regras de negocio puras, sem I/O.
 *   2) TERMINAL: funcoes que interagem com o atendente via teclado.
 *   3) PERSISTENCIA (stub): carregar/salvar operando em memoria.
 */

#define MAX_AGENDAMENTOS 500

typedef struct {
    int protocolo;
    int idPaciente;   /* indice no array global de pacientes */
    int idMedico;      /* indice no array global de medicos */
    int slot;           /* indice de 0 a 17 na grade do medico */
    char data[11];       /* formato DD/MM/AAAA */
    char queixa[201];     /* texto livre, ate 200 caracteres + terminador nulo */
    int cancelado;          /* 0 = ativo, 1 = cancelado */
} Agendamento;

extern Agendamento agendamentos[MAX_AGENDAMENTOS];
extern int num_agendamentos;
extern int proximo_protocolo;

typedef enum {
    AGENDAMENTO_OK = 0,
    AGENDAMENTO_ERRO_CAPACIDADE_MAXIMA,
    AGENDAMENTO_ERRO_PACIENTE_INVALIDO,
    AGENDAMENTO_ERRO_MEDICO_INVALIDO,
    AGENDAMENTO_ERRO_GRADE_OU_DATA_INVALIDA,
    AGENDAMENTO_ERRO_SLOT_INVALIDO,
    AGENDAMENTO_ERRO_SLOT_OCUPADO,
    AGENDAMENTO_ERRO_QUEIXA_VAZIA,
    AGENDAMENTO_ERRO_CONFLITO_MESMO_DIA,
    AGENDAMENTO_ERRO_PROTOCOLO_NAO_ENCONTRADO,
    AGENDAMENTO_ERRO_JA_CANCELADO
} ResultadoAgendamento;

/* ===================== CAMADA CORE (sem I/O) ===================== */

/* Valida e registra um agendamento. Ocupa o slot na grade e vincula o
 * indice ao historico do paciente. Em sucesso escreve o protocolo gerado
 * em *protocolo_saida (se nao for NULL) e retorna AGENDAMENTO_OK. */
ResultadoAgendamento agendamento_criar(int idPaciente, int idMedico,
                                        const char *data, int slot_idx,
                                        const char *queixa, int *protocolo_saida);

/* Retorna 1 se o paciente ja possui agendamento ativo nessa data, 0
 * caso contrario. Comparacao de data e exata (string DD/MM/AAAA). */
int agendamento_paciente_tem_conflito_dia(int idPaciente, const char *data);

/* Marca o agendamento no indice dado como cancelado e libera o slot
 * correspondente na grade. */
ResultadoAgendamento agendamento_cancelar(int idx_agendamento);

/* Busca linear pelo numero de protocolo. Retorna o indice em
 * agendamentos[] ou -1 se nao encontrado. */
int buscar_agendamento_protocolo(int protocolo);

/* Preenche 'resultados' com os indices de agendamentos do paciente.
 * Se apenas_ativos for 1, filtra apenas os nao cancelados. Retorna a
 * quantidade encontrada (limitada a 'max'). */
int buscar_agendamentos_paciente(int idPaciente, int apenas_ativos,
                                  int *resultados, int max);

/* Retorna ponteiro somente-leitura para o agendamento no indice dado,
 * ou NULL se o indice for invalido. */
const Agendamento *agendamento_obter(int idx);

/* ==================== CAMADA TERMINAL (com I/O) ==================== */

/* Conduz o atendente pelo fluxo completo de criacao de agendamento:
 * busca (ou cadastro) do paciente, selecao de especialidade/medico,
 * coleta de queixa, escolha de data e slot, resumo e confirmacao. */
void agendamento_criar_terminal(void);

/* Conduz o atendente pelo fluxo de cancelamento: busca por protocolo
 * ou por CPF, exibicao dos dados completos e confirmacao explicita. */
void agendamento_cancelar_terminal(void);

/* =================== CAMADA DE PERSISTENCIA (stub) =================== */

/* Zera num_agendamentos e inicializa proximo_protocolo = 1. Stub:
 * nenhum arquivo e lido nesta fase. */
int agendamento_carregar(void);

/* Stub: nao grava nada, apenas sinaliza sucesso. */
int agendamento_salvar(void);

#endif /* AGENDAMENTO_H */
