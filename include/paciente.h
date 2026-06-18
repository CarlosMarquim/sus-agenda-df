#ifndef PACIENTE_H
#define PACIENTE_H

/*
 * Modulo Paciente - Fase 1 do SUS-Agenda DF
 *
 * O arquivo e dividido em tres camadas:
 *
 *   1) CORE: regras de negocio puras (validacao, cadastro, busca),
 *      sem nenhuma chamada de I/O (printf/scanf/fgets). Esta camada
 *      pode ser reaproveitada por qualquer interface futura (terminal,
 *      web, GUI, testes automatizados) sem nenhuma alteracao.
 *
 *   2) TERMINAL: funcoes que interagem com o atendente via teclado e
 *      delegam as regras de negocio para a camada core acima.
 *
 *   3) PERSISTENCIA (stub): pontos de entrada isolados para carregar e
 *      salvar o estado de pacientes. Nesta fase operam apenas em
 *      memoria (nao ha arquivo ainda); quando a persistencia em
 *      arquivo for implementada, so o CORPO dessas duas funcoes muda,
 *      sem afetar core nem terminal.
 */

#define MAX_PACIENTES 100
#define MAX_HISTORICO 50

typedef struct {
    char cpf[13];                     /* 12 caracteres + terminador nulo (11 digitos numericos) */
    char nome[100];                   /* nome completo do paciente */
    char data_nascimento[11];         /* formato DD/MM/AAAA */
    char telefone[16];                /* ex.: (61)99999-9999 */
    int historico_ids[MAX_HISTORICO]; /* indices de agendamentos do paciente (preenchido na Fase 3) */
    int num_historico;                /* quantidade de agendamentos registrados no historico */

    /* RESERVADO para a Fase 3 (modulo agendamento): indice do agendamento
     * atualmente ativo deste paciente. -1 significa "nenhum agendamento
     * em aberto". Mantido aqui desde a Fase 1 para evitar refatoracao
     * futura da struct. */
    int idx_agendamento_atual;
} Paciente;

extern Paciente pacientes[MAX_PACIENTES];
extern int num_pacientes;

/* Codigos de resultado das operacoes de paciente. Permitem que cada
 * interface (terminal, web, etc.) decida como exibir cada erro, sem
 * a camada core precisar saber nada sobre apresentacao. */
typedef enum {
    PACIENTE_OK = 0,
    PACIENTE_ERRO_CAPACIDADE_MAXIMA,
    PACIENTE_ERRO_CPF_FORMATO_INVALIDO,
    PACIENTE_ERRO_CPF_DUPLICADO,
    PACIENTE_ERRO_NOME_VAZIO,
    PACIENTE_ERRO_DATA_VAZIA,
    PACIENTE_ERRO_TELEFONE_VAZIO
} ResultadoPaciente;

/* ===================== CAMADA CORE (sem I/O) ===================== */

/* Valida e registra um paciente a partir de dados ja coletados (de
 * onde quer que tenham vindo: teclado, formulario web, requisicao
 * HTTP, etc.). Em caso de sucesso, grava o indice em *idx_saida (se
 * nao for NULL) e retorna PACIENTE_OK. Em caso de erro, retorna o
 * codigo correspondente e nao altera o estado global. */
ResultadoPaciente paciente_registrar(const char *cpf, const char *nome,
                                      const char *data_nascimento, const char *telefone,
                                      int *idx_saida);

/* Busca um paciente pelo CPF (busca linear). Retorna o indice ou -1
 * se nao encontrado. */
int buscar_paciente_cpf(const char *cpf);

/* Retorna um ponteiro somente-leitura para o paciente no indice dado,
 * ou NULL se o indice estiver fora do intervalo valido. */
const Paciente *paciente_obter(int idx);

/* Valida se uma string tem o formato de CPF (11 digitos numericos).
 * Retorna 1 se valido, 0 caso contrario. Nao valida os digitos
 * verificadores, apenas o formato. */
int validar_cpf_formato(const char *cpf);

/* Adiciona o indice de um agendamento ao historico do paciente. Retorna
 * 1 em sucesso, 0 se o indice de paciente for invalido ou o historico
 * estiver cheio (MAX_HISTORICO). */
int paciente_adicionar_historico(int idx_paciente, int idx_agendamento);

/* ==================== CAMADA TERMINAL (com I/O) ==================== */

/* Cadastra um novo paciente interagindo via terminal (le os dados do
 * teclado e delega a validacao/gravacao para paciente_registrar).
 * Retorna o indice do paciente cadastrado ou -1 em caso de erro. */
int cadastrar_paciente(void);

/* Lista todos os pacientes cadastrados em formato tabular no terminal. */
void listar_pacientes(void);

/* Exibe no terminal os dados detalhados de um paciente pelo indice. */
void exibir_paciente(int idx);

/* =================== CAMADA DE PERSISTENCIA (stub) =================== */

/* Carrega o estado inicial de pacientes. Nesta fase e um stub: apenas
 * garante que o sistema comeca com a base vazia em memoria (equivalente
 * a "ainda nao existe arquivo salvo"). Retorna a quantidade de
 * pacientes carregados. Deve ser chamada uma vez, no inicio do
 * programa (main.c), antes de qualquer interacao com o menu. */
int paciente_carregar(void);

/* Persiste o estado atual de pacientes. Nesta fase e um stub: nao
 * escreve em nenhum arquivo, apenas sinaliza sucesso. Retorna 1 em
 * sucesso, 0 em falha (reservado para quando houver I/O real). Deve
 * ser chamada uma vez, ao final do programa (main.c). */
int paciente_salvar(void);

#endif /* PACIENTE_H */
