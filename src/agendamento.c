#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "agendamento.h"
#include "paciente.h"
#include "medico.h"
#include "slot.h"

/* Definicao dos globais declarados como extern em agendamento.h. */
Agendamento agendamentos[MAX_AGENDAMENTOS];
int num_agendamentos;
int proximo_protocolo;

/* Le uma linha do stdin, remove '\n' final e descarta excesso de entrada.
 * Mesmo padrao usado em paciente.c, medico.c e slot.c. */
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

/* Retorna 1 se a string for NULL, vazia ou contiver apenas espacos. */
static int queixa_invalida(const char *queixa) {
    int i;

    if (queixa == NULL || queixa[0] == '\0') {
        return 1;
    }
    for (i = 0; queixa[i] != '\0'; i++) {
        if (!isspace((unsigned char)queixa[i])) {
            return 0;
        }
    }
    return 1;
}

/* ===================== CAMADA CORE (sem I/O) ===================== */

int agendamento_paciente_tem_conflito_dia(int idPaciente, const char *data) {
    int i;

    if (data == NULL) {
        return 0;
    }
    for (i = 0; i < num_agendamentos; i++) {
        if (agendamentos[i].idPaciente == idPaciente
                && strcmp(agendamentos[i].data, data) == 0
                && agendamentos[i].cancelado == 0) {
            return 1;
        }
    }
    return 0;
}

ResultadoAgendamento agendamento_criar(int idPaciente, int idMedico,
                                        const char *data, int slot_idx,
                                        const char *queixa, int *protocolo_saida) {
    int grade_idx;
    int novo_idx;

    if (num_agendamentos >= MAX_AGENDAMENTOS) {
        return AGENDAMENTO_ERRO_CAPACIDADE_MAXIMA;
    }
    if (paciente_obter(idPaciente) == NULL) {
        return AGENDAMENTO_ERRO_PACIENTE_INVALIDO;
    }
    if (idMedico < 0 || idMedico >= num_medicos) {
        return AGENDAMENTO_ERRO_MEDICO_INVALIDO;
    }
    if (queixa_invalida(queixa)) {
        return AGENDAMENTO_ERRO_QUEIXA_VAZIA;
    }

    grade_idx = grade_buscar(idMedico, data);
    if (grade_idx == -1) {
        grade_idx = grade_criar(idMedico, data);
    }
    if (grade_idx == -1) {
        return AGENDAMENTO_ERRO_GRADE_OU_DATA_INVALIDA;
    }

    if (slot_idx < 0 || slot_idx >= grades[grade_idx].num_slots) {
        return AGENDAMENTO_ERRO_SLOT_INVALIDO;
    }
    if (grades[grade_idx].slots[slot_idx].ocupado) {
        return AGENDAMENTO_ERRO_SLOT_OCUPADO;
    }
    if (agendamento_paciente_tem_conflito_dia(idPaciente, data)) {
        return AGENDAMENTO_ERRO_CONFLITO_MESMO_DIA;
    }

    novo_idx = num_agendamentos;

    agendamentos[novo_idx].protocolo  = proximo_protocolo;
    agendamentos[novo_idx].idPaciente = idPaciente;
    agendamentos[novo_idx].idMedico   = idMedico;
    agendamentos[novo_idx].slot       = slot_idx;
    agendamentos[novo_idx].cancelado  = 0;

    strncpy(agendamentos[novo_idx].data, data, sizeof(agendamentos[novo_idx].data) - 1);
    agendamentos[novo_idx].data[sizeof(agendamentos[novo_idx].data) - 1] = '\0';

    strncpy(agendamentos[novo_idx].queixa, queixa, sizeof(agendamentos[novo_idx].queixa) - 1);
    agendamentos[novo_idx].queixa[sizeof(agendamentos[novo_idx].queixa) - 1] = '\0';

    slot_ocupar(grade_idx, slot_idx, idPaciente);
    slot_vincular_agendamento(grade_idx, slot_idx, novo_idx);
    paciente_adicionar_historico(idPaciente, novo_idx);

    if (protocolo_saida != NULL) {
        *protocolo_saida = proximo_protocolo;
    }

    num_agendamentos++;
    proximo_protocolo++;

    return AGENDAMENTO_OK;
}

ResultadoAgendamento agendamento_cancelar(int idx_agendamento) {
    int grade_idx;

    if (idx_agendamento < 0 || idx_agendamento >= num_agendamentos) {
        return AGENDAMENTO_ERRO_PROTOCOLO_NAO_ENCONTRADO;
    }
    if (agendamentos[idx_agendamento].cancelado) {
        return AGENDAMENTO_ERRO_JA_CANCELADO;
    }

    agendamentos[idx_agendamento].cancelado = 1;

    grade_idx = grade_buscar(agendamentos[idx_agendamento].idMedico,
                              agendamentos[idx_agendamento].data);
    if (grade_idx != -1) {
        slot_liberar(grade_idx, agendamentos[idx_agendamento].slot);
    }

    return AGENDAMENTO_OK;
}

