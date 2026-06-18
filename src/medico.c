#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "medico.h"

/* Definicao do array global e do contador de medicos (declarados como
 * extern em medico.h). */
Medico medicos[MAX_MEDICOS];
int num_medicos;

/* Nomes dos dias da semana, na mesma ordem usada em
 * Medico.disponibilidade (0=domingo ... 6=sabado). */
static const char *dias_semana[7] = {
    "Domingo", "Segunda", "Terca", "Quarta", "Quinta", "Sexta", "Sabado"
};

/* Le uma linha do stdin, remove o '\n' final (se houver) e descarta o
 * restante da linha caso o conteudo exceda o tamanho do buffer.
 * Funcao auxiliar interna da camada terminal (mesmo padrao usado em
 * paciente.c). Usar fgets em vez de scanf evita o bug de loop
 * infinito em EOF que foi corrigido no menu da Fase 1. */
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

/* Retorna 1 se a string for NULL, vazia, ou contiver apenas espacos
 * em branco; 0 caso contrario. Usada para validar a especialidade. */
static int esta_vazia_ou_so_espacos(const char *texto) {
    int i;

    if (texto == NULL) {
        return 1;
    }
    for (i = 0; texto[i] != '\0'; i++) {
        if (!isspace((unsigned char)texto[i])) {
            return 0;
        }
    }
    return 1;
}

/* ===================== CAMADA CORE (sem I/O) ===================== */

int buscar_medico_crm(const char *crm) {
    int i;

    if (crm == NULL) {
        return -1;
    }
    for (i = 0; i < num_medicos; i++) {
        if (strcmp(medicos[i].crm, crm) == 0) {
            return i;
        }
    }
    return -1;
}

int buscar_medicos_especialidade(const char *especialidade, int *resultados, int max) {
    int i;
    int total = 0;

    if (especialidade == NULL || resultados == NULL || max <= 0) {
        return 0;
    }
    for (i = 0; i < num_medicos && total < max; i++) {
        if (strcmp(medicos[i].especialidade, especialidade) == 0) {
            resultados[total] = i;
            total++;
        }
    }
    return total;
}

void medico_obter(int idx, Medico *dest) {
    if (dest == NULL) {
        return;
    }
    if (idx < 0 || idx >= num_medicos) {
        return;
    }
    *dest = medicos[idx];
}

int medico_registrar(const char *crm, const char *nome, const char *especialidade) {
    int idx;
    int dia;
    int turno;

    if (num_medicos >= MAX_MEDICOS) {
        return -1;
    }
    if (crm == NULL || strlen(crm) == 0) {
        return -1;
    }
    if (buscar_medico_crm(crm) != -1) {
        return -1;
    }
    if (nome == NULL || strlen(nome) == 0) {
        return -1;
    }
    if (esta_vazia_ou_so_espacos(especialidade)) {
        return -1;
    }

    idx = num_medicos;

    strncpy(medicos[idx].crm, crm, sizeof(medicos[idx].crm) - 1);
    medicos[idx].crm[sizeof(medicos[idx].crm) - 1] = '\0';

    strncpy(medicos[idx].nome, nome, sizeof(medicos[idx].nome) - 1);
    medicos[idx].nome[sizeof(medicos[idx].nome) - 1] = '\0';

    strncpy(medicos[idx].especialidade, especialidade, sizeof(medicos[idx].especialidade) - 1);
    medicos[idx].especialidade[sizeof(medicos[idx].especialidade) - 1] = '\0';

    for (dia = 0; dia < 7; dia++) {
        for (turno = 0; turno < 2; turno++) {
            medicos[idx].disponibilidade[dia][turno] = 0;
        }
    }

    num_medicos++;
    return idx;
}

/* ==================== CAMADA TERMINAL (com I/O) ==================== */

void medico_cadastrar_terminal(void) {
    char crm[32];
    char nome[256];
    char especialidade[64];
    int idx;

    printf("\n=== CADASTRO DE MEDICO ===\n");

    printf("CRM: ");
    ler_linha(crm, sizeof(crm));

    printf("Nome completo: ");
    ler_linha(nome, sizeof(nome));

    printf("Especialidade: ");
    ler_linha(especialidade, sizeof(especialidade));

    /* Validacoes especificas na camada terminal para dar uma mensagem
     * de erro precisa ao atendente. medico_registrar tambem revalida
     * tudo internamente (defesa em profundidade). */
    if (strlen(crm) == 0) {
        printf("Erro: CRM nao pode ser vazio.\n");
        return;
    }
    if (buscar_medico_crm(crm) != -1) {
        printf("Erro: CRM ja cadastrado.\n");
        return;
    }
    if (strlen(nome) == 0) {
        printf("Erro: nome nao pode ser vazio.\n");
        return;
    }
    if (esta_vazia_ou_so_espacos(especialidade)) {
        printf("Erro: especialidade nao pode ser vazia.\n");
        return;
    }

    idx = medico_registrar(crm, nome, especialidade);
    if (idx == -1) {
        printf("Erro: capacidade maxima de medicos atingida.\n");
        return;
    }

    printf("Medico cadastrado com sucesso. Protocolo interno: #%03d\n", idx + 1);
}

