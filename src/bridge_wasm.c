#include <stdio.h>
#include <string.h>

#include "bridge_wasm.h"
#include "paciente.h"
#include "medico.h"
#include "slot.h"
#include "agendamento.h"

#define BUF_SIZE 16384
static char buf[BUF_SIZE];

static void escape_json(char *dst, int max, const char *src) {
    int i = 0, j = 0;
    if (src == NULL) { dst[0] = '\0'; return; }
    while (src[i] != '\0' && j < max - 2) {
        if (src[i] == '"' || src[i] == '\\') {
            dst[j++] = '\\';
        }
        dst[j++] = src[i++];
    }
    dst[j] = '\0';
}

const char *bridge_paciente_cadastrar(const char *cpf, const char *nome,
                                       const char *data_nascimento, const char *telefone) {
    int idx;
    ResultadoPaciente r = paciente_registrar(cpf, nome, data_nascimento, telefone, &idx);
    if (r == PACIENTE_OK) {
        snprintf(buf, BUF_SIZE, "{\"ok\":true,\"idx\":%d}", idx);
    } else {
        const char *msg = "Erro desconhecido";
        switch (r) {
            case PACIENTE_ERRO_CAPACIDADE_MAXIMA: msg = "Capacidade maxima de pacientes atingida"; break;
            case PACIENTE_ERRO_CPF_FORMATO_INVALIDO: msg = "CPF deve conter 11 digitos numericos"; break;
            case PACIENTE_ERRO_CPF_DUPLICADO: msg = "CPF ja cadastrado"; break;
            case PACIENTE_ERRO_NOME_VAZIO: msg = "Nome nao pode ser vazio"; break;
            case PACIENTE_ERRO_DATA_VAZIA: msg = "Data de nascimento nao pode ser vazia"; break;
            case PACIENTE_ERRO_DATA_FORMATO_INVALIDO: msg = "Data deve estar no formato DD/MM/AAAA"; break;
            case PACIENTE_ERRO_TELEFONE_VAZIO: msg = "Telefone nao pode ser vazio"; break;
            default: break;
        }
        snprintf(buf, BUF_SIZE, "{\"ok\":false,\"erro\":\"%s\"}", msg);
    }
    return buf;
}

const char *bridge_paciente_buscar_cpf(const char *cpf) {
    int idx = buscar_paciente_cpf(cpf);
    if (idx == -1) {
        snprintf(buf, BUF_SIZE, "{\"encontrado\":false}");
        return buf;
    }
    const Paciente *p = paciente_obter(idx);
    char nome_esc[200], tel_esc[32];
    escape_json(nome_esc, sizeof(nome_esc), p->nome);
    escape_json(tel_esc, sizeof(tel_esc), p->telefone);
    snprintf(buf, BUF_SIZE,
        "{\"encontrado\":true,\"idx\":%d,\"cpf\":\"%s\",\"nome\":\"%s\","
        "\"data_nascimento\":\"%s\",\"telefone\":\"%s\",\"num_historico\":%d}",
        idx, p->cpf, nome_esc, p->data_nascimento, tel_esc, p->num_historico);
    return buf;
}

const char *bridge_paciente_listar(void) {
    int i, pos = 0;
    pos += snprintf(buf + pos, BUF_SIZE - pos, "[");
    for (i = 0; i < num_pacientes; i++) {
        char nome_esc[200];
        escape_json(nome_esc, sizeof(nome_esc), pacientes[i].nome);
        if (i > 0) pos += snprintf(buf + pos, BUF_SIZE - pos, ",");
        pos += snprintf(buf + pos, BUF_SIZE - pos,
            "{\"idx\":%d,\"cpf\":\"%s\",\"nome\":\"%s\","
            "\"data_nascimento\":\"%s\",\"telefone\":\"%s\",\"num_historico\":%d}",
            i, pacientes[i].cpf, nome_esc,
            pacientes[i].data_nascimento, pacientes[i].telefone,
            pacientes[i].num_historico);
    }
    pos += snprintf(buf + pos, BUF_SIZE - pos, "]");
    return buf;
}

