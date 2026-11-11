#include "benchmark_commands.h"
#include "../BenchmarkManager.h"
#include "../SecurityUtils.h"
#include "../SystemManager.h"
#include <Arduino.h>
#include <globals.h>
#include <ArduinoJson.h>
#include <vector>

// Constantes de segurança
constexpr size_t MAX_COMMAND_ARGS = 10;


/**
 * @brief Implementação dos comandos seriais para BenchmarkManager
 */

// Ponteiro global para o BenchmarkManager
static BenchmarkManager *benchmarkManager = nullptr;

/**
 * @brief Callback para comando benchmark_run
 */
uint32_t benchmarkRunCallback(cmd *c) {
  (void)c; // Unused parameter
  Command cmd(c);
  AdvancedLogger &logger = AdvancedLogger::getInstance();

  if (!benchmarkManager) {
    logger.error(LogModule::SYSTEM, "BenchmarkManager not available");
    serialDevice->println("ERROR: BenchmarkManager not available");
    return 0;
  }

  String benchmarkName = cmd.getArgument(0).getValue();
  // Validação de entrada
  if (benchmarkName.length() == 0 || benchmarkName.length() > 50) {
    logger.warning(LogModule::SYSTEM, "Invalid benchmark name length: %d",
                   benchmarkName.length());
    serialDevice->println("ERROR: Benchmark name required (1-50 chars)");
    serialDevice->println("Usage: benchmark_run <name>");
    return 0;
  }

  // Sanitizar nome (apenas alfanumérico e underscore)
  for (char ch : benchmarkName) {
    if (!isalnum(ch) && ch != '_') {
      logger.warning(LogModule::SYSTEM,
                     "Invalid character in benchmark name: %c", ch);
      serialDevice->println(
          "ERROR: Benchmark name contains invalid characters");
      return 0;
    }
  }

  logger.info(LogModule::SYSTEM, "Running benchmark: %s",
              benchmarkName.c_str());
  bool success = benchmarkManager->runBenchmark(benchmarkName);
  serialDevice->printf("Benchmark '%s' executed: %s\n", benchmarkName.c_str(),
                       success ? "SUCCESS" : "FAILED");
  logger.info(LogModule::SYSTEM, "Benchmark %s completed: %s",
              benchmarkName.c_str(), success ? "SUCCESS" : "FAILED");
  return success ? 1 : 0;
}

/**
 * @brief Callback para comando benchmark_report
 */
uint32_t benchmarkReportCallback(cmd *c) {
  (void)c; // Unused parameter
  Command cmd(c);
  AdvancedLogger &logger = AdvancedLogger::getInstance();

  if (!benchmarkManager) {
    logger.error(LogModule::SYSTEM, "BenchmarkManager not available");
    serialDevice->println("ERROR: BenchmarkManager not available");
    return 0;
  }

  bool includeHistorical = cmd.getArgument(0).getValue() == "full";
  logger.info(LogModule::SYSTEM, "Generating benchmark report (historical: %s)",
              includeHistorical ? "yes" : "no");

  String report = benchmarkManager->generateReport(includeHistorical);

  serialDevice->println("=== PERFORMANCE REPORT ===");
  serialDevice->println(report);
  serialDevice->println("==========================");
  return 1;
}

/**
 * @brief Callback para comando benchmark_stress
 */