int buscar_agendamento_protocolo(int protocolo) {
    int i;

    for (i = 0; i < num_agendamentos; i++) {
        if (agendamentos[i].protocolo == protocolo) {
            return i;
        }
    }
    return -1;
}

int buscar_agendamentos_paciente(int idPaciente, int apenas_ativos,
                                  int *resultados, int max) {
    int i;
    int total = 0;

    if (resultados == NULL || max <= 0) {
        return 0;
    }
    for (i = 0; i < num_agendamentos && total < max; i++) {
        if (agendamentos[i].idPaciente != idPaciente) {
            continue;
        }
        if (apenas_ativos && agendamentos[i].cancelado) {
            continue;
        }
        resultados[total] = i;
        total++;
    }
    return total;
}

const Agendamento *agendamento_obter(int idx) {
    if (idx < 0 || idx >= num_agendamentos) {
        return NULL;
    }
    return &agendamentos[idx];
}

/* ==================== CAMADA TERMINAL (com I/O) ==================== */

/* Exibe os dados completos de um agendamento no terminal. Funcao
 * auxiliar interna compartilhada entre criar e cancelar. */
static void exibir_agendamento(int idx) {
    const Agendamento *ag = agendamento_obter(idx);
    int grade_idx;
    const char *hora = "?";

    if (ag == NULL) {
        return;
    }

    grade_idx = grade_buscar(ag->idMedico, ag->data);
    if (grade_idx != -1 && ag->slot >= 0 && ag->slot < grades[grade_idx].num_slots) {
        hora = grades[grade_idx].slots[ag->slot].hora;
    }

    printf("Protocolo.: %d\n", ag->protocolo);
    printf("Paciente..: %s (CPF: %s)\n",
           pacientes[ag->idPaciente].nome,
           pacientes[ag->idPaciente].cpf);
    printf("Medico....: %s (CRM: %s)\n",
           medicos[ag->idMedico].nome,
           medicos[ag->idMedico].crm);
    printf("Especialid: %s\n", medicos[ag->idMedico].especialidade);
    printf("Data......: %s\n", ag->data);
    printf("Horario...: %s\n", hora);
    printf("Queixa....: %s\n", ag->queixa);
    printf("Status....: %s\n", ag->cancelado ? "CANCELADO" : "ATIVO");
}