const char *bridge_medico_cadastrar(const char *crm, const char *nome, int idx_especialidade) {
    int idx;
    ResultadoMedico r;
    if (idx_especialidade < 0 || idx_especialidade >= NUM_ESPECIALIDADES) {
        snprintf(buf, BUF_SIZE, "{\"ok\":false,\"erro\":\"Especialidade invalida\"}");
        return buf;
    }
    r = medico_registrar(crm, nome, ESPECIALIDADES[idx_especialidade], &idx);
    if (r == MEDICO_OK) {
        snprintf(buf, BUF_SIZE, "{\"ok\":true,\"idx\":%d}", idx);
    } else {
        const char *msg = "Erro desconhecido";
        switch (r) {
            case MEDICO_ERRO_CAPACIDADE_MAXIMA: msg = "Capacidade maxima de medicos atingida"; break;
            case MEDICO_ERRO_CRM_VAZIO: msg = "CRM nao pode ser vazio"; break;
            case MEDICO_ERRO_CRM_DUPLICADO: msg = "CRM ja cadastrado"; break;
            case MEDICO_ERRO_NOME_VAZIO: msg = "Nome nao pode ser vazio"; break;
            case MEDICO_ERRO_ESPECIALIDADE_VAZIA: msg = "Especialidade nao pode ser vazia"; break;
            case MEDICO_ERRO_ESPECIALIDADE_INVALIDA: msg = "Especialidade invalida"; break;
            default: break;
        }
        snprintf(buf, BUF_SIZE, "{\"ok\":false,\"erro\":\"%s\"}", msg);
    }
    return buf;
}

const char *bridge_medico_listar(void) {
    int i, pos = 0;
    pos += snprintf(buf + pos, BUF_SIZE - pos, "[");
    for (i = 0; i < num_medicos; i++) {
        char nome_esc[200];
        escape_json(nome_esc, sizeof(nome_esc), medicos[i].nome);
        if (i > 0) pos += snprintf(buf + pos, BUF_SIZE - pos, ",");
        pos += snprintf(buf + pos, BUF_SIZE - pos,
            "{\"idx\":%d,\"crm\":\"%s\",\"nome\":\"%s\",\"especialidade\":\"%s\"}",
            i, medicos[i].crm, nome_esc, medicos[i].especialidade);
    }
    pos += snprintf(buf + pos, BUF_SIZE - pos, "]");
    return buf;
}

const char *bridge_medico_buscar_crm(const char *crm) {
    int idx = buscar_medico_crm(crm);
    if (idx == -1) {
        snprintf(buf, BUF_SIZE, "{\"encontrado\":false}");
        return buf;
    }
    const Medico *m = medico_obter(idx);
    char nome_esc[200];
    escape_json(nome_esc, sizeof(nome_esc), m->nome);
    snprintf(buf, BUF_SIZE,
        "{\"encontrado\":true,\"idx\":%d,\"crm\":\"%s\",\"nome\":\"%s\",\"especialidade\":\"%s\"}",
        idx, m->crm, nome_esc, m->especialidade);
    return buf;
}

const char *bridge_medico_buscar_especialidade(int idx_especialidade) {
    int resultados[MAX_MEDICOS];
    int total, i, pos = 0;
    if (idx_especialidade < 0 || idx_especialidade >= NUM_ESPECIALIDADES) {
        snprintf(buf, BUF_SIZE, "[]");
        return buf;
    }
    total = buscar_medicos_especialidade(ESPECIALIDADES[idx_especialidade], resultados, MAX_MEDICOS);
    pos += snprintf(buf + pos, BUF_SIZE - pos, "[");
    for (i = 0; i < total; i++) {
        int mi = resultados[i];
        char nome_esc[200];
        escape_json(nome_esc, sizeof(nome_esc), medicos[mi].nome);
        if (i > 0) pos += snprintf(buf + pos, BUF_SIZE - pos, ",");
        pos += snprintf(buf + pos, BUF_SIZE - pos,
            "{\"idx\":%d,\"crm\":\"%s\",\"nome\":\"%s\",\"especialidade\":\"%s\"}",
            mi, medicos[mi].crm, nome_esc, medicos[mi].especialidade);
    }
    pos += snprintf(buf + pos, BUF_SIZE - pos, "]");
    return buf;
}

const char *bridge_medico_disponibilidade_obter(const char *crm) {
    int idx = buscar_medico_crm(crm);
    int dia, turno, pos = 0;
    if (idx == -1) {
        snprintf(buf, BUF_SIZE, "{\"ok\":false,\"erro\":\"Medico nao encontrado\"}");
        return buf;
    }
    pos += snprintf(buf + pos, BUF_SIZE - pos, "{\"ok\":true,\"disp\":[");
    for (dia = 0; dia < 7; dia++) {
        if (dia > 0) pos += snprintf(buf + pos, BUF_SIZE - pos, ",");
        pos += snprintf(buf + pos, BUF_SIZE - pos, "[%d,%d]",
            medicos[idx].disponibilidade[dia][TURNO_MANHA],
            medicos[idx].disponibilidade[dia][TURNO_TARDE]);
    }
    pos += snprintf(buf + pos, BUF_SIZE - pos, "]}");
    return buf;
}

