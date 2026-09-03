#ifndef __ML_MODULE_H__
#define __ML_MODULE_H__

#include "core/IModule.h"
#include "core/SecurityUtils.h"
#include "core/SystemModel.h"
#include "core/SystemView.h"
#include <algorithm>
#include <array>
#include <map>
#include <memory>
#include <string>
#include <vector>

// Constantes de otimização para ESP32-S3
#define ML_MAX_TRAINING_POINTS 1000 ///< Máximo pontos de treinamento por tipo
#define ML_MAX_FEATURES 10          ///< Máximo número de features por amostra
#define ML_KNN_K_DEFAULT 3          ///< Valor padrão de k para k-NN
#define ML_ANOMALY_THRESHOLD_DEFAULT 2.5f ///< Threshold Z-score para anomalias
#define ML_HISTORY_BUFFER_SIZE 500        ///< Tamanho do buffer histórico
#define ML_CONFIDENCE_THRESHOLD 0.6f      ///< Threshold mínimo de confiança

// Macros de validação
#define ML_VALIDATE_FEATURES(features)                                         \
  ((features).size() > 0 && (features).size() <= ML_MAX_FEATURES &&            \
   std::all_of((features).begin(), (features).end(),                           \
               [](float f) { return std::isfinite(f); }))

#define ML_VALIDATE_DEVICE_TYPE(type)                                          \
  ((type) >= WIFI_ROUTER && (type) <= UNKNOWN_DEVICE)

/**
 * @brief Módulo de Machine Learning do Willy Firmware - Detecção de Padrões
 *
 * O MLModule implementa algoritmos de aprendizado de máquina leves e otimizados
 * para detecção de padrões em dispositivos WiFi, RF e RFID. Utiliza a
 * arquitetura modular do sistema através da interface IModule.
 *
 * Capacidades principais:
 * - Análise estatística básica (média, desvio padrão, correlação)
 * - Classificação k-NN otimizada com busca aproximada
 * - Detecção de anomalias baseada em Z-score com validação robusta
 * - Aprendizado incremental com limite de memória
 * - Treinamento offline e inferência em tempo real
 * - Integração com módulos WiFi, RF e RFID
 * - Validações de segurança e sanitização de entrada
 * - Logging detalhado e tratamento de erros
 *
 * @note Implementação otimizada para ESP32-S3 com uso mínimo de memória (<
 * 50KB)
 * @note Algoritmos leves sem dependências externas pesadas
 * @note Thread-safe para operações críticas
 * @note Suporte a até 1000 pontos de treinamento por tipo de dispositivo
 *
 * @author Willy Firmware Team
 * @version 2.0.0
 * @date 2024
 *
 * @see IModule Interface base implementada
 * @see SystemModel Para acesso a configuração
 * @see SystemView Para interface do usuário
 * @see SecurityUtils Para validações de segurança
 */
class MLModule : public IModule {
public:
  /**
   * @brief Tipos de dispositivos suportados para classificação
   */
  enum DeviceType {
    WIFI_ROUTER = 0,
    WIFI_SMARTPHONE,
    WIFI_IOT_DEVICE,
    WIFI_LAPTOP,
    RF_REMOTE_CONTROL,
    RF_SENSOR,
    RF_GARAGE_DOOR,
    RFID_CARD,
    RFID_TAG,
    RFID_KEY,
    UNKNOWN_DEVICE
  };

  /**
   * @brief Estrutura para dados de treinamento com validação
   */
  struct TrainingData {
    std::vector<float> features; // Características extraídas
    DeviceType label;            // Tipo do dispositivo
    uint32_t timestamp;          // Timestamp da coleta

    /**
     * @brief Construtor padrão
     */
    TrainingData() : label(UNKNOWN_DEVICE), timestamp(0) {}

    /**
     * @brief Valida os dados de treinamento
     * @return true se válido
     */
    bool isValid() const {
      return ML_VALIDATE_DEVICE_TYPE(label) && ML_VALIDATE_FEATURES(features) &&
             timestamp > 0;
    }
  };