void medico_listar_terminal(void) {
    int i;

    printf("\n=== LISTA DE MEDICOS ===\n");

    if (num_medicos == 0) {
        printf("Nenhum medico cadastrado.\n");
        return;
    }

    printf("%-6s %-12s %-30s %-20s\n", "Prot.", "CRM", "Nome", "Especialidade");
    printf("------------------------------------------------------------------\n");
    for (i = 0; i < num_medicos; i++) {
        printf("%-6d %-12s %-30s %-20s\n",
               i + 1,
               medicos[i].crm,
               medicos[i].nome,
               medicos[i].especialidade);
    }
}

void medico_buscar_terminal(void) {
    char opcao_buffer[16];
    int opcao;

    printf("\n=== BUSCAR MEDICO ===\n");
    printf("1. Buscar por CRM\n");
    printf("2. Buscar por especialidade\n");
    printf("Escolha uma opcao: ");
    ler_linha(opcao_buffer, sizeof(opcao_buffer));
    opcao = atoi(opcao_buffer);

    if (opcao == 1) {
        char crm[32];
        int idx;

        printf("Informe o CRM: ");
        ler_linha(crm, sizeof(crm));

        idx = buscar_medico_crm(crm);
        if (idx == -1) {
            printf("Erro: medico nao encontrado para o CRM informado.\n");
            return;
        }

        printf("\n--- Dados do Medico #%03d ---\n", idx + 1);
        printf("CRM..............: %s\n", medicos[idx].crm);
        printf("Nome..............: %s\n", medicos[idx].nome);
        printf("Especialidade.....: %s\n", medicos[idx].especialidade);
    } else if (opcao == 2) {
        char especialidade[64];
        int resultados[MAX_MEDICOS];
        int total;
        int i;

        printf("Informe a especialidade: ");
        ler_linha(especialidade, sizeof(especialidade));

        total = buscar_medicos_especialidade(especialidade, resultados, MAX_MEDICOS);
        if (total == 0) {
            printf("Nenhum medico encontrado para essa especialidade.\n");
            return;
        }

        printf("\n--- Medicos encontrados (%d) ---\n", total);
        printf("%-6s %-12s %-30s\n", "Prot.", "CRM", "Nome");
        printf("---------------------------------------\n");
        for (i = 0; i < total; i++) {
            int idx = resultados[i];

            printf("%-6d %-12s %-30s\n",
                   idx + 1,
                   medicos[idx].crm,
                   medicos[idx].nome);
        }
    } else {
        printf("Erro: opcao de busca invalida.\n");
    }
}

/* Imprime a tabela de disponibilidade atual de um medico (7 dias x
 * 2 turnos), sem alterar nenhum dado. */
static void exibir_tabela_disponibilidade(const Medico *m) {
    int dia;

    printf("\n=== DISPONIBILIDADE - Dr. %s ===\n\n", m->nome);
    printf("Dia           Manha    Tarde\n");
    for (dia = 0; dia < 7; dia++) {
        printf("%-13s %-8s %-8s\n",
               dias_semana[dia],
               m->disponibilidade[dia][TURNO_MANHA] ? "S" : "N",
               m->disponibilidade[dia][TURNO_TARDE] ? "S" : "N");
    }
}

void medico_configurar_disponibilidade_terminal(void) {
    char crm[32];
    char linha[32];
    int idx;
    int dia;
    int turno;

    printf("\n=== CONFIGURAR DISPONIBILIDADE ===\n");
    printf("CRM do medico: ");
    ler_linha(crm, sizeof(crm));

    idx = buscar_medico_crm(crm);
    if (idx == -1) {
        printf("Erro: medico nao encontrado para o CRM informado.\n");
        return;
    }

    for (;;) {
        exibir_tabela_disponibilidade(&medicos[idx]);

        printf("\nDigite o dia (0=domingo ... 6=sabado) ou -1 para sair: ");
        ler_linha(linha, sizeof(linha));
        dia = atoi(linha);
        if (dia == -1) {
            break;
        }
        if (dia < 0 || dia > 6) {
            printf("Erro: dia invalido. Use 0 (domingo) a 6 (sabado).\n");
            continue;
        }

        printf("Digite o turno (0=manha, 1=tarde): ");
        ler_linha(linha, sizeof(linha));
        turno = atoi(linha);
        if (turno != TURNO_MANHA && turno != TURNO_TARDE) {
            printf("Erro: turno invalido. Use 0 (manha) ou 1 (tarde).\n");
            continue;
        }

        medicos[idx].disponibilidade[dia][turno] = !medicos[idx].disponibilidade[dia][turno];
    }

    printf("\n=== DISPONIBILIDADE FINAL ===\n");
    exibir_tabela_disponibilidade(&medicos[idx]);
}

/* =================== CAMADA DE PERSISTENCIA (stub) =================== */

int medico_carregar(void) {
    /* Stub da Fase 2: ainda nao existe arquivo para ler, entao o
     * sistema sempre comeca com a base de medicos vazia, em memoria.
     * Mesmo padrao de paciente_carregar (Fase 1). */
    num_medicos = 0;
    return num_medicos;
}

int medico_salvar(void) {
    /* Stub da Fase 2: os dados permanecem apenas em memoria durante a
     * execucao. Mesmo padrao de paciente_salvar (Fase 1). */
    return 1;
}
