#ifndef BRIDGE_WASM_H
#define BRIDGE_WASM_H

/*
 * Camada de ponte entre o frontend web (JavaScript) e o core C
 * compilado para WebAssembly via Emscripten.
 *
 * Cada funcao recebe parametros simples (inteiros, strings) e retorna
 * uma string JSON escrita em buffer estatico. O JavaScript deve copiar
 * o resultado imediatamente apos cada chamada.
 */

const char *bridge_paciente_cadastrar(const char *cpf, const char *nome,
                                       const char *data_nascimento, const char *telefone);
const char *bridge_paciente_buscar_cpf(const char *cpf);
const char *bridge_paciente_listar(void);

const char *bridge_medico_cadastrar(const char *crm, const char *nome, int idx_especialidade);
const char *bridge_medico_listar(void);
const char *bridge_medico_buscar_crm(const char *crm);
const char *bridge_medico_buscar_especialidade(int idx_especialidade);
const char *bridge_medico_disponibilidade_obter(const char *crm);
const char *bridge_medico_disponibilidade_alternar(const char *crm, int dia, int turno);

const char *bridge_grade_obter_ou_criar(const char *crm, const char *data);

const char *bridge_agendamento_criar(const char *cpf, const char *crm, const char *data,
                                      int slot_idx, const char *queixa);
const char *bridge_agendamento_cancelar(int protocolo);
const char *bridge_agendamento_buscar_protocolo(int protocolo);
const char *bridge_agendamento_listar_paciente(const char *cpf, int apenas_ativos);

const char *bridge_agenda_dia(const char *data);

const char *bridge_especialidades_listar(void);
const char *bridge_contadores(const char *data_hoje);

#endif /* BRIDGE_WASM_H */
