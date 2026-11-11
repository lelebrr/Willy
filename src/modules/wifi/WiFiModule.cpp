#include "WiFiModule.h"
#include "../core/BenchmarkManager.h"
#include "../core/SecurityUtils.h" // Para validações de segurança
#include "../core/SystemManager.h"
#include "../core/advanced_logger.h" // Para logging avançado
#include "wifi_atks.h"               // Para delegar funcionalidades existentes
#include <WiFi.h>
#include <esp32-hal-psram.h>
#include <esp_heap_caps.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <unordered_map>
#include <vector>

/**
 * @brief Gerenciador de Ataques Adaptativos
 *
 * Implementa algoritmos que aprendem com respostas do alvo,
 * ajustando intensidade e estratégias de ataque dinamicamente.
 */
class AdaptiveAttackManager {
public:
  AdaptiveAttackManager()
      : successRate(0.5f), intensity(1.0f), lastAdjustment(0) {}

  /**
   * @brief Registra resultado de ataque
   * @param success true se ataque teve efeito detectado
   */
  void recordResult(bool success) {
    // Algoritmo simples: média móvel de sucesso
    successRate = successRate * 0.9f + (success ? 1.0f : 0.0f) * 0.1f;

    // Ajustar intensidade baseada na taxa de sucesso
    if (millis() - lastAdjustment > 60000) { // A cada 1min
      if (successRate > 0.7f) {
        intensity = min(intensity * 1.1f, 2.0f); // Aumentar se muito efetivo
      } else if (successRate < 0.3f) {
        intensity = max(intensity * 0.9f, 0.5f); // Diminuir se pouco efetivo
      }
      lastAdjustment = millis();
    }
  }

  /**
   * @brief Obtém intensidade adaptada para ataques
   * @return Fator de multiplicação para número de pacotes/delay
   */
  float getAdaptiveIntensity() const { return intensity; }

private:
  float successRate;       // Taxa de sucesso (0-1)
  float intensity;         // Intensidade atual (0.5-2.0)
  uint32_t lastAdjustment; // Último ajuste
};

WiFiModule::WiFiModule(std::shared_ptr<SystemModel> model,
                       std::shared_ptr<SystemView> view)
    : model_(model), view_(view), active_(false), initialized_(false),
      adaptiveManager_(new AdaptiveAttackManager()), asyncScanActive_(false),
      asyncScanTask_(nullptr) {

  // Inicializar cache de scanning
  scanCache_.lastScanTime = 0;
  scanCache_.cacheValidityMs = 30000; // 30 segundos

  // Inicializar limiter de ataques
  attackLimiter_.lastActionTime = 0;
  attackLimiter_.minIntervalMs = 1000; // 1 segundo mínimo entre ações
  attackLimiter_.actionCount = 0;
  attackLimiter_.maxActionsPerMinute = 60; // Máximo 60 ações por minuto
}

bool WiFiModule::init() {
  if (initialized_)
    return true;

  Serial.println("[WiFiModule] Inicializando módulo WiFi...");

  // Configuração inicial do WiFi
  // Delega para lógica existente se necessário

  initialized_ = true;
  active_ = true;

  Serial.println("[WiFiModule] Módulo WiFi inicializado");
  return true;
}

void WiFiModule::deinit() {
  if (!initialized_)
    return;

  Serial.println("[WiFiModule] Desinicializando módulo WiFi...");

  active_ = false;
  initialized_ = false;

  Serial.println("[WiFiModule] Módulo WiFi desinicializado");
}

void WiFiModule::process() {
  if (!active_)
    return;

  // Otimizado: aplicar técnicas de stealth e otimização de recursos
  applyStealthTechniques();
  optimizeForLongAttacks();

  // Limpar cache expirado periodicamente
  static uint32_t lastCacheCleanup = 0;
  if (millis() - lastCacheCleanup > 60000) { // A cada 1min
    cleanupExpiredCache();
    lastCacheCleanup = millis();
  }

  // Monitorar recursos do sistema
  static uint32_t lastResourceCheck = 0;
  if (millis() - lastResourceCheck > 30000) { // A cada 30s
    monitorMemoryUsage();
    lastResourceCheck = millis();
  }

  // Processamento contínuo do módulo WiFi
  // TODO: Implementar processamento específico se necessário
}

void WiFiModule::wifi_atk_menu() {
  if (!active_)
    return;

  // Delega para função existente
  ::wifi_atk_menu();
}

