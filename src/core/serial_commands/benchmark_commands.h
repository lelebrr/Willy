#ifndef __BENCHMARK_COMMANDS_H__
#define __BENCHMARK_COMMANDS_H__

#include <SimpleCLI.h>

/**
 * @brief Comandos seriais para controle do BenchmarkManager
 *
 * Implementa interface de linha de comando para executar benchmarks,
 * visualizar relatórios e controlar testes de stress remotamente.
 *
 * Comandos disponíveis:
 * - benchmark_run <nome> : Executa benchmark específico
 * - benchmark_report [full] : Gera relatório de performance
 * - benchmark_stress <nome> <duração_ms> : Executa teste de stress
 * - benchmark_clear [max_age_ms] : Limpa resultados antigos
 * - benchmark_stats : Mostra estatísticas gerais
 *
 * @author Willy Firmware Team
 * @version 1.0.0
 * @date 2024
 */

void createBenchmarkCommands(SimpleCLI *cli);

#endif // __BENCHMARK_COMMANDS_H__