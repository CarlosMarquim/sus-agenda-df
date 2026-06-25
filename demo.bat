@echo off
cd /d "%~dp0"

if not exist sus-agenda.exe (
    echo Compilando SUS-Agenda DF...
    mingw32-make
    echo.
)

echo Carregando 10 medicos, 10 pacientes, 20 agendamentos...
echo O sistema vai abrir no menu principal em alguns segundos.
echo.

"C:\Program Files\Git\bin\bash.exe" -c "cat inputs_populacao.txt - | ./sus-agenda"
