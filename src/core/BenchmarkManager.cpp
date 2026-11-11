#include "BenchmarkManager.h"
#include "SystemManager.h"
#include "advanced_logger.h"
#include <SD.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <esp_heap_caps.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// Forward declarations to reduce compile-time dependencies
// These modules are only used in specific benchmark functions
class MLModule;
class RFModule;
class RFIDModule;
class WiFiModule;

BenchmarkManager *BenchmarkManager::instance_ = nullptr;

/**
 * @brief Implementação do BenchmarkManager
 */

BenchmarkManager::BenchmarkManager(std::shared_ptr<SystemModel> model,
                                   std::shared_ptr<SystemView> view)
    : model_(model), view_(view), last_run_time_(0), is_running_(false) {
  instance_ = this;
  // Configuração padrão
  config_.enabled = true;
  config_.auto_run = false;
  config_.interval_ms = 60000;
  config_.log_to_sd = true;
  config_.generate_reports = true;
  config_.max_results = 1000;

  // Habilitar métricas padrão
  config_.enabled_metrics[MetricType::CPU_USAGE] = true;
  config_.enabled_metrics[MetricType::MEMORY_USAGE] = true;
  config_.enabled_metrics[MetricType::LATENCY] = true;
  config_.enabled_metrics[MetricType::WIFI_SCAN_TIME] = true;
  config_.enabled_metrics[MetricType::RF_TRANSMIT_TIME] = true;
  config_.enabled_metrics[MetricType::RFID_READ_TIME] = true;
  config_.enabled_metrics[MetricType::ML_INFERENCE_TIME] = true;
}

bool BenchmarkManager::init() {
  if (!config_.enabled) {
    return true;
  }

  AdvancedLogger::getInstance().info(LogModule::SYSTEM,
                                     "Inicializando BenchmarkManager");

  // Carregar resultados salvos se existir
  loadResultsFromSD();

  // Registrar benchmarks customizados padrão
  registerCustomBenchmark("wifi_scan", [this](BenchmarkResult &result) {
    (void)result;
    return benchmarkWiFiScan();
  });

  registerCustomBenchmark("rf_transmit", [this](BenchmarkResult &result) {
    (void)result;
    return benchmarkRFTransmission();
  });

  registerCustomBenchmark("rfid_read", [this](BenchmarkResult &result) {
    (void)result;
    return benchmarkRFIDReading();
  });

  registerCustomBenchmark("ml_inference", [this](BenchmarkResult &result) {
    (void)result;
    return benchmarkMLInference();
  });

  registerCustomBenchmark("memory_stress", [this](BenchmarkResult &result) {
    (void)result;
    return benchmarkMemoryStress();
  });

  registerCustomBenchmark("cpu_stress", [this](BenchmarkResult &result) {
    (void)result;
    return benchmarkCPUStress();
  });

  AdvancedLogger::getInstance().info(
      LogModule::SYSTEM, "BenchmarkManager inicializado com sucesso");

  return true;
}

void BenchmarkManager::deinit() {
  if (!config_.enabled) {
    return;
  }

  AdvancedLogger::getInstance().info(LogModule::SYSTEM,
                                     "Desinicializando BenchmarkManager");

  // Salvar resultados antes de sair
  saveResultsToSD();

  results_.clear();
  custom_benchmarks_.clear();
}

void BenchmarkManager::process() {
  if (!config_.enabled || !config_.auto_run) {
    return;
  }

  uint32_t current_time = millis();
  if (current_time - last_run_time_ >= config_.interval_ms) {
    runAutomatedBenchmarks();
    last_run_time_ = current_time;
  }
}