void WiFiModule::target_atk_menu(String tssid, String mac, uint8_t channel) {
  if (!active_)
    return;

  // Validação de segurança dos parâmetros
  if (!SecurityValidator::validateSSID(tssid)) {
    SecurityLogger::log(SecurityLogger::WARNING, "WiFi",
                        "SSID inválido rejeitado: " + tssid);
    return;
  }

  if (!SecurityValidator::validateMAC(mac)) {
    SecurityLogger::log(SecurityLogger::WARNING, "WiFi",
                        "MAC inválido rejeitado: " + mac);
    return;
  }

  if (!SecurityValidator::validateWiFiChannel(channel)) {
    SecurityLogger::log(SecurityLogger::WARNING, "WiFi",
                        "Canal inválido rejeitado: " + String(channel));
    return;
  }

  // Rate limiting para prevenir abuso
  if (!attackLimiter_.allowAction()) {
    SecurityLogger::log(SecurityLogger::WARNING, "WiFi",
                        "Ataque bloqueado por rate limiting");
    return;
  }

  // Log de segurança
  SecurityLogger::logSecurityEvent("WiFi Attack Menu",
                                   "SSID: " + tssid + ", MAC: " + mac +
                                       ", Channel: " + String(channel));

  // Delega para função existente
  ::target_atk_menu(tssid, mac, channel);
}

void WiFiModule::target_atk(String tssid, String mac, uint8_t channel) {
  if (!active_)
    return;

  // Delega para função existente
  ::target_atk(tssid, mac, channel);
}

void WiFiModule::capture_handshake(String tssid, String mac, uint8_t channel) {
  if (!active_)
    return;

  // Delega para função existente
  ::capture_handshake(tssid, mac, channel);
}

void WiFiModule::beaconAttack() {
  if (!active_)
    return;

  // Delega para função existente
  ::beaconAttack();
}

int WiFiModule::classifyDevice(float rssi, float frequency, int channel,
                               String ssid) {
  if (!active_)
    return -1; // UNKNOWN_DEVICE

  // Obter instância do módulo ML
  auto &systemManager = SystemManager::getInstance();
  auto mlModule = static_cast<MLModule *>(systemManager.getModule("ML"));

  if (!mlModule) {
    Serial.println("[WiFiModule] Módulo ML não encontrado");
    return -1;
  }

  // Classificar usando ML
  auto result = mlModule->classifyWiFiDevice(rssi, frequency, channel,
                                             std::string(ssid.c_str()));

  Serial.printf(
      "[WiFiModule] Dispositivo classificado: tipo=%d, confiança=%.2f\n",
      static_cast<int>(result.predictedType), result.confidence);

  return static_cast<int>(result.predictedType);
}

/**
 * @brief Otimiza uso de recursos para ataques longos
 *
 * Implementa técnicas para reduzir consumo de energia e CPU
 * durante operações prolongadas, mantendo eficácia.
 */
void WiFiModule::optimizeForLongAttacks() {
  if (!active_)
    return;

  // Otimizado: reduzir prioridade de tarefas não críticas
  // Implementar yield para permitir outras tarefas
  vTaskDelay(1 / portTICK_PERIOD_MS);

  // Monitorar uso de memória e liberar se necessário
  // (implementação futura)
}

/**
 * @brief Implementa técnicas de stealth para reduzir detecção
 *
 * Modula timing, potência e padrões para evitar detecção
 * por sistemas de monitoramento WiFi.
 */
void WiFiModule::applyStealthTechniques() {
  if (!active_)
    return;

  // Otimizado: variar canais mais frequentemente para evitar detecção
  static uint32_t lastChannelChange = 0;
  if (millis() - lastChannelChange > 30000) { // A cada 30s
    nextChannel();
    lastChannelChange = millis();
  }

  // Técnicas de stealth adicionais:
  // - Variação de timing entre pacotes
  // - Uso de potência adaptativa
  // - MAC spoofing inteligente
}

void WiFiModule::optimizeForLongAttacks() {
  if (!active_)
    return;

  // Otimizado: reduzir prioridade de tarefas não críticas
  // Implementar yield para permitir outras tarefas
  vTaskDelay(1 / portTICK_PERIOD_MS);

  // Monitorar uso de memória e liberar se necessário
  monitorMemoryUsage();
}

void WiFiModule::nextChannel() {
  if (!active_)
    return;

  // Implementar mudança de canal WiFi
  static uint8_t currentChannel = 1;
  currentChannel = (currentChannel % 13) + 1; // Canais 1-13

  // Mudar canal usando ESP-IDF
  esp_wifi_set_channel(currentChannel, WIFI_SECOND_CHAN_NONE);

  // Log da mudança de canal
  Serial.printf("[WiFi] Hopping to channel %d\n", currentChannel);
}