uint32_t benchmarkStressCallback(cmd *c) {
  (void)c; // Unused parameter
  Command cmd(c);
  AdvancedLogger &logger = AdvancedLogger::getInstance();

  if (!benchmarkManager) {
    logger.error(LogModule::SYSTEM, "BenchmarkManager not available");
    serialDevice->println("ERROR: BenchmarkManager not available");
    return 0;
  }

  String testName = cmd.getArgument(0).getValue();
  String durationStr = cmd.getArgument(1).getValue();

  // Validações
  if (testName.length() == 0 || testName.length() > 50) {
    logger.warning(LogModule::SYSTEM, "Invalid test name length: %d",
                   testName.length());
    serialDevice->println("ERROR: Test name required (1-50 chars)");
    serialDevice->println("Usage: benchmark_stress <name> <duration_ms>");
    return 0;
  }

  if (durationStr.length() == 0) {
    logger.warning(LogModule::SYSTEM, "Duration not provided");
    serialDevice->println("ERROR: Duration required");
    serialDevice->println("Usage: benchmark_stress <name> <duration_ms>");
    return 0;
  }

  uint32_t duration = durationStr.toInt();
  if (duration == 0 || duration > 3600000) { // Máximo 1 hora
    logger.warning(LogModule::SYSTEM, "Invalid duration: %u", duration);
    serialDevice->println("ERROR: Invalid duration (1-3600000 ms)");
    return 0;
  }

  // Sanitizar nome
  for (char ch : testName) {
    if (!isalnum(ch) && ch != '_') {
      logger.warning(LogModule::SYSTEM, "Invalid character in test name: %c",
                     ch);
      serialDevice->println("ERROR: Test name contains invalid characters");
      return 0;
    }
  }

  logger.info(LogModule::SYSTEM, "Starting stress test '%s' for %u ms",
              testName.c_str(), duration);
  serialDevice->printf("Starting stress test '%s' for %u ms...\n",
                       testName.c_str(), duration);

  bool success = benchmarkManager->runStressTest(testName, duration);
  serialDevice->printf("Stress test '%s' completed: %s\n", testName.c_str(),
                       success ? "SUCCESS" : "FAILED");
  logger.info(LogModule::SYSTEM, "Stress test %s completed: %s",
              testName.c_str(), success ? "SUCCESS" : "FAILED");
  return success ? 1 : 0;
}

/**
 * @brief Callback para comando benchmark_clear
 */
uint32_t benchmarkClearCallback(cmd *c) {
  (void)c; // Unused parameter
  Command cmd(c);

  if (!benchmarkManager) {
    Serial.println("ERROR: BenchmarkManager not available");
    return 0;
  }

  String maxAgeStr = cmd.getArgument(0).getValue();
  uint32_t maxAge =
      maxAgeStr.length() > 0 ? maxAgeStr.toInt() : 86400000; // 24h default

  benchmarkManager->clearOldResults(maxAge);
  Serial.printf("Old results cleared (max age: %lu ms)\n", maxAge);
  return 1;
}

/**
 * @brief Callback para comando benchmark_stats
 */
uint32_t benchmarkStatsCallback(cmd *c) {
  (void)c; // Unused parameter
  Command cmd(c);
  AdvancedLogger &logger = AdvancedLogger::getInstance();

  if (!benchmarkManager) {
    logger.error(LogModule::SYSTEM, "BenchmarkManager not available");
    serialDevice->println("ERROR: BenchmarkManager not available");
    return 0;
  }

  logger.info(LogModule::SYSTEM, "Retrieving benchmark statistics");
  auto stats = benchmarkManager->getStatistics();

  serialDevice->println("=== BENCHMARK STATISTICS ===");
  serialDevice->printf("Total Results: %lu\n",
                       stats["total_results"].as<uint32_t>());
  serialDevice->printf("Memory Usage: %lu bytes\n",
                       stats["memory_usage"].as<uint32_t>());
  serialDevice->printf("Uptime: %lu ms\n", stats["uptime"].as<uint32_t>());

  if (stats["type_counts"].is<JsonObject>()) {
    serialDevice->println("Results by Type:");
    JsonObject counts = stats["type_counts"];
    for (JsonPair kv : counts) {
      serialDevice->printf("  %s: %lu\n", kv.key().c_str(),
                           kv.value().as<uint32_t>());
    }
  }
  serialDevice->println("============================");
  return 1;
}

/**
 * @brief Callback para comando benchmark_config
 */