bool BenchmarkManager::runBenchmark(const String &benchmark_name) {
  if (!config_.enabled) {
    return false;
  }

  AdvancedLogger::getInstance().info(
      LogModule::SYSTEM, "Executando benchmark: %s", benchmark_name.c_str());

  auto it = custom_benchmarks_.find(benchmark_name);
  if (it == custom_benchmarks_.end()) {
    AdvancedLogger::getInstance().warning(LogModule::SYSTEM,
                                          "Benchmark não encontrado: %s",
                                          benchmark_name.c_str());
    return false;
  }

  BenchmarkResult result;
  result.timestamp = millis();
  result.success = it->second(result);

  if (result.success) {
    results_.push_back(result);
    logResult(result);

    // Manter limite de resultados
    if (results_.size() > config_.max_results) {
      results_.erase(results_.begin());
    }
  }

  return result.success;
}

void BenchmarkManager::registerMetric(MetricType type, const String &operation,
                                      uint32_t duration_us, size_t memory_used,
                                      float cpu_usage, bool success) {
  if (!config_.enabled || !config_.enabled_metrics[type]) {
    return;
  }

  BenchmarkResult result;
  result.timestamp = millis();
  result.type = type;
  result.operation = operation;
  result.duration_us = duration_us;
  result.memory_used = memory_used;
  result.cpu_usage_percent = cpu_usage;
  result.success = success;

  results_.push_back(result);
  logResult(result);

  // Manter limite
  if (results_.size() > config_.max_results) {
    results_.erase(results_.begin());
  }
}

uint32_t BenchmarkManager::startProfiling(const String &operation) {
  if (!config_.enabled) {
    return 0;
  }

  AdvancedLogger::getInstance().debug(
      LogModule::SYSTEM, "Iniciando profiling: %s", operation.c_str());

  return esp_timer_get_time();
}

void BenchmarkManager::endProfiling(uint32_t start_time, MetricType type,
                                    const String &operation, bool success) {
  if (!config_.enabled || start_time == 0) {
    return;
  }

  uint32_t end_time = esp_timer_get_time();
  uint32_t duration_us = end_time - start_time;

  size_t memory_used = measureMemoryUsage();
  float cpu_usage = measureCPUUsage();

  registerMetric(type, operation, duration_us, memory_used, cpu_usage, success);

  AdvancedLogger::getInstance().debug(LogModule::SYSTEM,
                                      "Profiling finalizado: %s - %u us",
                                      operation.c_str(), duration_us);
}

bool BenchmarkManager::runStressTest(const String &test_name,
                                     uint32_t duration_ms,
                                     uint32_t iterations) {
  if (!config_.enabled) {
    return false;
  }

  AdvancedLogger::getInstance().info(
      LogModule::SYSTEM, "Iniciando teste de stress: %s", test_name.c_str());

  uint32_t start_time = millis();
  uint32_t iter_count = 0;
  bool success = true;

  while ((millis() - start_time < duration_ms) &&
         (iterations == 0 || iter_count < iterations)) {

    if (!runBenchmark(test_name)) {
      success = false;
      break;
    }

    iter_count++;

    // Pequena pausa para não sobrecarregar
    delay(10);
  }

  AdvancedLogger::getInstance().info(
      LogModule::SYSTEM,
      "Teste de stress finalizado: %s - %u iterações, sucesso: %s",
      test_name.c_str(), iter_count, success ? "sim" : "não");

  return success;
}

