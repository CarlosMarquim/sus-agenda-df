#ifndef SLOT_H
#define SLOT_H

/*
 * Modulo Slot/Grade - Fase 2 do SUS-Agenda DF
 *
 * Mesma convencao de camadas da Fase 1 (core / terminal / persistencia
 * stub). Ver paciente.h/paciente.c como referencia.
 */

/* Capacidade maxima de slots por grade.
 *
 * NOTA DE CONSISTENCIA: o enunciado original previa capacidade 16
 * ("suficiente para 8h em slots de 30min"), mas a geracao automatica
 * pedida para o turno "ambos" e manha (9 slots) + tarde (9 slots) =
 * 18 slots. Com capacidade 16 isso causaria estouro de array (escrita
 * fora dos limites) ao gerar a grade de um medico de turno ambos.
 * Por isso a capacidade real usada aqui e 18 -- exatamente o que o
 * turno "ambos" precisa, sem folga extra. */
#define MAX_SLOTS_GRADE 18

#define MAX_GRADES 200

typedef struct {
    char hora[6];          /* "HH:MM" + terminador nulo */
    int ocupado;            /* 0 = livre, 1 = ocupado */
    int paciente_idx;       /* indice do paciente que ocupa o slot, -1 se livre */

    /* RESERVADO para a Fase 3 (modulo agendamento): indice do
     * agendamento vinculado a este slot. -1 significa "sem
     * agendamento" (slot livre ou ocupado sem vinculo formal ainda). */
    int agendamento_idx;
} Slot;

typedef struct {
    int medico_idx;                  /* indice do medico em medicos[] */
    char data[11];                    /* "DD/MM/AAAA" + terminador nulo */
    Slot slots[MAX_SLOTS_GRADE];
    int num_slots;
} GradeDia;

extern GradeDia grades[MAX_GRADES];
extern int num_grades;

/* ===================== CAMADA CORE (sem I/O) ===================== */

/* Cria a grade do dia de um medico, gerando os slots automaticamente
 * conforme a disponibilidade do medico (Medico.disponibilidade) no
 * dia da semana correspondente a 'data':
 *   disponibilidade[dia][TURNO_MANHA] == 1 -> gera 07:00 as 11:30 (9 slots)
 *   disponibilidade[dia][TURNO_TARDE] == 1 -> gera 13:00 as 17:30 (9 slots)
 * Se o medico nao estiver disponivel em nenhum dos dois turnos
 * naquele dia, a grade e criada com 0 slots (sem erro). Retorna o
 * indice da grade criada, ou -1 se o indice do medico for invalido,
 * a data for vazia/invalida, se ja existir grade para esse medico
 * nessa data, ou se a capacidade maxima de grades (MAX_GRADES) tiver
 * sido atingida. */
int grade_criar(int medico_idx, const char *data);

/* Busca a grade de um medico numa data especifica. Retorna o indice
 * ou -1 se nao encontrada. */
int grade_buscar(int medico_idx, const char *data);

/* Retorna um ponteiro somente-leitura para a grade no indice dado,
 * ou NULL se o indice estiver fora do intervalo valido. */
const GradeDia *grade_obter(int idx);

/* Marca um slot como ocupado por um paciente. Retorna 0 em sucesso, -1
 * se os indices forem invalidos ou se o slot ja estiver ocupado
 * (nunca sobrescreve um slot ja ocupado). */
int slot_ocupar(int grade_idx, int slot_idx, int paciente_idx);

/* Libera um slot previamente ocupado. Retorna 0 em sucesso, -1 se os
 * indices forem invalidos ou se o slot ja estiver livre. */
int slot_liberar(int grade_idx, int slot_idx);

/* Vincula um agendamento a um slot, gravando agendamento_idx. Nao
 * altera o campo 'ocupado' (isso e responsabilidade de slot_ocupar,
 * que deve ser chamada separadamente). Retorna 0 em sucesso, -1 se
 * os indices forem invalidos. */
int slot_vincular_agendamento(int grade_idx, int slot_idx, int agendamento_idx);

/* ==================== CAMADA TERMINAL (com I/O) ==================== */

/* Exibe no terminal a grade do dia indicada: medico, data, e cada
 * slot identificado por horario, status (livre/ocupado) e o nome do
 * paciente, quando ocupado. */
void grade_exibir_terminal(int grade_idx);

/* Exibe a grade e pede ao atendente que escolha um slot livre.
 * Escreve o indice do slot escolhido em *slot_escolhido, ou -1 se a
 * operacao for cancelada, o indice for invalido, ou o slot escolhido
 * estiver ocupado. Reservada para uso pelo fluxo de agendamento
 * (Fase 3); nao e chamada pelo menu nesta fase. */
void grade_selecionar_slot_terminal(int grade_idx, int *slot_escolhido);

/* =================== CAMADA DE PERSISTENCIA (stub) =================== */

/* Carrega o estado inicial de grades. Stub da Fase 2: comeca vazio. */
int grade_carregar(void);

/* Persiste o estado atual de grades. Stub da Fase 2: nao escreve em
 * nenhum arquivo, apenas sinaliza sucesso. */
int grade_salvar(void);

#endif /* SLOT_H */