void WiFiModule::cleanupExpiredCache() {
  if (!active_)
    return;

  uint32_t now = millis();
  if (now - scanCache_.lastScanTime > scanCache_.cacheValidityMs) {
    // Limpar cache expirado
    scanCache_.ssids.clear();
    scanCache_.macs.clear();
    scanCache_.rssis.clear();
    scanCache_.channels.clear();
    scanCache_.lastScanTime = 0;

    logWiFiEvent("Cache de scanning limpo", "Cache expirado", LogLevel::DEBUG);
  }
}

void WiFiModule::monitorMemoryUsage() {
  if (!active_)
    return;

  // Monitorar uso de heap
  uint32_t freeHeap = ESP.getFreeHeap();
  uint32_t minFreeHeap = ESP.getMinFreeHeap();

  // Log se memória baixa
  if (freeHeap < 50000) { // Menos de 50KB livre
    logWiFiEvent("Memória baixa detectada",
                 "Free: " + String(freeHeap) + " Min: " + String(minFreeHeap),
                 LogLevel::WARNING);
  }
}

void WiFiModule::logWiFiEvent(const String &event, const String &details,
                              LogLevel level) {
  // Usar advanced_logger se disponível
  // Por enquanto, usar Serial
  String logMsg = "[WiFiModule] " + event;
  if (!details.isEmpty()) {
    logMsg += ": " + details;
  }

  switch (level) {
  case LogLevel::ERROR:
    Serial.println("ERROR: " + logMsg);
    break;
  case LogLevel::WARNING:
    Serial.println("WARN: " + logMsg);
    break;
  case LogLevel::INFO:
    Serial.println("INFO: " + logMsg);
    break;
  case LogLevel::DEBUG:
    Serial.println("DEBUG: " + logMsg);
    break;
  }
}

int WiFiModule::performWiFiScan() {
  if (!active_)
    return 0;

  // Verificar cache primeiro
  cleanupExpiredCache();
  if (!scanCache_.ssids.empty() &&
      (millis() - scanCache_.lastScanTime) < scanCache_.cacheValidityMs) {
    logWiFiEvent("Usando cache de scanning",
                 String(scanCache_.ssids.size()) + " redes encontradas",
                 LogLevel::DEBUG);
    return scanCache_.ssids.size();
  }

  // Realizar scanning real
  logWiFiEvent("Iniciando scanning WiFi", "", LogLevel::INFO);

  // Delegar para implementação existente
  int result = ::performWiFiScan();

  // Atualizar cache se scanning bem-sucedido
  if (result > 0) {
    scanCache_.lastScanTime = millis();
    // Popular cache com resultados reais
    scanCache_.ssids.clear();
    scanCache_.macs.clear();
    scanCache_.rssis.clear();
    scanCache_.channels.clear();

    // Simular obtenção de resultados (implementar baseado na API real)
    // Por enquanto, placeholder funcional
    for (int i = 0; i < result; i++) {
      scanCache_.ssids.push_back("SSID_" + String(i));
      scanCache_.macs.push_back("00:11:22:33:44:" + String(i, HEX));
      scanCache_.rssis.push_back(-50 - i);
      scanCache_.channels.push_back((i % 13) + 1);
    }
  }

  return result;
}

void WiFiModule::startAsyncScan() {
  if (!active_ || asyncScanActive_)
    return;

  logWiFiEvent("Iniciando scanning assíncrono", "", LogLevel::INFO);

  // Criar tarefa para scanning assíncrono
  xTaskCreate(
      [](void *param) {
        WiFiModule *module = static_cast<WiFiModule *>(param);
        while (module->asyncScanActive_) {
          module->performWiFiScan();
          vTaskDelay(pdMS_TO_TICKS(30000)); // Scan a cada 30s
        }
        vTaskDelete(NULL);
      },
      "WiFiScanTask", 4096, this, 1, &asyncScanTask_);

  asyncScanActive_ = true;
}

void WiFiModule::stopAsyncScan() {
  if (!asyncScanActive_)
    return;

  logWiFiEvent("Parando scanning assíncrono", "", LogLevel::INFO);

  asyncScanActive_ = false;

  if (asyncScanTask_ != nullptr) {
    vTaskDelete(asyncScanTask_);
    asyncScanTask_ = nullptr;
  }
}