const char *bridge_medico_disponibilidade_alternar(const char *crm, int dia, int turno) {
    int idx = buscar_medico_crm(crm);
    if (idx == -1) {
        snprintf(buf, BUF_SIZE, "{\"ok\":false,\"erro\":\"Medico nao encontrado\"}");
        return buf;
    }
    if (medico_alternar_disponibilidade(idx, dia, turno) != 0) {
        snprintf(buf, BUF_SIZE, "{\"ok\":false,\"erro\":\"Dia ou turno invalido\"}");
        return buf;
    }
    return bridge_medico_disponibilidade_obter(crm);
}

const char *bridge_grade_obter_ou_criar(const char *crm, const char *data) {
    int medico_idx = buscar_medico_crm(crm);
    int grade_idx, s, pos = 0;
    if (medico_idx == -1) {
        snprintf(buf, BUF_SIZE, "{\"ok\":false,\"erro\":\"Medico nao encontrado\"}");
        return buf;
    }
    grade_idx = grade_buscar(medico_idx, data);
    if (grade_idx == -1) {
        grade_idx = grade_criar(medico_idx, data);
    }
    if (grade_idx == -1) {
        snprintf(buf, BUF_SIZE, "{\"ok\":false,\"erro\":\"Data invalida ou grade nao pode ser criada\"}");
        return buf;
    }
    pos += snprintf(buf + pos, BUF_SIZE - pos,
        "{\"ok\":true,\"grade_idx\":%d,\"num_slots\":%d,\"slots\":[",
        grade_idx, grades[grade_idx].num_slots);
    for (s = 0; s < grades[grade_idx].num_slots; s++) {
        const Slot *sl = &grades[grade_idx].slots[s];
        if (s > 0) pos += snprintf(buf + pos, BUF_SIZE - pos, ",");
        pos += snprintf(buf + pos, BUF_SIZE - pos,
            "{\"idx\":%d,\"hora\":\"%s\",\"ocupado\":%d", s, sl->hora, sl->ocupado);
        if (sl->ocupado && sl->paciente_idx >= 0) {
            const Paciente *p = paciente_obter(sl->paciente_idx);
            char nome_esc[200];
            if (p) escape_json(nome_esc, sizeof(nome_esc), p->nome);
            else nome_esc[0] = '\0';
            pos += snprintf(buf + pos, BUF_SIZE - pos,
                ",\"paciente\":\"%s\"", nome_esc);
            if (sl->agendamento_idx >= 0) {
                const Agendamento *ag = agendamento_obter(sl->agendamento_idx);
                if (ag) {
                    char queixa_esc[420];
                    escape_json(queixa_esc, sizeof(queixa_esc), ag->queixa);
                    pos += snprintf(buf + pos, BUF_SIZE - pos,
                        ",\"queixa\":\"%s\",\"protocolo\":%d", queixa_esc, ag->protocolo);
                }
            }
        }
        pos += snprintf(buf + pos, BUF_SIZE - pos, "}");
    }
    pos += snprintf(buf + pos, BUF_SIZE - pos, "]}");
    return buf;
}

const char *bridge_agendamento_criar(const char *cpf, const char *crm, const char *data,
                                      int slot_idx, const char *queixa) {
    int idx_pac, idx_med, protocolo;
    ResultadoAgendamento r;

    idx_pac = buscar_paciente_cpf(cpf);
    if (idx_pac == -1) {
        snprintf(buf, BUF_SIZE, "{\"ok\":false,\"erro\":\"Paciente nao encontrado\"}");
        return buf;
    }
    idx_med = buscar_medico_crm(crm);
    if (idx_med == -1) {
        snprintf(buf, BUF_SIZE, "{\"ok\":false,\"erro\":\"Medico nao encontrado\"}");
        return buf;
    }
    r = agendamento_criar(idx_pac, idx_med, data, slot_idx, queixa, &protocolo);
    if (r == AGENDAMENTO_OK) {
        snprintf(buf, BUF_SIZE, "{\"ok\":true,\"protocolo\":%d}", protocolo);
    } else {
        const char *msg = "Erro desconhecido";
        switch (r) {
            case AGENDAMENTO_ERRO_CAPACIDADE_MAXIMA: msg = "Capacidade maxima de agendamentos atingida"; break;
            case AGENDAMENTO_ERRO_PACIENTE_INVALIDO: msg = "Paciente invalido"; break;
            case AGENDAMENTO_ERRO_MEDICO_INVALIDO: msg = "Medico invalido"; break;
            case AGENDAMENTO_ERRO_GRADE_OU_DATA_INVALIDA: msg = "Data invalida ou grade nao pode ser criada"; break;
            case AGENDAMENTO_ERRO_SLOT_INVALIDO: msg = "Slot invalido"; break;
            case AGENDAMENTO_ERRO_SLOT_OCUPADO: msg = "Slot ja ocupado por outro atendimento"; break;
            case AGENDAMENTO_ERRO_QUEIXA_VAZIA: msg = "Queixa nao pode ser vazia"; break;
            case AGENDAMENTO_ERRO_CONFLITO_MESMO_DIA: msg = "Paciente ja possui agendamento ativo nessa data"; break;
            default: break;
        }
        snprintf(buf, BUF_SIZE, "{\"ok\":false,\"erro\":\"%s\"}", msg);
    }
    return buf;
}

