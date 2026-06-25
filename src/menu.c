#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "menu.h"
#include "paciente.h"
#include "medico.h"
#include "slot.h"
#include "agendamento.h"
#include "agenda.h"

/* Comando de limpeza de tela conforme o sistema operacional. */
#ifdef _WIN32
#define COMANDO_LIMPAR_TELA "cls"
#else
#define COMANDO_LIMPAR_TELA "clear"
#endif

/* Descarta o restante da linha do stdin apos uma leitura com scanf,
 * evitando que sobras no buffer de entrada afetem a proxima leitura. */
static void limpar_buffer_entrada(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
        /* descarta */
    }
}

/* Limpa a tela do terminal (cls no Windows, clear em outros sistemas),
 * para que cada tela de menu comece sempre limpa. */
static void limpar_tela(void) {
    system(COMANDO_LIMPAR_TELA);
}

/* Pausa a execucao ate o atendente pressionar ENTER, garantindo que
 * ele leu o resultado da acao antes da tela ser limpa na proxima
 * iteracao do menu. */
static void pausar_e_continuar(void) {
    int c;

    printf("\nPressione ENTER para continuar...");
    while ((c = getchar()) != '\n' && c != EOF) {
        /* descarta */
    }
}

static void exibir_cabecalho(void) {
    printf("========================================\n");
    printf("        SUS-AGENDA DF - Recepcao\n");
    printf("========================================\n");
}

/* Le uma opcao numerica de menu. Retorna 1 em sucesso (valor em
 * *opcao), 0 se a entrada nao for numerica, ou -1 se EOF foi
 * detectado (quem chamar deve encerrar o programa nesse caso, em vez
 * de repetir o erro indefinidamente). */
static int ler_opcao_menu(int *opcao) {
    int leitura = scanf("%d", opcao);
    int eof_detectado = (leitura == EOF);

    limpar_buffer_entrada();

    if (eof_detectado) {
        return -1;
    }
    return (leitura == 1) ? 1 : 0;
}

static void opcao_buscar_paciente(void) {
    char cpf[64];
    int idx;

    printf("\n=== BUSCAR PACIENTE POR CPF ===\n");
    printf("Informe o CPF (11 digitos): ");
    if (fgets(cpf, sizeof(cpf), stdin) != NULL) {
        cpf[strcspn(cpf, "\n")] = '\0';
    } else {
        cpf[0] = '\0';
    }

    if (!validar_cpf_formato(cpf)) {
        printf("CPF invalido. Digite 11 digitos numericos.\n");
        return;
    }

    idx = buscar_paciente_cpf(cpf);
    if (idx == -1) {
        printf("Erro: paciente nao encontrado para o CPF informado.\n");
        return;
    }
    exibir_paciente(idx);
}

/* Localiza (ou cria, se ainda nao existir) a grade do dia de um
 * medico numa data, e a exibe no terminal. */
static void opcao_exibir_grade_dia(void) {
    char crm[32];
    char data[16];
    int medico_idx;
    int grade_idx;

    printf("\n=== EXIBIR GRADE DO DIA ===\n");

    printf("CRM do medico: ");
    if (fgets(crm, sizeof(crm), stdin) != NULL) {
        crm[strcspn(crm, "\n")] = '\0';
    } else {
        crm[0] = '\0';
    }

    medico_idx = buscar_medico_crm(crm);
    if (medico_idx == -1) {
        printf("Erro: medico nao encontrado para o CRM informado.\n");
        return;
    }

    printf("Data (DD/MM/AAAA): ");
    if (fgets(data, sizeof(data), stdin) != NULL) {
        data[strcspn(data, "\n")] = '\0';
    } else {
        data[0] = '\0';
    }
    normalizar_data(data);

    grade_idx = grade_buscar(medico_idx, data);
    if (grade_idx == -1) {
        grade_idx = grade_criar(medico_idx, data);
        if (grade_idx == -1) {
            printf("Erro: nao foi possivel criar a grade para esse medico/data.\n");
            return;
        }
    }

    grade_exibir_terminal(grade_idx);
}

/* ===================== SUBMENUS POR MODULO =====================
 *
 * Cada submenu tem seu proprio loop e roda enquanto o atendente nao
 * escolher "0. Voltar". Retornam 1 ao voltar normalmente para o hub,
 * ou 0 se EOF foi detectado (o hub deve encerrar o programa nesse
 * caso, sem desenhar mais nenhuma tela). */

static int submenu_pacientes(void) {
    int opcao;
    int resultado;

    do {
        limpar_tela();
        exibir_cabecalho();
        printf("--- PACIENTES ---\n");
        printf("1. Cadastrar paciente\n");
        printf("2. Buscar paciente por CPF\n");
        printf("3. Listar todos os pacientes\n");
        printf("0. Voltar\n");
        printf("Escolha uma opcao: ");

        resultado = ler_opcao_menu(&opcao);
        if (resultado == -1) {
            return 0;
        }
        if (resultado == 0) {
            printf("Erro: entrada invalida. Digite um numero.\n");
            pausar_e_continuar();
            opcao = -1;
            continue;
        }

        switch (opcao) {
            case 1:
                cadastrar_paciente();
                break;
            case 2:
                opcao_buscar_paciente();
                break;
            case 3:
                listar_pacientes();
                break;
            case 0:
                break;
            default:
                printf("Erro: opcao invalida. Tente novamente.\n");
        }

        if (opcao != 0) {
            pausar_e_continuar();
        }
    } while (opcao != 0);

    return 1;
}