void agendamento_criar_terminal(void) {
    char cpf[64];
    char buffer[32];
    char queixa[202];
    char data[16];
    char crm[32];
    int idx_paciente;
    int idx_medico;
    int grade_idx;
    int slot_escolhido;
    int protocolo_saida;
    int opcao_esp;
    int resultados_medicos[MAX_MEDICOS];
    int total_medicos;
    int i;
    int encontrado;
    const Paciente *p;
    ResultadoAgendamento resultado;

    printf("\n=== CRIAR AGENDAMENTO ===\n");

    /* Passo 1: localizar paciente */
    printf("CPF do paciente (11 digitos): ");
    ler_linha(cpf, sizeof(cpf));

    if (!validar_cpf_formato(cpf)) {
        printf("CPF invalido. Digite 11 digitos numericos.\n");
        return;
    }

    idx_paciente = buscar_paciente_cpf(cpf);
    if (idx_paciente == -1) {
        printf("Paciente nao encontrado.\n");
        printf("1. Cadastrar agora\n0. Cancelar operacao\nOpcao: ");
        ler_linha(buffer, sizeof(buffer));
        if (atoi(buffer) != 1) {
            printf("Operacao cancelada.\n");
            return;
        }
        idx_paciente = cadastrar_paciente();
        if (idx_paciente == -1) {
            printf("Cadastro nao concluido. Operacao cancelada.\n");
            return;
        }
    }

    p = paciente_obter(idx_paciente);
    printf("Paciente selecionado: %s\n", p->nome);

    /* Passo 2: selecionar especialidade */
    printf("\nEspecialidade:\n");
    for (i = 0; i < NUM_ESPECIALIDADES; i++) {
        printf("%d. %s\n", i + 1, ESPECIALIDADES[i]);
    }
    printf("Opcao (1-%d): ", NUM_ESPECIALIDADES);
    ler_linha(buffer, sizeof(buffer));
    opcao_esp = atoi(buffer);

    if (opcao_esp < 1 || opcao_esp > NUM_ESPECIALIDADES) {
        printf("Erro: opcao de especialidade invalida. Operacao cancelada.\n");
        return;
    }

    /* Passo 3: selecionar medico da especialidade */
    total_medicos = buscar_medicos_especialidade(ESPECIALIDADES[opcao_esp - 1],
                                                  resultados_medicos, MAX_MEDICOS);
    if (total_medicos == 0) {
        printf("Nenhum medico disponivel para %s. Operacao cancelada.\n",
               ESPECIALIDADES[opcao_esp - 1]);
        return;
    }

    printf("\nMedicos disponiveis para %s:\n", ESPECIALIDADES[opcao_esp - 1]);
    printf("%-12s %-30s\n", "CRM", "Nome");
    printf("------------------------------------------\n");
    for (i = 0; i < total_medicos; i++) {
        int mi = resultados_medicos[i];

        printf("%-12s %-30s\n", medicos[mi].crm, medicos[mi].nome);
    }

    printf("Informe o CRM do medico escolhido: ");
    ler_linha(crm, sizeof(crm));

    idx_medico = buscar_medico_crm(crm);
    if (idx_medico == -1) {
        printf("Erro: CRM nao encontrado. Operacao cancelada.\n");
        return;
    }

    encontrado = 0;
    for (i = 0; i < total_medicos; i++) {
        if (resultados_medicos[i] == idx_medico) {
            encontrado = 1;
            break;
        }
    }
    if (!encontrado) {
        printf("Erro: o CRM informado nao pertence a especialidade escolhida. Operacao cancelada.\n");
        return;
    }

    /* Passo 4: queixa principal */
    printf("\nQueixa principal (ate 200 caracteres): ");
    ler_linha(queixa, sizeof(queixa));

    /* Passo 5: data */
    printf("Data da consulta (DD/MM/AAAA): ");
    ler_linha(data, sizeof(data));

    /* Passo 6: localizar/criar grade e selecionar slot */
    grade_idx = grade_buscar(idx_medico, data);
    if (grade_idx == -1) {
        grade_idx = grade_criar(idx_medico, data);
        if (grade_idx == -1) {
            printf("Erro: data invalida ou nao foi possivel criar a grade para esse medico/data.\n");
            return;
        }
    }

    if (grades[grade_idx].num_slots == 0) {
        printf("Erro: o medico nao tem disponibilidade nessa data.\n");
        return;
    }

    slot_escolhido = -1;
    grade_selecionar_slot_terminal(grade_idx, &slot_escolhido);

    if (slot_escolhido == -1) {
        return;
    }

    /* Passo 7: resumo de pre-visualizacao (protocolo nao e incrementado aqui) */
    printf("\n=== RESUMO DO AGENDAMENTO ===\n");
    printf("Paciente..: %s (CPF: %s)\n", p->nome, p->cpf);
    printf("Medico....: %s (CRM: %s)\n",
           medicos[idx_medico].nome, medicos[idx_medico].crm);
    printf("Especialid: %s\n", medicos[idx_medico].especialidade);
    printf("Queixa....: %s\n", queixa);
    printf("Data......: %s\n", data);
    printf("Horario...: %s\n", grades[grade_idx].slots[slot_escolhido].hora);
    printf("Protocolo.: %d (provisorio)\n", proximo_protocolo);

    /* Passo 8: confirmacao */
    printf("\nConfirmar agendamento?\n1. Sim\n0. Nao\nOpcao: ");
    ler_linha(buffer, sizeof(buffer));
    if (atoi(buffer) != 1) {
        printf("Agendamento nao realizado.\n");
        return;
    }

    resultado = agendamento_criar(idx_paciente, idx_medico, data,
                                   slot_escolhido, queixa, &protocolo_saida);
    switch (resultado) {
        case AGENDAMENTO_OK:
            printf("Agendamento realizado com sucesso! Protocolo: %d\n", protocolo_saida);
            break;
        case AGENDAMENTO_ERRO_CAPACIDADE_MAXIMA:
            printf("Erro: capacidade maxima de agendamentos atingida.\n");
            break;
        case AGENDAMENTO_ERRO_PACIENTE_INVALIDO:
            printf("Erro: paciente invalido.\n");
            break;
        case AGENDAMENTO_ERRO_MEDICO_INVALIDO:
            printf("Erro: medico invalido.\n");
            break;
        case AGENDAMENTO_ERRO_GRADE_OU_DATA_INVALIDA:
            printf("Erro: data invalida ou grade nao pode ser criada.\n");
            break;
        case AGENDAMENTO_ERRO_SLOT_INVALIDO:
            printf("Erro: slot invalido.\n");
            break;
        case AGENDAMENTO_ERRO_SLOT_OCUPADO:
            printf("Erro: o slot foi ocupado por outro atendimento. Tente um horario diferente.\n");
            break;
        case AGENDAMENTO_ERRO_QUEIXA_VAZIA:
            printf("Erro: a queixa nao pode ser vazia.\n");
            break;
        case AGENDAMENTO_ERRO_CONFLITO_MESMO_DIA:
            printf("Erro: este paciente ja possui agendamento ativo nessa data.\n");
            break;
        default:
            printf("Erro: falha desconhecida ao criar agendamento.\n");
    }
}