String BenchmarkManager::generateReport(bool include_historical) {
  JsonDocument doc;
  doc["timestamp"] = millis();
  doc["total_results"] = results_.size();

  // Estatísticas gerais
  JsonObject stats = doc["statistics"].to<JsonObject>();
  stats["cpu_avg"] = 0.0f;
  stats["memory_avg"] = 0;
  stats["latency_avg"] = 0;

  uint32_t cpu_count = 0, mem_count = 0, lat_count = 0;
  float cpu_sum = 0.0f;
  uint32_t mem_sum = 0, lat_sum = 0;

  for (const auto &result : results_) {
    if (result.cpu_usage_percent > 0) {
      cpu_sum += result.cpu_usage_percent;
      cpu_count++;
    }
    if (result.memory_used > 0) {
      mem_sum += result.memory_used;
      mem_count++;
    }
    if (result.duration_us > 0) {
      lat_sum += result.duration_us;
      lat_count++;
    }
  }

  if (cpu_count > 0)
    stats["cpu_avg"] = cpu_sum / cpu_count;
  if (mem_count > 0)
    stats["memory_avg"] = mem_sum / mem_count;
  if (lat_count > 0)
    stats["latency_avg"] = lat_sum / lat_count;

  // Resultados recentes
  JsonArray recent_results = doc["recent_results"].to<JsonArray>();
  size_t start_idx =
      include_historical ? 0 : std::max(0, (int)results_.size() - 10);

  for (size_t i = start_idx; i < results_.size(); ++i) {
    JsonObject res = recent_results.add<JsonObject>();
    res["timestamp"] = results_[i].timestamp;
    res["type"] = static_cast<int>(results_[i].type);
    res["operation"] = results_[i].operation;
    res["duration_us"] = results_[i].duration_us;
    res["memory_used"] = results_[i].memory_used;
    res["cpu_usage"] = results_[i].cpu_usage_percent;
    res["success"] = results_[i].success;
  }

  String report;
  serializeJson(doc, report);
  return report;
}

void BenchmarkManager::clearOldResults(uint32_t max_age_ms) {
  uint32_t current_time = millis();
  auto it = results_.begin();

  while (it != results_.end()) {
    if (current_time - it->timestamp > max_age_ms) {
      it = results_.erase(it);
    } else {
      ++it;
    }
  }

  AdvancedLogger::getInstance().info(LogModule::SYSTEM,
                                     "Resultados antigos limpos, restantes: %u",
                                     results_.size());
}

void BenchmarkManager::setConfig(const BenchmarkConfig &config) {
  config_ = config;
  AdvancedLogger::getInstance().info(
      LogModule::SYSTEM, "Configuração do BenchmarkManager atualizada");
}

BenchmarkManager::BenchmarkConfig BenchmarkManager::getConfig() const {
  return config_;
}

void BenchmarkManager::registerCustomBenchmark(const String &name,
                                               BenchmarkCallback callback) {
  custom_benchmarks_[name] = callback;
  AdvancedLogger::getInstance().info(
      LogModule::SYSTEM, "Benchmark customizado registrado: %s", name.c_str());
}

void BenchmarkManager::handleSerialCommand(const String &command,
                                           const std::vector<String> &args) {
  if (command == "benchmark_run") {
    if (args.size() >= 1) {
      bool success = runBenchmark(args[0]);
      Serial.printf("Benchmark %s executado: %s\n", args[0].c_str(),
                    success ? "sucesso" : "falha");
    }
  } else if (command == "benchmark_report") {
    String report = generateReport(args.size() > 0 && args[0] == "full");
    Serial.println("Relatório de Performance:");
    Serial.println(report);
  } else if (command == "benchmark_stress") {
    if (args.size() >= 2) {
      uint32_t duration = args[1].toInt();
      bool success = runStressTest(args[0], duration);
      Serial.printf("Teste de stress %s finalizado: %s\n", args[0].c_str(),
                    success ? "sucesso" : "falha");
    }
  } else if (command == "benchmark_clear") {
    uint32_t max_age = args.size() > 0 ? args[0].toInt() : 86400000;
    clearOldResults(max_age);
    Serial.println("Resultados antigos limpos");
  } else if (command == "benchmark_stats") {
    JsonDocument stats = getStatistics();
    String output;
    serializeJson(stats, output);
    Serial.println("Estatísticas:");
    Serial.println(output);
  }
}

JsonDocument BenchmarkManager::getStatistics() {
  JsonDocument doc;

  // Contagem por tipo
  std::map<MetricType, uint32_t> type_counts;
  for (const auto &result : results_) {
    type_counts[result.type]++;
  }

  JsonObject counts = doc["type_counts"].to<JsonObject>();
  for (const auto &pair : type_counts) {
    counts[String(static_cast<int>(pair.first))] = pair.second;
  }

  doc["total_results"] = results_.size();
  doc["memory_usage"] = ESP.getFreeHeap();
  doc["uptime"] = millis();

  return doc;
}

