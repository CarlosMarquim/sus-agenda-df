CC = gcc
CFLAGS = -Wall -Wextra -Iinclude
TARGET = sus-agenda
SRCS = src/main.c src/paciente.c src/medico.c src/slot.c src/menu.c src/agendamento.c src/agenda.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

# mingw32-make usa cmd.exe como shell no Windows (sem rm); o "make" do
# Linux/Mac usa bash (sem del). O "-" na frente ignora erro se o
# arquivo nao existir, em qualquer um dos dois casos.
ifeq ($(OS),Windows_NT)
clean:
	-del /Q $(TARGET).exe
	-del /Q $(TARGET)
else
clean:
	rm -f $(TARGET) $(TARGET).exe
endif

EMCC = emcc
WASM_SRCS = src/paciente.c src/medico.c src/slot.c src/agendamento.c src/agenda.c src/bridge_wasm.c
WASM_EXPORTS = ['_bridge_paciente_cadastrar','_bridge_paciente_buscar_cpf','_bridge_paciente_listar','_bridge_medico_cadastrar','_bridge_medico_listar','_bridge_medico_buscar_crm','_bridge_medico_buscar_especialidade','_bridge_medico_disponibilidade_obter','_bridge_medico_disponibilidade_alternar','_bridge_grade_obter_ou_criar','_bridge_agendamento_criar','_bridge_agendamento_cancelar','_bridge_agendamento_buscar_protocolo','_bridge_agendamento_listar_paciente','_bridge_agenda_dia','_bridge_especialidades_listar','_bridge_contadores']

wasm: web/sus-agenda.js

web/sus-agenda.js: $(WASM_SRCS)
	$(EMCC) $(WASM_SRCS) -Iinclude -O2 \
	  -s EXPORTED_FUNCTIONS="$(WASM_EXPORTS)" \
	  -s EXPORTED_RUNTIME_METHODS="['ccall','cwrap','UTF8ToString','stringToUTF8','lengthBytesUTF8']" \
	  -s MODULARIZE=1 -s EXPORT_NAME="SusAgendaModule" \
	  -s ALLOW_MEMORY_GROWTH=1 \
	  -o web/sus-agenda.js

.PHONY: all clean wasm