const char *bridge_agendamento_cancelar(int protocolo) {
    int idx = buscar_agendamento_protocolo(protocolo);
    ResultadoAgendamento r;
    if (idx == -1) {
        snprintf(buf, BUF_SIZE, "{\"ok\":false,\"erro\":\"Protocolo nao encontrado\"}");
        return buf;
    }
    r = agendamento_cancelar(idx);
    if (r == AGENDAMENTO_OK) {
        snprintf(buf, BUF_SIZE, "{\"ok\":true}");
    } else if (r == AGENDAMENTO_ERRO_JA_CANCELADO) {
        snprintf(buf, BUF_SIZE, "{\"ok\":false,\"erro\":\"Agendamento ja cancelado\"}");
    } else {
        snprintf(buf, BUF_SIZE, "{\"ok\":false,\"erro\":\"Erro ao cancelar\"}");
    }
    return buf;
}

const char *bridge_agendamento_buscar_protocolo(int protocolo) {
    int idx = buscar_agendamento_protocolo(protocolo);
    if (idx == -1) {
        snprintf(buf, BUF_SIZE, "{\"encontrado\":false}");
        return buf;
    }
    const Agendamento *ag = agendamento_obter(idx);
    char queixa_esc[420];
    escape_json(queixa_esc, sizeof(queixa_esc), ag->queixa);

    int gi = grade_buscar(ag->idMedico, ag->data);
    const char *hora = "?";
    if (gi != -1 && ag->slot >= 0 && ag->slot < grades[gi].num_slots)
        hora = grades[gi].slots[ag->slot].hora;

    char nome_pac[200], nome_med[200];
    const Paciente *p = paciente_obter(ag->idPaciente);
    escape_json(nome_pac, sizeof(nome_pac), p ? p->nome : "?");
    const Medico *m = medico_obter(ag->idMedico);
    escape_json(nome_med, sizeof(nome_med), m ? m->nome : "?");

    snprintf(buf, BUF_SIZE,
        "{\"encontrado\":true,\"protocolo\":%d,\"paciente\":\"%s\",\"cpf\":\"%s\","
        "\"medico\":\"%s\",\"crm\":\"%s\",\"especialidade\":\"%s\","
        "\"data\":\"%s\",\"hora\":\"%s\",\"queixa\":\"%s\","
        "\"cancelado\":%d}",
        ag->protocolo, nome_pac, p ? p->cpf : "?",
        nome_med, m ? m->crm : "?", m ? m->especialidade : "?",
        ag->data, hora, queixa_esc, ag->cancelado);
    return buf;
}

const char *bridge_agendamento_listar_paciente(const char *cpf, int apenas_ativos) {
    int idx_pac = buscar_paciente_cpf(cpf);
    int resultados[MAX_AGENDAMENTOS];
    int total, i, pos = 0;
    if (idx_pac == -1) {
        snprintf(buf, BUF_SIZE, "{\"ok\":false,\"erro\":\"Paciente nao encontrado\"}");
        return buf;
    }
    total = buscar_agendamentos_paciente(idx_pac, apenas_ativos, resultados, MAX_AGENDAMENTOS);
    pos += snprintf(buf + pos, BUF_SIZE - pos, "{\"ok\":true,\"total\":%d,\"agendamentos\":[", total);
    for (i = 0; i < total; i++) {
        const Agendamento *ag = agendamento_obter(resultados[i]);
        if (!ag) continue;
        char queixa_esc[420];
        escape_json(queixa_esc, sizeof(queixa_esc), ag->queixa);
        int gi = grade_buscar(ag->idMedico, ag->data);
        const char *hora = "?";
        if (gi != -1 && ag->slot >= 0 && ag->slot < grades[gi].num_slots)
            hora = grades[gi].slots[ag->slot].hora;
        if (i > 0) pos += snprintf(buf + pos, BUF_SIZE - pos, ",");
        pos += snprintf(buf + pos, BUF_SIZE - pos,
            "{\"protocolo\":%d,\"crm\":\"%s\",\"especialidade\":\"%s\","
            "\"data\":\"%s\",\"hora\":\"%s\",\"cancelado\":%d,\"queixa\":\"%s\"}",
            ag->protocolo, medicos[ag->idMedico].crm,
            medicos[ag->idMedico].especialidade,
            ag->data, hora, ag->cancelado, queixa_esc);
    }
    pos += snprintf(buf + pos, BUF_SIZE - pos, "]}");
    return buf;
}

