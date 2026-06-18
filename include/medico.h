#ifndef MEDICO_H
#define MEDICO_H

/*
 * Modulo Medico - Fase 2 do SUS-Agenda DF
 *
 * Segue a mesma convencao de camadas estabelecida na Fase 1
 * (ver paciente.h/paciente.c como referencia):
 *
 *   1) CORE: regras de negocio puras, sem I/O.
 *   2) TERMINAL: funcoes que interagem com o atendente via teclado.
 *   3) PERSISTENCIA (stub): pontos de entrada isolados para
 *      carregar/salvar, operando hoje apenas em memoria.
 */

#define MAX_MEDICOS 20

#define NUM_ESPECIALIDADES 5
extern const char *ESPECIALIDADES[NUM_ESPECIALIDADES];

/* Retorna 1 se a string corresponde exatamente a uma das especialidades
 * fixas do sistema, 0 caso contrario. */
int especialidade_valida(const char *especialidade);

/* Indices da dimensao de turno em Medico.disponibilidade[dia][turno]. */
#define TURNO_MANHA 0
#define TURNO_TARDE 1

typedef struct {
    char crm[11];          /* 10 caracteres + terminador nulo */
    char nome[100];        /* nome completo do medico */
    char especialidade[50];

    /* Disponibilidade por dia da semana e turno.
     * Dimensao 0 (dia da semana): 0=domingo, 1=segunda, ..., 6=sabado.
     * Dimensao 1 (turno): TURNO_MANHA (0) ou TURNO_TARDE (1).
     * Valor 1 = disponivel, 0 = indisponivel.
     * Comeca toda zerada no cadastro; e configurada depois via
     * medico_configurar_disponibilidade_terminal. */
    int disponibilidade[7][2];
} Medico;

extern Medico medicos[MAX_MEDICOS];
extern int num_medicos;

/* ===================== CAMADA CORE (sem I/O) ===================== */

/* Valida e registra um medico a partir de dados ja coletados. A
 * disponibilidade comeca toda zerada (indisponivel em todos os dias e
 * turnos); configure-a depois com
 * medico_configurar_disponibilidade_terminal. Retorna o indice do
 * medico cadastrado, ou -1 se: capacidade maxima atingida, CRM vazio
 * ou duplicado, nome vazio, ou especialidade vazia/so espacos. */
int medico_registrar(const char *crm, const char *nome, const char *especialidade);

/* Busca um medico pelo CRM (busca linear). Retorna o indice ou -1 se
 * nao encontrado. */
int buscar_medico_crm(const char *crm);

/* Busca medicos por especialidade (comparacao exata, sensivel a
 * maiusculas/minusculas). Preenche 'resultados' (capacidade 'max')
 * com os indices encontrados e retorna a quantidade encontrada. */
int buscar_medicos_especialidade(const char *especialidade, int *resultados, int max);

/* Copia os dados do medico no indice 'idx' para *dest. Nao faz nada se
 * 'dest' for NULL ou 'idx' estiver fora do intervalo valido -- o
 * chamador deve garantir um indice valido (ex.: obtido via
 * buscar_medico_crm) antes de chamar esta funcao. */
void medico_obter(int idx, Medico *dest);

/* ==================== CAMADA TERMINAL (com I/O) ==================== */

/* Cadastra um novo medico interagindo via terminal (le os dados do
 * teclado, valida cada campo com mensagens especificas e delega o
 * registro propriamente dito para medico_registrar). */
void medico_cadastrar_terminal(void);

/* Lista todos os medicos cadastrados em formato tabular no terminal. */
void medico_listar_terminal(void);

/* Busca um medico no terminal, por CRM ou por especialidade, e exibe
 * o(s) resultado(s). */
void medico_buscar_terminal(void);

/* Permite ao atendente configurar, via teclado, a disponibilidade
 * (dia da semana x turno) de um medico buscado por CRM. Exibe a
 * tabela atual, alterna o status do dia/turno escolhido a cada
 * iteracao, ate o atendente digitar -1 para sair, e exibe a tabela
 * final. */
void medico_configurar_disponibilidade_terminal(void);

/* =================== CAMADA DE PERSISTENCIA (stub) =================== */

/* Carrega o estado inicial de medicos. Stub da Fase 2: apenas garante
 * que o sistema comeca com a base vazia em memoria. */
int medico_carregar(void);

/* Persiste o estado atual de medicos. Stub da Fase 2: nao escreve em
 * nenhum arquivo, apenas sinaliza sucesso. */
int medico_salvar(void);

#endif /* MEDICO_H */