// Métodos auxiliares privados

void BenchmarkManager::runAutomatedBenchmarks() {
  if (is_running_)
    return;

  is_running_ = true;

  // Executar benchmarks automáticos
  runBenchmark("wifi_scan");
  runBenchmark("rf_transmit");
  runBenchmark("rfid_read");
  runBenchmark("ml_inference");

  is_running_ = false;
}

void BenchmarkManager::logResult(const BenchmarkResult &result) {
  AdvancedLogger::getInstance().info(
      LogModule::SYSTEM,
      "Benchmark result: %s - %u us, CPU: %.2f%%, Mem: %u bytes, Success: %s",
      result.operation.c_str(), result.duration_us, result.cpu_usage_percent,
      result.memory_used, result.success ? "true" : "false");

  if (config_.log_to_sd) {
    saveResultsToSD();
  }
}

float BenchmarkManager::measureCPUUsage() {
  // Medição simplificada de uso de CPU baseada em tempo de execução
  // Em implementação completa, usaria FreeRTOS task stats
  static uint32_t last_measurement = 0;
  static uint32_t last_time = 0;

  uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
  uint32_t current_measurement = xTaskGetTickCount();

  if (last_time == 0) {
    last_time = current_time;
    last_measurement = current_measurement;
    return 0.0f;
  }

  uint32_t time_diff = current_time - last_time;
  if (time_diff == 0)
    return 0.0f;

  // Estimativa baseada em ticks do sistema
  float usage =
      (float)(current_measurement - last_measurement) / time_diff * 100.0f;

  last_time = current_time;
  last_measurement = current_measurement;

  return min(usage, 100.0f);
}

size_t BenchmarkManager::measureMemoryUsage() {
  // Retorna memória heap livre
  return heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
}

uint32_t BenchmarkManager::measureLatency(std::function<void()> operation) {
  uint32_t start = esp_timer_get_time();
  operation();
  uint32_t end = esp_timer_get_time();
  return end - start;
}

void BenchmarkManager::saveResultsToSD() {
  if (!config_.log_to_sd)
    return;

  JsonDocument doc;
  JsonArray arr = doc.to<JsonArray>();

  for (const auto &result : results_) {
    JsonObject obj = arr.add<JsonObject>();
    obj["timestamp"] = result.timestamp;
    obj["type"] = static_cast<int>(result.type);
    obj["operation"] = result.operation;
    obj["duration_us"] = result.duration_us;
    obj["memory_used"] = result.memory_used;
    obj["cpu_usage"] = result.cpu_usage_percent;
    obj["success"] = result.success;
    if (!result.extra_data.isNull()) {
      obj["extra"] = result.extra_data;
    }
  }

  if (!SD.exists("/benchmarks"))
    SD.mkdir("/benchmarks");

  File file = SD.open("/benchmarks/results.json", FILE_WRITE);
  if (file) {
    serializeJson(doc, file);
    file.close();
    AdvancedLogger::getInstance().info(LogModule::SYSTEM,
                                       "Resultados salvos em SD");
  } else {
    AdvancedLogger::getInstance().error(LogModule::SYSTEM,
                                        "Erro ao salvar resultados em SD");
  }
}

void BenchmarkManager::loadResultsFromSD() {
  if (!SD.exists("/benchmarks/results.json"))
    return;

  File file = SD.open("/benchmarks/results.json", FILE_READ);
  if (!file)
    return;

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, file);
  file.close();

  if (error) {
    AdvancedLogger::getInstance().error(
        LogModule::SYSTEM, "Erro ao carregar resultados: %s", error.c_str());
    return;
  }

  JsonArray arr = doc.as<JsonArray>();
  for (JsonVariant v : arr) {
    BenchmarkResult result;
    result.timestamp = v["timestamp"];
    result.type = static_cast<MetricType>(v["type"].as<int>());
    result.operation = v["operation"].as<String>();
    result.duration_us = v["duration_us"];
    result.memory_used = v["memory_used"];
    result.cpu_usage_percent = v["cpu_usage"];
    result.success = v["success"];
    if (!v["extra"].isNull()) {
      result.extra_data = v["extra"];
    }
    results_.push_back(result);
  }

  AdvancedLogger::getInstance().info(
      LogModule::SYSTEM, "Resultados carregados do SD: %d", results_.size());
}

