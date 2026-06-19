#ifndef AGENDA_H
#define AGENDA_H

/*
 * Modulo Agenda - Fase 4 do SUS-Agenda DF
 *
 * Visoes consolidadas de agenda e historico de consultas. Modulo de
 * exibicao pura (camada terminal): le dados dos demais modulos sem
 * manter estado proprio. Nao ha camada core nem persistencia.
 */

/* RF16 - Exibe a agenda consolidada de TODOS os medicos para uma data,
 * degradando graciosamente (avisa e segue) quando a grade de um medico
 * individual nao puder ser carregada. */
void agenda_dia_terminal(void);

/* RF18 - Exibe o historico completo de consultas de um paciente (ativos
 * e cancelados), buscado por CPF. */
void agenda_historico_paciente_terminal(void);

#endif /* AGENDA_H */