const char *bridge_agenda_dia(const char *data) {
    int i, s, pos = 0;
    pos += snprintf(buf + pos, BUF_SIZE - pos, "[");
    for (i = 0; i < num_medicos; i++) {
        const Medico *m = medico_obter(i);
        if (!m) continue;
        int grade_idx = grade_buscar(i, data);
        if (grade_idx == -1) grade_idx = grade_criar(i, data);

        char nome_esc[200];
        escape_json(nome_esc, sizeof(nome_esc), m->nome);
        if (i > 0) pos += snprintf(buf + pos, BUF_SIZE - pos, ",");
        pos += snprintf(buf + pos, BUF_SIZE - pos,
            "{\"idx\":%d,\"crm\":\"%s\",\"nome\":\"%s\",\"especialidade\":\"%s\"",
            i, m->crm, nome_esc, m->especialidade);

        if (grade_idx == -1) {
            pos += snprintf(buf + pos, BUF_SIZE - pos, ",\"erro\":\"Grade nao disponivel\",\"slots\":[]}");
            continue;
        }
        pos += snprintf(buf + pos, BUF_SIZE - pos, ",\"num_slots\":%d,\"slots\":[",
            grades[grade_idx].num_slots);
        for (s = 0; s < grades[grade_idx].num_slots; s++) {
            const Slot *sl = &grades[grade_idx].slots[s];
            if (s > 0) pos += snprintf(buf + pos, BUF_SIZE - pos, ",");
            pos += snprintf(buf + pos, BUF_SIZE - pos,
                "{\"hora\":\"%s\",\"ocupado\":%d", sl->hora, sl->ocupado);
            if (sl->ocupado && sl->paciente_idx >= 0) {
                const Paciente *p = paciente_obter(sl->paciente_idx);
                char pn[200];
                if (p) escape_json(pn, sizeof(pn), p->nome); else pn[0] = '\0';
                pos += snprintf(buf + pos, BUF_SIZE - pos, ",\"paciente\":\"%s\"", pn);
                if (sl->agendamento_idx >= 0) {
                    const Agendamento *ag = agendamento_obter(sl->agendamento_idx);
                    if (ag) {
                        char qe[420];
                        escape_json(qe, sizeof(qe), ag->queixa);
                        pos += snprintf(buf + pos, BUF_SIZE - pos,
                            ",\"queixa\":\"%s\",\"protocolo\":%d", qe, ag->protocolo);
                    }
                }
            }
            pos += snprintf(buf + pos, BUF_SIZE - pos, "}");
        }
        pos += snprintf(buf + pos, BUF_SIZE - pos, "]}");
    }
    pos += snprintf(buf + pos, BUF_SIZE - pos, "]");
    return buf;
}

const char *bridge_especialidades_listar(void) {
    int i, pos = 0;
    pos += snprintf(buf + pos, BUF_SIZE - pos, "[");
    for (i = 0; i < NUM_ESPECIALIDADES; i++) {
        if (i > 0) pos += snprintf(buf + pos, BUF_SIZE - pos, ",");
        pos += snprintf(buf + pos, BUF_SIZE - pos, "\"%s\"", ESPECIALIDADES[i]);
    }
    pos += snprintf(buf + pos, BUF_SIZE - pos, "]");
    return buf;
}

const char *bridge_contadores(const char *data_hoje) {
    int i, ativos = 0, hoje = 0;
    for (i = 0; i < num_agendamentos; i++) {
        if (!agendamentos[i].cancelado) {
            ativos++;
            if (data_hoje && strcmp(agendamentos[i].data, data_hoje) == 0)
                hoje++;
        }
    }
    snprintf(buf, BUF_SIZE,
        "{\"pacientes\":%d,\"medicos\":%d,\"agendamentos\":%d,\"consultas_hoje\":%d}",
        num_pacientes, num_medicos, ativos, hoje);
    return buf;
}
