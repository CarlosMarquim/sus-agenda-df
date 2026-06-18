#ifndef AGENDAMENTO_H
#define AGENDAMENTO_H

/*
 * Modulo Agendamento - struct minima pre-declarada na Fase 2 para
 * evitar dependencia circular futura (Fase 3). Nenhuma funcao e
 * implementada ainda; isso fica para quando o modulo agendamento for
 * de fato construido.
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

#endif /* AGENDAMENTO_H */