static int submenu_medicos(void) {
    int opcao;
    int resultado;

    do {
        limpar_tela();
        exibir_cabecalho();
        printf("--- MEDICOS ---\n");
        printf("1. Cadastrar medico\n");
        printf("2. Listar medicos\n");
        printf("3. Buscar medico (CRM ou especialidade)\n");
        printf("4. Configurar disponibilidade\n");
        printf("0. Voltar\n");
        printf("Escolha uma opcao: ");

        resultado = ler_opcao_menu(&opcao);
        if (resultado == -1) {
            return 0;
        }
        if (resultado == 0) {
            printf("Erro: entrada invalida. Digite um numero.\n");
            pausar_e_continuar();
            opcao = -1;
            continue;
        }

        switch (opcao) {
            case 1:
                medico_cadastrar_terminal();
                break;
            case 2:
                medico_listar_terminal();
                break;
            case 3:
                medico_buscar_terminal();
                break;
            case 4:
                medico_configurar_disponibilidade_terminal();
                break;
            case 0:
                break;
            default:
                printf("Erro: opcao invalida. Tente novamente.\n");
        }

        if (opcao != 0) {
            pausar_e_continuar();
        }
    } while (opcao != 0);

    return 1;
}

static int submenu_agenda(void) {
    int opcao;
    int resultado;

    do {
        limpar_tela();
        exibir_cabecalho();
        printf("--- AGENDA ---\n");
        printf("1. Exibir grade do dia\n");
        printf("2. Agenda do dia (todos os medicos)\n");
        printf("3. Historico do paciente\n");
        printf("0. Voltar\n");
        printf("Escolha uma opcao: ");

        resultado = ler_opcao_menu(&opcao);
        if (resultado == -1) {
            return 0;
        }
        if (resultado == 0) {
            printf("Erro: entrada invalida. Digite um numero.\n");
            pausar_e_continuar();
            opcao = -1;
            continue;
        }

        switch (opcao) {
            case 1:
                opcao_exibir_grade_dia();
                break;
            case 2:
                agenda_dia_terminal();
                break;
            case 3:
                agenda_historico_paciente_terminal();
                break;
            case 0:
                break;
            default:
                printf("Erro: opcao invalida. Tente novamente.\n");
        }

        if (opcao != 0) {
            pausar_e_continuar();
        }
    } while (opcao != 0);

    return 1;
}

static int submenu_agendamento(void) {
    int opcao;
    int resultado;

    do {
        limpar_tela();
        exibir_cabecalho();
        printf("--- AGENDAMENTOS ---\n");
        printf("1. Criar novo agendamento\n");
        printf("2. Cancelar agendamento\n");
        printf("0. Voltar\n");
        printf("Escolha uma opcao: ");

        resultado = ler_opcao_menu(&opcao);
        if (resultado == -1) {
            return 0;
        }
        if (resultado == 0) {
            printf("Erro: entrada invalida. Digite um numero.\n");
            pausar_e_continuar();
            opcao = -1;
            continue;
        }

        switch (opcao) {
            case 1:
                agendamento_criar_terminal();
                break;
            case 2:
                agendamento_cancelar_terminal();
                break;
            case 0:
                break;
            default:
                printf("Erro: opcao invalida. Tente novamente.\n");
        }

        if (opcao != 0) {
            pausar_e_continuar();
        }
    } while (opcao != 0);

    return 1;
}

/* ===================== MENU PRINCIPAL (HUB) ===================== */

void menu_principal(void) {
    int opcao;
    int resultado;
    int continuar;

    do {
        limpar_tela();
        exibir_cabecalho();
        printf("1. Pacientes\n");
        printf("2. Medicos\n");
        printf("3. Agenda\n");
        printf("4. Agendamentos\n");
        printf("0. Sair\n");
        printf("Escolha uma opcao: ");

        resultado = ler_opcao_menu(&opcao);
        if (resultado == -1) {
            printf("\nFim da entrada detectado. Encerrando o sistema.\n");
            return;
        }
        if (resultado == 0) {
            printf("Erro: entrada invalida. Digite um numero.\n");
            pausar_e_continuar();
            opcao = -1;
            continue;
        }

        continuar = 1;
        switch (opcao) {
            case 1:
                continuar = submenu_pacientes();
                break;
            case 2:
                continuar = submenu_medicos();
                break;
            case 3:
                continuar = submenu_agenda();
                break;
            case 4:
                continuar = submenu_agendamento();
                break;
            case 0:
                printf("\nSaindo do sistema. Ate logo!\n");
                break;
            default:
                printf("Erro: opcao invalida. Tente novamente.\n");
                pausar_e_continuar();
        }

        if (!continuar) {
            printf("\nFim da entrada detectado. Encerrando o sistema.\n");
            return;
        }
    } while (opcao != 0);
}