uint32_t benchmarkConfigCallback(cmd *c) {
  (void)c; // Unused parameter
  Command cmd(c);

  if (!benchmarkManager) {
    Serial.println("ERROR: BenchmarkManager not available");
    return 0;
  }

  String param = cmd.getArgument(0).getValue();
  String value = cmd.getArgument(1).getValue();

  if (param == "auto_run") {
    auto config = benchmarkManager->getConfig();
    config.auto_run = value == "1" || value == "true";
    Serial.printf("Auto run set to: %s\n",
                  benchmarkManager->getConfig().auto_run ? "true" : "false");
  } else if (param == "interval") {
    uint32_t interval = value.toInt();
    auto config = benchmarkManager->getConfig();
    config.interval_ms = interval;
    Serial.printf("Interval set to: %lu ms\n", interval);
  } else if (param == "log_sd") {
    auto config = benchmarkManager->getConfig();
    config.log_to_sd = value == "1" || value == "true";
    Serial.printf("Log to SD set to: %s\n",
                  benchmarkManager->getConfig().log_to_sd ? "true" : "false");
  } else {
    Serial.println("Unknown parameter. Available: auto_run, interval, log_sd");
  }
  return 1;
}

/**
 * @brief Callback para comando benchmark_start_auto
 */
uint32_t benchmarkStartAutoCallback(cmd *c) {
  (void)c; // Unused parameter
  Command cmd(c);

  if (!benchmarkManager) {
    Serial.println("ERROR: BenchmarkManager not available");
    return 0;
  }

  auto config = benchmarkManager->getConfig();
  config.auto_run = true;
  Serial.println("Auto benchmark started");
  return 1;
}

/**
 * @brief Callback para comando benchmark_stop_auto
 */
uint32_t benchmarkStopAutoCallback(cmd *c) {
  (void)c; // Unused parameter
  Command cmd(c);

  if (!benchmarkManager) {
    Serial.println("ERROR: BenchmarkManager not available");
    return 0;
  }

  auto config = benchmarkManager->getConfig();
  config.auto_run = false;
  Serial.println("Auto benchmark stopped");
  return 1;
}

/**
 * @brief Inicializa comandos de benchmark
 */
void createBenchmarkCommands(SimpleCLI *cli) {
  // Obtém referência ao BenchmarkManager
  auto &systemManager = SystemManager::getInstance();
  auto benchmarkModule = systemManager.getModule("BenchmarkManager");

  if (benchmarkModule) {
    benchmarkManager = static_cast<BenchmarkManager *>(benchmarkModule);
  } else {
    Serial.println("WARNING: BenchmarkManager not found in SystemManager");
    return;
  }

  // Comando benchmark_run
  Command benchmarkRunCmd =
      cli->addCommand("benchmark_run", benchmarkRunCallback);
  benchmarkRunCmd.addArgument("name", "Benchmark name to execute");

  // Comando benchmark_report
  Command benchmarkReportCmd =
      cli->addCommand("benchmark_report", benchmarkReportCallback);
  benchmarkReportCmd.addArgument("full", "Include historical data (optional)");

  // Comando benchmark_stress
  Command benchmarkStressCmd =
      cli->addCommand("benchmark_stress", benchmarkStressCallback);
  benchmarkStressCmd.addArgument("name", "Test name");
  benchmarkStressCmd.addArgument("duration", "Duration in milliseconds");

  // Comando benchmark_clear
  Command benchmarkClearCmd =
      cli->addCommand("benchmark_clear", benchmarkClearCallback);
  benchmarkClearCmd.addArgument("max_age",
                                "Maximum age in ms (default: 86400000)");

  // Comando benchmark_stats
  Command benchmarkStatsCmd =
      cli->addCommand("benchmark_stats", benchmarkStatsCallback);

  // Comando benchmark_config
  Command benchmarkConfigCmd =
      cli->addCommand("benchmark_config", benchmarkConfigCallback);
  benchmarkConfigCmd.addArgument("param", "Parameter to configure");
  benchmarkConfigCmd.addArgument("value", "Value to set");

  // Comando benchmark_start_auto
  Command benchmarkStartAutoCmd =
      cli->addCommand("benchmark_start_auto", benchmarkStartAutoCallback);

  // Comando benchmark_stop_auto
  Command benchmarkStopAutoCmd =
      cli->addCommand("benchmark_stop_auto", benchmarkStopAutoCallback);

  Serial.println("Benchmark commands initialized");
}