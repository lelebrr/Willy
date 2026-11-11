#ifndef __BENCHMARK_MANAGER_H__
#define __BENCHMARK_MANAGER_H__

#include "IModule.h"
#include "SystemModel.h"
#include "SystemView.h"
#include "advanced_logger.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <array>
#include <atomic>
#include <functional>
#include <map>
#include <memory>

// Forward declarations to reduce compile-time dependencies
class SystemModel;
class SystemView;

/**
 * @brief Gerenciador de Benchmarking e Profiling para Willy Firmware
 *
 * O BenchmarkManager implementa sistema completo de benchmarking automatizado,
 * profiling de performance e análise de métricas para todos os módulos do
 * sistema. Permite execução automática de testes de performance, medição de
 * CPU, memória e latência em operações críticas, além de geração de relatórios
 * detalhados.
 *
 * Capacidades principais:
 * - Execução automática de benchmarks
 * - Profiling de CPU, memória e latência
 * - Métricas específicas para WiFi, RF, RFID, ML
 * - Sistema de relatórios de performance
 * - Testes de stress automatizados
 * - Integração com sistema de logging
 * - Comandos seriais para controle remoto
 *
 * @note Requer hardware ESP32-S3 para medições precisas
 * @note Integrado com AdvancedLogger para análise histórica
 *
 * @author Willy Firmware Team
 * @version 1.0.0
 * @date 2024
 *
 * @see IModule Interface base implementada
 * @see AdvancedLogger Para logging integrado
 * @see SystemModel Para acesso a configuração
 */

// Constantes otimizadas
constexpr size_t MAX_BENCHMARK_RESULTS = 500;  // Buffer circular limitado
constexpr size_t MAX_OPERATION_NAME_SIZE = 64; // Nome da operação limitado
constexpr uint32_t BENCHMARK_TIMEOUT_MS =
    30000; // Timeout de 30s para benchmarks
constexpr uint32_t MAX_STRESS_TEST_ITERATIONS = 1000; // Limite de iterações

class BenchmarkManager : public IModule {
public:
  static BenchmarkManager *getInstance() { return instance_; }
  /**
   * @brief Tipos de métricas suportadas
   */
  enum class MetricType {
    CPU_USAGE,
    MEMORY_USAGE,
    LATENCY,
    THROUGHPUT,
    WIFI_SCAN_TIME,
    WIFI_CONNECT_TIME,
    RF_TRANSMIT_TIME,
    RFID_READ_TIME,
    ML_INFERENCE_TIME,
    POWER_CONSUMPTION
  };

  /**
   * @brief Estrutura para armazenar resultado de benchmark
   */
  struct BenchmarkResult {
    uint32_t timestamp;
    MetricType type;
    String operation;
    uint32_t duration_us;
    size_t memory_used;
    float cpu_usage_percent;
    bool success;
    JsonDocument extra_data;
  };

  /**
   * @brief Configuração do sistema de benchmarking
   */
  struct BenchmarkConfig {
    bool enabled = true;
    bool auto_run = false;
    uint32_t interval_ms = 60000; // 1 minuto
    bool log_to_sd = true;
    bool generate_reports = true;
    uint32_t max_results = 1000;
    std::map<MetricType, bool> enabled_metrics;
  };

  /**
   * @brief Função callback para testes customizados
   */
  using BenchmarkCallback = std::function<bool(BenchmarkResult &)>;

  /**
   * @brief Construtor do BenchmarkManager
   */
  BenchmarkManager(std::shared_ptr<SystemModel> model,
                   std::shared_ptr<SystemView> view);

  /**
   * @brief Destrutor
   */
  ~BenchmarkManager() override = default;

  // Implementação da interface IModule
  bool init() override;
  void deinit() override;
  void process() override;
  String getName() const override { return "BenchmarkManager"; }
  bool isActive() const override { return true; }
  int getPriority() const override { return 5; }
  bool executeCommand(const String &command, JsonDocument &result) override {
    (void)command;
    (void)result;
    return false;
  }

  /**
   * @brief Executa benchmark específico
   */
  bool runBenchmark(const String &benchmark_name);

  /**
   * @brief Registra nova métrica
   */
  void registerMetric(MetricType type, const String &operation,
                      uint32_t duration_us, size_t memory_used = 0,
                      float cpu_usage = 0.0f, bool success = true);

  /**
   * @brief Inicia profiling de operação
   */
  uint32_t startProfiling(const String &operation);

  /**
   * @brief Finaliza profiling e registra resultado
   */
  void endProfiling(uint32_t start_time, MetricType type,
                    const String &operation, bool success = true);

  /**
   * @brief Executa teste de stress
   */
  bool runStressTest(const String &test_name, uint32_t duration_ms,
                     uint32_t iterations = 0);

  /**
   * @brief Gera relatório de performance
   */
  String generateReport(bool include_historical = false);

  /**
   * @brief Limpa resultados antigos
   */
  void clearOldResults(uint32_t max_age_ms = 86400000); // 24h default

  /**
   * @brief Configura sistema de benchmarking
   */
  void setConfig(const BenchmarkConfig &config);

  /**
   * @brief Obtém configuração atual
   */
  BenchmarkConfig getConfig() const;

  /**
   * @brief Registra callback de benchmark customizado
   */
  void registerCustomBenchmark(const String &name, BenchmarkCallback callback);

  /**
   * @brief Processa comandos seriais
   */
  void handleSerialCommand(const String &command,
                           const std::vector<String> &args);

  /**
   * @brief Obtém estatísticas gerais
   */
  JsonDocument getStatistics();

private:
  std::shared_ptr<SystemModel> model_;
  std::shared_ptr<SystemView> view_;
  BenchmarkConfig config_;
  std::vector<BenchmarkResult> results_;
  std::map<String, BenchmarkCallback> custom_benchmarks_;
  uint32_t last_run_time_;
  bool is_running_;

  static BenchmarkManager *instance_;

  // Métodos auxiliares
  void runAutomatedBenchmarks();
  void logResult(const BenchmarkResult &result);
  float measureCPUUsage();
  size_t measureMemoryUsage();
  uint32_t measureLatency(std::function<void()> operation);
  void saveResultsToSD();
  void loadResultsFromSD();

  // Benchmarks específicos dos módulos
  bool benchmarkWiFiScan();
  bool benchmarkRFTransmission();
  bool benchmarkRFIDReading();
  bool benchmarkMLInference();
  bool benchmarkMemoryStress();
  bool benchmarkCPUStress();
};

#endif // __BENCHMARK_MANAGER_H__
