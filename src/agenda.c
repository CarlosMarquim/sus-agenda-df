#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "agenda.h"
#include "paciente.h"
#include "medico.h"
#include "slot.h"
#include "agendamento.h"

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

void agenda_dia_terminal(void) {
    char data[16];
    int i;
    int grade_idx;
    int s;
    int turno_atual;

    printf("\n=== AGENDA DO DIA (TODOS OS MEDICOS) ===\n");

    if (num_medicos == 0) {
        printf("Nenhum medico cadastrado no sistema.\n");
        return;
    }

    printf("Data (DD/MM/AAAA): ");
    ler_linha(data, sizeof(data));

    for (i = 0; i < num_medicos; i++) {
        const Medico *m = medico_obter(i);

        if (m == NULL) {
            continue;
        }

        grade_idx = grade_buscar(i, data);
        if (grade_idx == -1) {
            grade_idx = grade_criar(i, data);
        }

        printf("\n--- %s (CRM: %s) - %s ---\n", m->nome, m->crm, m->especialidade);

        if (grade_idx == -1) {
            printf("  Nao foi possivel carregar a grade deste medico.\n");
            continue;
        }

        if (grades[grade_idx].num_slots == 0) {
            printf("  Sem disponibilidade nesse dia da semana.\n");
            continue;
        }

        turno_atual = -1;
        for (s = 0; s < grades[grade_idx].num_slots; s++) {
            const Slot *sl = &grades[grade_idx].slots[s];
            char hora_display[6];
            int hora_val = atoi(sl->hora);
            int turno_slot = (hora_val < 12) ? 0 : 1;

            if (turno_slot != turno_atual) {
                printf("\n  %s\n", turno_slot == 0 ? "MANHA" : "TARDE");
                turno_atual = turno_slot;
            }

            strcpy(hora_display, sl->hora);
            hora_display[2] = 'h';

            if (!sl->ocupado) {
                printf("  %s - LIVRE\n", hora_display);
            } else {
                const Paciente *p = paciente_obter(sl->paciente_idx);
                const char *nome_pac = (p != NULL) ? p->nome : "(desconhecido)";

                if (sl->agendamento_idx >= 0) {
                    const Agendamento *ag = agendamento_obter(sl->agendamento_idx);
                    if (ag != NULL) {
                        printf("  %s - %s | %.60s\n", hora_display, nome_pac, ag->queixa);
                    } else {
                        printf("  %s - %s\n", hora_display, nome_pac);
                    }
                } else {
                    printf("  %s - %s\n", hora_display, nome_pac);
                }
            }
        }
    }
}

void agenda_historico_paciente_terminal(void) {
    char cpf[64];
    int idx_paciente;
    int resultados[MAX_AGENDAMENTOS];
    int total;
    int i;

    printf("\n=== HISTORICO DE CONSULTAS ===\n");

    printf("CPF do paciente (11 digitos): ");
    ler_linha(cpf, sizeof(cpf));

    idx_paciente = buscar_paciente_cpf(cpf);
    if (idx_paciente == -1) {
        printf("Erro: paciente nao encontrado.\n");
        return;
    }

    {
        const Paciente *p = paciente_obter(idx_paciente);
        printf("Paciente: %s\n", p->nome);
    }

    total = buscar_agendamentos_paciente(idx_paciente, 0, resultados, MAX_AGENDAMENTOS);
    if (total == 0) {
        printf("Nenhum agendamento no historico desse paciente.\n");
        return;
    }

    printf("\n%-8s %-10s %-18s %-12s %-6s %-10s %s\n",
           "Proto.", "CRM", "Especialidade", "Data", "Hora", "Status", "Queixa");
    printf("--------------------------------------------------------------------------------------------\n");

    for (i = 0; i < total; i++) {
        const Agendamento *ag = agendamento_obter(resultados[i]);
        const char *hora = "?";
        int gi;

        if (ag == NULL) {
            continue;
        }

        gi = grade_buscar(ag->idMedico, ag->data);
        if (gi != -1 && ag->slot >= 0 && ag->slot < grades[gi].num_slots) {
            hora = grades[gi].slots[ag->slot].hora;
        }

        printf("%-8d %-10s %-18s %-12s %-6s %-10s %.40s\n",
               ag->protocolo,
               medicos[ag->idMedico].crm,
               medicos[ag->idMedico].especialidade,
               ag->data,
               hora,
               ag->cancelado ? "CANCELADO" : "ATIVO",
               ag->queixa);
    }
}