  /**
   * @brief Resultado de classificação com validação
   */
  struct ClassificationResult {
    DeviceType predictedType;
    float confidence; // 0.0 a 1.0
    std::array<std::pair<DeviceType, float>, 5>
        probabilities; // Top 5 probabilidades
    size_t probabilityCount;

    /**
     * @brief Construtor padrão
     */
    ClassificationResult()
        : predictedType(UNKNOWN_DEVICE), confidence(0.0f), probabilityCount(0) {
    }

    /**
     * @brief Valida o resultado
     * @return true se válido
     */
    bool isValid() const {
      return ML_VALIDATE_DEVICE_TYPE(predictedType) && confidence >= 0.0f &&
             confidence <= 1.0f && probabilityCount <= 5;
    }
  };

  /**
   * @brief Resultado de detecção de anomalia com validação
   */
  struct AnomalyResult {
    bool isAnomaly;
    float score; // Score de anomalia (0-1, onde 1 é altamente anômalo)
    std::string reason;

    /**
     * @brief Construtor padrão
     */
    AnomalyResult() : isAnomaly(false), score(0.0f) {}

    /**
     * @brief Valida o resultado
     * @return true se válido
     */
    bool isValid() const {
      return score >= 0.0f && score <= 1.0f && !reason.empty();
    }
  };

  /**
   * @brief Construtor do módulo ML
   *
   * @param model Ponteiro compartilhado para o modelo do sistema
   * @param view Ponteiro compartilhado para a visão/interface do sistema
   */
  MLModule(std::shared_ptr<SystemModel> model,
           std::shared_ptr<SystemView> view);

  /**
   * @brief Destrutor
   */
  ~MLModule() override = default;

  // Implementação da interface IModule
  bool init() override;
  void deinit() override;
  void process() override;
  String getName() const override { return "ML"; }
  bool isActive() const override { return true; }
  int getPriority() const override { return 50; }
  bool executeCommand(const String &command, JsonDocument &result) override;
  String getDescription() const;
  void getCapabilities(std::vector<std::string> &caps) const;

  /**
   * @brief Classifica um dispositivo WiFi baseado em características
   *
   * @param features Vetor de características (RSSI, frequência, etc.)
   * @return Resultado da classificação
   */
  ClassificationResult classifyWiFiDevice(const std::vector<float> &features);

  /**
   * @brief Classifica um sinal RF
   *
   * @param features Características do sinal RF
   * @return Resultado da classificação
   */
  ClassificationResult classifyRFSignal(const std::vector<float> &features);

  /**
   * @brief Detecta comportamento suspeito em RFID
   *
   * @param features Características do comportamento RFID
   * @return Resultado da detecção
   */
  AnomalyResult detectRFIDAnomaly(const std::vector<float> &features);

  /**
   * @brief Adiciona dados de treinamento
   *
   * @param data Dados de treinamento
   * @return true se adicionado com sucesso
   */
  bool addTrainingData(const TrainingData &data);

  /**
   * @brief Treina os modelos com dados coletados
   *
   * @return true se treinamento bem-sucedido
   */
  bool trainModels();

  /**
   * @brief Salva modelos treinados no armazenamento
   *
   * @return true se salvo com sucesso
   */
  bool saveModels();

  /**
   * @brief Carrega modelos do armazenamento
   *
   * @return true se carregado com sucesso
   */
  bool loadModels();

  /**
   * @brief Obtém estatísticas dos modelos
   *
   * @return Mapa com estatísticas
   */
  std::map<std::string, float> getModelStats();

  /**
   * @brief Integração com WiFi: classifica dispositivo detectado
   *
   * @param rssi RSSI do sinal
   * @param frequency Frequência
   * @param channel Canal
   * @param ssid Nome da rede (opcional)
   * @return Classificação do dispositivo
   */
  ClassificationResult classifyWiFiDevice(float rssi, float frequency,
                                          int channel,
                                          const std::string &ssid = "");