void agendamento_cancelar_terminal(void) {
    char buffer[32];
    int opcao;
    int idx_agendamento = -1;
    ResultadoAgendamento resultado;

    printf("\n=== CANCELAR AGENDAMENTO ===\n");
    printf("Buscar por:\n");
    printf("1. Protocolo\n");
    printf("2. CPF do paciente\n");
    printf("Opcao: ");
    ler_linha(buffer, sizeof(buffer));
    opcao = atoi(buffer);

    if (opcao == 1) {
        int protocolo;

        printf("Numero do protocolo: ");
        ler_linha(buffer, sizeof(buffer));
        protocolo = atoi(buffer);

        idx_agendamento = buscar_agendamento_protocolo(protocolo);
        if (idx_agendamento == -1) {
            printf("Erro: protocolo nao encontrado.\n");
            return;
        }
    } else if (opcao == 2) {
        char cpf[64];
        int idx_paciente;
        int resultados[MAX_AGENDAMENTOS];
        int total;
        int protocolo_escolhido;
        int i;

        printf("CPF do paciente (11 digitos): ");
        ler_linha(cpf, sizeof(cpf));

        idx_paciente = buscar_paciente_cpf(cpf);
        if (idx_paciente == -1) {
            printf("Erro: paciente nao encontrado.\n");
            return;
        }

        total = buscar_agendamentos_paciente(idx_paciente, 1, resultados, MAX_AGENDAMENTOS);
        if (total == 0) {
            printf("Nenhum agendamento ativo encontrado para esse paciente.\n");
            return;
        }

        printf("\nAgendamentos ativos de %s:\n", pacientes[idx_paciente].nome);
        printf("%-10s %-12s %-12s %-8s %s\n",
               "Protocolo", "CRM medico", "Data", "Hora", "Queixa");
        printf("------------------------------------------------------------------\n");
        for (i = 0; i < total; i++) {
            const Agendamento *ag = agendamento_obter(resultados[i]);
            int gi = grade_buscar(ag->idMedico, ag->data);
            const char *hora = "?";

            if (gi != -1 && ag->slot >= 0 && ag->slot < grades[gi].num_slots) {
                hora = grades[gi].slots[ag->slot].hora;
            }
            printf("%-10d %-12s %-12s %-8s %.40s\n",
                   ag->protocolo,
                   medicos[ag->idMedico].crm,
                   ag->data,
                   hora,
                   ag->queixa);
        }

        printf("Informe o protocolo para cancelar: ");
        ler_linha(buffer, sizeof(buffer));
        protocolo_escolhido = atoi(buffer);

        idx_agendamento = buscar_agendamento_protocolo(protocolo_escolhido);
        if (idx_agendamento == -1) {
            printf("Erro: protocolo nao encontrado.\n");
            return;
        }
        if (agendamentos[idx_agendamento].idPaciente != idx_paciente) {
            printf("Erro: o protocolo informado nao pertence a esse paciente.\n");
            return;
        }
    } else {
        printf("Erro: opcao invalida.\n");
        return;
    }

    printf("\n=== DADOS DO AGENDAMENTO ===\n");
    exibir_agendamento(idx_agendamento);

    if (agendamentos[idx_agendamento].cancelado) {
        printf("Erro: este agendamento ja esta cancelado.\n");
        return;
    }

    printf("\nConfirmar cancelamento?\n1. Sim\n0. Nao\nOpcao: ");
    ler_linha(buffer, sizeof(buffer));
    if (atoi(buffer) != 1) {
        printf("Cancelamento nao realizado.\n");
        return;
    }

    resultado = agendamento_cancelar(idx_agendamento);
    switch (resultado) {
        case AGENDAMENTO_OK:
            printf("Agendamento cancelado com sucesso. O slot foi liberado.\n");
            break;
        case AGENDAMENTO_ERRO_JA_CANCELADO:
            printf("Erro: este agendamento ja estava cancelado.\n");
            break;
        case AGENDAMENTO_ERRO_PROTOCOLO_NAO_ENCONTRADO:
            printf("Erro: agendamento nao encontrado.\n");
            break;
        default:
            printf("Erro: falha ao cancelar agendamento.\n");
    }
}

/* =================== CAMADA DE PERSISTENCIA (stub) =================== */

int agendamento_carregar(void) {
    num_agendamentos = 0;
    proximo_protocolo = 1;
    return 0;
}

int agendamento_salvar(void) {
    return 1;
}
