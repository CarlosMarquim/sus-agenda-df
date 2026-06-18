CC = gcc
CFLAGS = -Wall -Wextra -Iinclude
TARGET = sus-agenda
SRCS = src/main.c src/paciente.c src/medico.c src/slot.c src/menu.c src/agendamento.c

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

.PHONY: all clean