  /**
   * @brief Integração com RF: classifica sinal detectado
   *
   * @param frequency Frequência
   * @param duration Duração do sinal
   * @param amplitude Amplitude
   * @param modulation Tipo de modulação (opcional)
   * @return Classificação do sinal
   */
  ClassificationResult classifyRFSignal(float frequency, float duration,
                                        float amplitude,
                                        const std::string &modulation = "");

  /**
   * @brief Integração com RFID: detecta anomalia em comportamento
   *
   * @param readInterval Intervalo entre leituras
   * @param signalStrength Força do sinal
   * @param tagType Tipo de tag
   * @return Resultado da detecção de anomalia
   */
  AnomalyResult detectRFIDAnomaly(float readInterval, float signalStrength,
                                  const std::string &tagType = "");

  /**
   * @brief Adiciona dados de treinamento de dispositivo WiFi
   *
   * @param rssi RSSI
   * @param frequency Frequência
   * @param channel Canal
   * @param deviceType Tipo do dispositivo
   * @return true se adicionado
   */
  bool addWiFiTrainingData(float rssi, float frequency, int channel,
                           DeviceType deviceType);

  /**
   * @brief Adiciona dados de treinamento de sinal RF
   *
   * @param frequency Frequência
   * @param duration Duração
   * @param amplitude Amplitude
   * @param deviceType Tipo do dispositivo
   * @return true se adicionado
   */
  bool addRFTrainingData(float frequency, float duration, float amplitude,
                         DeviceType deviceType);

  /**
   * @brief Adiciona dados de treinamento RFID
   *
   * @param readInterval Intervalo
   * @param signalStrength Força do sinal
   * @param deviceType Tipo do dispositivo
   * @return true se adicionado
   */
  bool addRFIDTrainingData(float readInterval, float signalStrength,
                           DeviceType deviceType);

private:
  std::shared_ptr<SystemModel> _model;
  std::shared_ptr<SystemView> _view;

  // Estado do módulo
  bool _initialized;
  bool _active;

  // Mutex para thread-safety (FreeRTOS)
  mutable SemaphoreHandle_t _mlMutex;

  // Dados de treinamento com limite de memória
  std::vector<TrainingData> _trainingData;
  size_t _maxTrainingData = ML_MAX_TRAINING_POINTS;

  // Modelos k-NN otimizados (armazenar apenas pontos essenciais)
  std::vector<TrainingData> _knnModel;

  // Estatísticas históricas para detecção de anomalias (buffer circular)
  std::map<DeviceType, std::vector<float>> _historicalStats;
  size_t _maxHistorySize = ML_HISTORY_BUFFER_SIZE;

  // Configurações otimizadas
  int _knnK = ML_KNN_K_DEFAULT; // Número de vizinhos para k-NN
  float _anomalyThreshold =
      ML_ANOMALY_THRESHOLD_DEFAULT; // Threshold para Z-score
  float _confidenceThreshold =
      ML_CONFIDENCE_THRESHOLD; // Threshold mínimo de confiança

  // Cache para otimizações
  mutable std::map<DeviceType, std::pair<float, float>>
      _statsCache; // média, desvio para cada tipo
  mutable uint32_t _cacheTimestamp = 0;

  // Métodos auxiliares
  float calculateEuclideanDistance(const std::vector<float> &a,
                                   const std::vector<float> &b);
  ClassificationResult performKNN(const std::vector<float> &features);
  AnomalyResult detectAnomalyZScore(const std::vector<float> &features,
                                    DeviceType type);
  void updateHistoricalStats(const TrainingData &data);
  std::vector<float>
  extractFeaturesFromWiFi(const std::map<std::string, float> &wifiData);
  std::vector<float>
  extractFeaturesFromRF(const std::map<std::string, float> &rfData);
  std::vector<float>
  extractFeaturesFromRFID(const std::map<std::string, float> &rfidData);

  // Utilitários estatísticos
  float calculateMean(const std::vector<float> &data);
  float calculateStdDev(const std::vector<float> &data, float mean);
};

#endif // __ML_MODULE_H__
