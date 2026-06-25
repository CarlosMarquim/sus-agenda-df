#!/bin/bash
# SUS-Agenda DF — Script de demo para apresentacao 26/06/2026
# Compila o sistema, carrega todos os dados e deixa o terminal interativo.
#
# Uso: ./demo.sh
# Ou no Windows com Git Bash: bash demo.sh
#
# O sistema inicia ja populado com:
#   - 10 medicos (2 por especialidade)
#   - 10 pacientes com historico
#   - 20 agendamentos (11 historicos + 5 do dia 26/06 + 4 futuros)
#   - 2 cancelamentos no historico
#
# Apos o carregamento, o terminal fica interativo para demonstracao ao vivo.

set -e

echo "=== Compilando SUS-Agenda DF ==="
make

echo ""
echo "=== Carregando dados de demonstracao ==="
echo "    10 medicos, 10 pacientes, 20 agendamentos..."
echo "    Aguarde, o sistema abrira no menu principal."
echo ""

cat inputs_populacao.txt - | ./sus-agenda
