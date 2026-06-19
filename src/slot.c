#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "slot.h"
#include "medico.h"
#include "paciente.h"

/* Definicao do array global e do contador de grades (declarados como
 * extern em slot.h). */
GradeDia grades[MAX_GRADES];
int num_grades;

/* Le uma linha do stdin (mesmo padrao usado em paciente.c/medico.c). */
static void ler_linha(char *destino, int tamanho) {
    if (fgets(destino, tamanho, stdin) == NULL) {
        destino[0] = '\0';
        return;
    }

    size_t fim = strcspn(destino, "\n");
    if (destino[fim] != '\n' && fim == (size_t)(tamanho - 1)) {
        int c;
        while ((c = getchar()) != '\n' && c != EOF) {
            /* descarta */
        }
    }
    destino[fim] = '\0';
}

/* Acrescenta 'quantidade' slots de 30 minutos a partir de
 * hora_inicial:minuto_inicial na grade, respeitando MAX_SLOTS_GRADE.
 * Funcao auxiliar interna da camada core, nao exposta no header. */
static void gerar_slots_periodo(GradeDia *grade, int hora_inicial, int minuto_inicial, int quantidade) {
    int i;
    int hora = hora_inicial;
    int minuto = minuto_inicial;

    for (i = 0; i < quantidade && grade->num_slots < MAX_SLOTS_GRADE; i++) {
        Slot *s = &grade->slots[grade->num_slots];

        snprintf(s->hora, sizeof(s->hora), "%02d:%02d", hora, minuto);
        s->ocupado = 0;
        s->paciente_idx = -1;
        s->agendamento_idx = -1;
        grade->num_slots++;

        minuto += 30;
        if (minuto >= 60) {
            minuto -= 60;
            hora += 1;
        }
    }
}

/* Calcula o dia da semana (0=domingo ... 6=sabado) de uma data no
 * formato "DD/MM/AAAA", usando o algoritmo de Sakamoto (equivalente
 * em precisao a formula de Zeller, em C puro). Retorna -1 se a
 * string nao puder ser interpretada como data. Funcao auxiliar
 * interna da camada core, nao exposta no header. */
static int dia_semana_de_data(const char *data) {
    static const int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    int dia, mes, ano, a;

    if (data == NULL) {
        return -1;
    }
    if (sscanf(data, "%d/%d/%d", &dia, &mes, &ano) != 3) {
        return -1;
    }
    if (mes < 1 || mes > 12 || dia < 1 || dia > 31) {
        return -1;
    }

    a = ano;
    if (mes < 3) {
        a--;
    }
    return (a + a / 4 - a / 100 + a / 400 + t[mes - 1] + dia) % 7;
}

/* ===================== CAMADA CORE (sem I/O) ===================== */

int grade_buscar(int medico_idx, const char *data) {
    int i;

    if (data == NULL) {
        return -1;
    }
    for (i = 0; i < num_grades; i++) {
        if (grades[i].medico_idx == medico_idx && strcmp(grades[i].data, data) == 0) {
            return i;
        }
    }
    return -1;
}

int grade_criar(int medico_idx, const char *data) {
    int idx;
    int dia_semana;
    const Medico *m;

    if (medico_idx < 0 || medico_idx >= num_medicos) {
        return -1;
    }
    if (data == NULL || strlen(data) == 0) {
        return -1;
    }
    if (grade_buscar(medico_idx, data) != -1) {
        return -1;
    }
    if (num_grades >= MAX_GRADES) {
        return -1;
    }

    dia_semana = dia_semana_de_data(data);
    if (dia_semana < 0 || dia_semana > 6) {
        return -1;
    }

    m = medico_obter(medico_idx);
    if (m == NULL) {
        return -1;
    }

    idx = num_grades;
    grades[idx].medico_idx = medico_idx;
    strncpy(grades[idx].data, data, sizeof(grades[idx].data) - 1);
    grades[idx].data[sizeof(grades[idx].data) - 1] = '\0';
    grades[idx].num_slots = 0;

    if (m->disponibilidade[dia_semana][TURNO_MANHA]) {
        gerar_slots_periodo(&grades[idx], 7, 0, 9);
    }
    if (m->disponibilidade[dia_semana][TURNO_TARDE]) {
        gerar_slots_periodo(&grades[idx], 13, 0, 9);
    }

    num_grades++;
    return idx;
}

const GradeDia *grade_obter(int idx) {
    if (idx < 0 || idx >= num_grades) {
        return NULL;
    }
    return &grades[idx];
}

