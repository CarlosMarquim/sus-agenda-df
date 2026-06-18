#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "paciente.h"

/* Definicao do array global e do contador de pacientes (declarados
 * como extern em paciente.h). */
Paciente pacientes[MAX_PACIENTES];
int num_pacientes;

/* Le uma linha do stdin, remove o '\n' final (se houver) e descarta
 * o restante da linha caso o conteudo exceda o tamanho do buffer.
 * Funcao auxiliar interna da camada terminal, nao exposta no header. */
static void ler_linha(char *destino, int tamanho) {
    if (fgets(destino, tamanho, stdin) == NULL) {
        destino[0] = '\0';
        return;
    }

    size_t fim = strcspn(destino, "\n");
    if (destino[fim] != '\n' && fim == (size_t)(tamanho - 1)) {
        /* Linha mais longa que o buffer: descarta o restante da entrada. */
        int c;
        while ((c = getchar()) != '\n' && c != EOF) {
            /* descarta */
        }
    }
    destino[fim] = '\0';
}

/* ===================== CAMADA CORE (sem I/O) ===================== */

int validar_cpf_formato(const char *cpf) {
    int i;

    if (cpf == NULL) {
        return 0;
    }
    if (strlen(cpf) != 11) {
        return 0;
    }
    for (i = 0; i < 11; i++) {
        if (!isdigit((unsigned char)cpf[i])) {
            return 0;
        }
    }
    return 1;
}

int buscar_paciente_cpf(const char *cpf) {
    int i;

    if (cpf == NULL) {
        return -1;
    }
    for (i = 0; i < num_pacientes; i++) {
        if (strcmp(pacientes[i].cpf, cpf) == 0) {
            return i;
        }
    }
    return -1;
}

const Paciente *paciente_obter(int idx) {
    if (idx < 0 || idx >= num_pacientes) {
        return NULL;
    }
    return &pacientes[idx];
}

ResultadoPaciente paciente_registrar(const char *cpf, const char *nome,
                                      const char *data_nascimento, int *idx_saida) {
    int idx;

    if (num_pacientes >= MAX_PACIENTES) {
        return PACIENTE_ERRO_CAPACIDADE_MAXIMA;
    }
    if (!validar_cpf_formato(cpf)) {
        return PACIENTE_ERRO_CPF_FORMATO_INVALIDO;
    }
    if (buscar_paciente_cpf(cpf) != -1) {
        return PACIENTE_ERRO_CPF_DUPLICADO;
    }
    if (nome == NULL || strlen(nome) == 0) {
        return PACIENTE_ERRO_NOME_VAZIO;
    }
    if (data_nascimento == NULL || strlen(data_nascimento) == 0) {
        return PACIENTE_ERRO_DATA_VAZIA;
    }

    idx = num_pacientes;

    strncpy(pacientes[idx].cpf, cpf, sizeof(pacientes[idx].cpf) - 1);
    pacientes[idx].cpf[sizeof(pacientes[idx].cpf) - 1] = '\0';

    strncpy(pacientes[idx].nome, nome, sizeof(pacientes[idx].nome) - 1);
    pacientes[idx].nome[sizeof(pacientes[idx].nome) - 1] = '\0';

    strncpy(pacientes[idx].data_nascimento, data_nascimento, sizeof(pacientes[idx].data_nascimento) - 1);
    pacientes[idx].data_nascimento[sizeof(pacientes[idx].data_nascimento) - 1] = '\0';

    pacientes[idx].num_historico = 0;
    pacientes[idx].idx_agendamento_atual = -1; /* sem agendamento ainda (Fase 3) */

    num_pacientes++;

    if (idx_saida != NULL) {
        *idx_saida = idx;
    }
    return PACIENTE_OK;
}

/* ==================== CAMADA TERMINAL (com I/O) ==================== */

