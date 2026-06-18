/*
 * SUS-Agenda DF - Sistema de agendamento de consultas em
 * Unidades Basicas de Saude do Distrito Federal.
 *
 * Roadmap de 4 fases:
 *   Fase 1 (IMPLEMENTADA) - Modulo paciente: cadastro, busca e
 *           listagem, via menu de terminal.
 *   Fase 2 (IMPLEMENTADA neste arquivo) - Modulo medico e modulo
 *           slot/grade: cadastro/busca/listagem de medicos e geracao
 *           automatica da grade diaria de horarios por medico.
 *   Fase 3 (futura) - Modulo agendamento: vinculo entre paciente,
 *           medico e horario (slot), populando historico_ids do
 *           paciente e usando grade_selecionar_slot_terminal.
 *   Fase 4 (futura) - Modulo agenda: visualizacao e controle de
 *           disponibilidade de horarios por medico/unidade.
 *
 * Persistencia: NAO e uma fase separada. Cada modulo entrega seu
 * proprio stub de persistencia isolado (carregar/salvar operando em
 * memoria) desde a fase em que e criado -- e o que paciente_carregar/
 * paciente_salvar (Fase 1) e medico_carregar/medico_salvar e
 * grade_carregar/grade_salvar (Fase 2) ja fazem aqui. Transformar
 * esses stubs em I/O de arquivo de fato e uma continuacao a ser
 * decidida apos a Fase 4, sem exigir uma fase numerada propria.
 */

#include "menu.h"
#include "paciente.h"
#include "medico.h"
#include "slot.h"

int main(void) {
    paciente_carregar();
    medico_carregar();
    grade_carregar();

    menu_principal();

    paciente_salvar();
    medico_salvar();
    grade_salvar();
    return 0;
}