int slot_ocupar(int grade_idx, int slot_idx, int paciente_idx) {
    if (grade_idx < 0 || grade_idx >= num_grades) {
        return -1;
    }
    if (slot_idx < 0 || slot_idx >= grades[grade_idx].num_slots) {
        return -1;
    }
    if (grades[grade_idx].slots[slot_idx].ocupado) {
        return -1; /* slot ja ocupado */
    }

    grades[grade_idx].slots[slot_idx].ocupado = 1;
    grades[grade_idx].slots[slot_idx].paciente_idx = paciente_idx;
    return 0;
}

int slot_vincular_agendamento(int grade_idx, int slot_idx, int agendamento_idx) {
    if (grade_idx < 0 || grade_idx >= num_grades) {
        return -1;
    }
    if (slot_idx < 0 || slot_idx >= grades[grade_idx].num_slots) {
        return -1;
    }
    grades[grade_idx].slots[slot_idx].agendamento_idx = agendamento_idx;
    return 0;
}

int slot_liberar(int grade_idx, int slot_idx) {
    if (grade_idx < 0 || grade_idx >= num_grades) {
        return -1;
    }
    if (slot_idx < 0 || slot_idx >= grades[grade_idx].num_slots) {
        return -1;
    }
    if (!grades[grade_idx].slots[slot_idx].ocupado) {
        return -1; /* slot ja livre */
    }

    grades[grade_idx].slots[slot_idx].ocupado = 0;
    grades[grade_idx].slots[slot_idx].paciente_idx = -1;
    grades[grade_idx].slots[slot_idx].agendamento_idx = -1;
    return 0;
}

/* ==================== CAMADA TERMINAL (com I/O) ==================== */

void grade_exibir_terminal(int grade_idx) {
    int i;
    const GradeDia *g;
    const Medico *m;
    int turno_atual = -1;

    if (grade_idx < 0 || grade_idx >= num_grades) {
        printf("Erro: grade nao encontrada.\n");
        return;
    }
    g = &grades[grade_idx];

    m = medico_obter(g->medico_idx);
    if (m == NULL) {
        printf("Erro: medico nao encontrado.\n");
        return;
    }

    printf("\n=== GRADE DE SLOTS - %s - %s ===\n", m->nome, g->data);

    if (g->num_slots == 0) {
        printf("\nNenhum slot disponivel nesta grade.\n");
        return;
    }

    for (i = 0; i < g->num_slots; i++) {
        const Slot *s = &g->slots[i];
        char indice_str[16];
        char hora_display[6];
        int hora_slot = atoi(s->hora);
        int turno_slot = (hora_slot < 12) ? 0 : 1;

        if (turno_slot != turno_atual) {
            printf("\n%s\n", turno_slot == 0 ? "MANHA" : "TARDE");
            turno_atual = turno_slot;
        }

        strcpy(hora_display, s->hora);
        hora_display[2] = 'h';

        snprintf(indice_str, sizeof(indice_str), "[%d]", i + 1);

        if (s->ocupado) {
            const Paciente *p = paciente_obter(s->paciente_idx);
            printf("%4s %s - OCUPADO (%s)\n", indice_str, hora_display, (p != NULL ? p->nome : "(desconhecido)"));
        } else {
            printf("%4s %s - LIVRE\n", indice_str, hora_display);
        }
    }
}

void grade_selecionar_slot_terminal(int grade_idx, int *slot_escolhido) {
    char buffer[16];
    int escolha;

    if (slot_escolhido != NULL) {
        *slot_escolhido = -1;
    }

    grade_exibir_terminal(grade_idx);

    if (grade_idx < 0 || grade_idx >= num_grades) {
        return;
    }

    printf("\nEscolha o numero do slot livre (1 a %d), ou 0 para cancelar: ", grades[grade_idx].num_slots);
    ler_linha(buffer, sizeof(buffer));
    escolha = atoi(buffer);

    if (escolha <= 0 || escolha > grades[grade_idx].num_slots) {
        printf("Operacao cancelada.\n");
        return;
    }

    if (grades[grade_idx].slots[escolha - 1].ocupado) {
        printf("Erro: slot ja ocupado.\n");
        return;
    }

    if (slot_escolhido != NULL) {
        *slot_escolhido = escolha - 1;
    }
}

/* =================== CAMADA DE PERSISTENCIA (stub) =================== */

int grade_carregar(void) {
    num_grades = 0;
    return num_grades;
}

int grade_salvar(void) {
    return 1;
}