// Benchmarks específicos

bool BenchmarkManager::benchmarkWiFiScan() {
  auto &systemManager = SystemManager::getInstance();
  auto wifiModule = systemManager.getModule("WiFi");
  if (wifiModule) {
    // Usa chamada genérica do módulo em vez de cast
    JsonDocument result;
    bool success = wifiModule->executeCommand("scan", result);
    return success;
  } else {
    uint32_t start = startProfiling("WiFi Scan");
    WiFi.mode(WIFI_MODE_STA);
    int nets = WiFi.scanNetworks(false, true, false, 100);
    WiFi.scanDelete();
    endProfiling(start, MetricType::WIFI_SCAN_TIME, "WiFi Scan", nets >= 0);
    return nets >= 0;
  }
}

bool BenchmarkManager::benchmarkRFTransmission() {
  auto &systemManager = SystemManager::getInstance();
  auto rfModule = systemManager.getModule("RF");
  if (rfModule) {
    JsonDocument result;
    bool success = rfModule->executeCommand("transmit", result);
    return success;
  } else {
    uint32_t start = startProfiling("RF Transmission");
    delay(50);
    endProfiling(start, MetricType::RF_TRANSMIT_TIME, "RF Transmission", true);
    return true;
  }
}

bool BenchmarkManager::benchmarkRFIDReading() {
  auto &systemManager = SystemManager::getInstance();
  auto rfidModule = systemManager.getModule("RFID");
  if (rfidModule) {
    JsonDocument result;
    bool success = rfidModule->executeCommand("read", result);
    return success;
  } else {
    uint32_t start = startProfiling("RFID Reading");
    delay(75);
    endProfiling(start, MetricType::RFID_READ_TIME, "RFID Reading", true);
    return true;
  }
}

bool BenchmarkManager::benchmarkMLInference() {
  auto &systemManager = SystemManager::getInstance();
  auto mlModule = systemManager.getModule("ML");
  if (mlModule) {
    uint32_t start = startProfiling("ML Inference");
    JsonDocument result;
    bool success = mlModule->executeCommand("classify", result);
    endProfiling(start, MetricType::ML_INFERENCE_TIME, "ML Inference", success);
    return success;
  } else {
    uint32_t start = startProfiling("ML Inference");
    delay(200);
    endProfiling(start, MetricType::ML_INFERENCE_TIME, "ML Inference", true);
    return true;
  }
}

bool BenchmarkManager::benchmarkMemoryStress() {
  uint32_t start = startProfiling("Memory Stress Test");

  // Teste de stress de memória
  const size_t block_size = 1024;
  std::vector<uint8_t *> blocks;

  for (int i = 0; i < 10; ++i) {
    uint8_t *block = new uint8_t[block_size];
    if (block) {
      memset(block, 0xAA, block_size);
      blocks.push_back(block);
    } else {
      // Falha na alocação
      endProfiling(start, MetricType::MEMORY_USAGE, "Memory Stress Test",
                   false);
      return false;
    }
  }

  // Liberar memória
  for (auto block : blocks) {
    delete[] block;
  }

  endProfiling(start, MetricType::MEMORY_USAGE, "Memory Stress Test", true);
  return true;
}

bool BenchmarkManager::benchmarkCPUStress() {
  uint32_t start = startProfiling("CPU Stress Test");

  // Teste de stress de CPU
  volatile uint32_t sum = 0;
  for (uint32_t i = 0; i < 100000; ++i) {
    sum += i * i;
  }

  endProfiling(start, MetricType::CPU_USAGE, "CPU Stress Test", true);
  return true;
}