int cadastrar_paciente(void) {
    char cpf[64];
    char nome[256];
    char data_nascimento[64];
    int idx;
    ResultadoPaciente resultado;

    printf("\n=== CADASTRO DE PACIENTE ===\n");

    printf("CPF (somente numeros, 11 digitos): ");
    ler_linha(cpf, sizeof(cpf));

    printf("Nome completo: ");
    ler_linha(nome, sizeof(nome));

    printf("Data de nascimento (DD/MM/AAAA): ");
    ler_linha(data_nascimento, sizeof(data_nascimento));

    resultado = paciente_registrar(cpf, nome, data_nascimento, &idx);

    switch (resultado) {
        case PACIENTE_OK:
            printf("Paciente cadastrado com sucesso. Protocolo interno: #%03d\n", idx + 1);
            return idx;
        case PACIENTE_ERRO_CAPACIDADE_MAXIMA:
            printf("Erro: capacidade maxima de pacientes atingida.\n");
            return -1;
        case PACIENTE_ERRO_CPF_FORMATO_INVALIDO:
            printf("Erro: CPF deve conter 11 digitos numericos.\n");
            return -1;
        case PACIENTE_ERRO_CPF_DUPLICADO:
            printf("Erro: CPF ja cadastrado.\n");
            return -1;
        case PACIENTE_ERRO_NOME_VAZIO:
            printf("Erro: nome nao pode ser vazio.\n");
            return -1;
        case PACIENTE_ERRO_DATA_VAZIA:
            printf("Erro: data de nascimento nao pode ser vazia.\n");
            return -1;
        default:
            printf("Erro: falha desconhecida ao cadastrar paciente.\n");
            return -1;
    }
}

void exibir_paciente(int idx) {
    const Paciente *p = paciente_obter(idx);

    if (p == NULL) {
        printf("Erro: paciente nao encontrado.\n");
        return;
    }

    printf("\n--- Dados do Paciente #%03d ---\n", idx + 1);
    printf("CPF..............: %s\n", p->cpf);
    printf("Nome..............: %s\n", p->nome);
    printf("Data Nascimento...: %s\n", p->data_nascimento);
    printf("Historico.........: %d agendamento(s) registrado(s)\n", p->num_historico);
}

void listar_pacientes(void) {
    int i;

    printf("\n=== LISTA DE PACIENTES ===\n");

    if (num_pacientes == 0) {
        printf("Nenhum paciente cadastrado.\n");
        return;
    }

    printf("%-6s %-15s %-30s %-12s\n", "Prot.", "CPF", "Nome", "Nascimento");
    printf("---------------------------------------------------------------\n");
    for (i = 0; i < num_pacientes; i++) {
        printf("%-6d %-15s %-30s %-12s\n",
               i + 1,
               pacientes[i].cpf,
               pacientes[i].nome,
               pacientes[i].data_nascimento);
    }
}

/* =================== CAMADA DE PERSISTENCIA (stub) =================== */

int paciente_carregar(void) {
    /* Stub da Fase 1: ainda nao existe arquivo para ler, entao o
     * sistema sempre comeca com a base de pacientes vazia, em memoria.
     * Quando a persistencia em arquivo for implementada (fase futura),
     * esta funcao passa a ler de um arquivo (ex.: pacientes.dat) e
     * popular 'pacientes'/'num_pacientes' -- sem que cadastrar_paciente,
     * buscar_paciente_cpf, listar_pacientes ou o menu precisem mudar
     * uma unica linha. */
    num_pacientes = 0;
    return num_pacientes;
}

int paciente_salvar(void) {
    /* Stub da Fase 1: os dados permanecem apenas em memoria durante a
     * execucao (encerrar o programa descarta o que foi cadastrado).
     * Quando a persistencia em arquivo for implementada, esta funcao
     * passa a escrever 'pacientes'/'num_pacientes' em disco, mantendo
     * a mesma assinatura usada pelo restante do sistema. */
    return 1;
}
