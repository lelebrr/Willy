#ifndef __DYNAMIC_CONFIG_MANAGER_H__
#define __DYNAMIC_CONFIG_MANAGER_H__

#include "IModule.h"
#include <ArduinoJson.h>
#include <FS.h>
#include <Preferences.h>
#include <functional>
#include <vector>

/**
 * @brief Gerenciador de configuração dinâmica com hot-reload
 *
 * Permite carregar configurações JSON do SD card em tempo de execução,
 * com validação de schema, hot-reload automático e fallback para NVS.
 * Integra-se com SystemModel para atualização dinâmica de parâmetros.
 */
class DynamicConfigManager : public IModule {
public:
  /**
   * @brief Estrutura de configuração crítica para fallback NVS
   */
  struct CriticalConfig {
    int bright = 100;
    bool wifiAtStartup = false;
    String startupApp = "";
    bool instantBoot = false;
    uint16_t bgColor = 0x0000;
    String themePath = "";
    bool themeFS = false;
  };

  /**
   * @brief Callback para validação de schema
   */
  using SchemaValidator = std::function<bool(const JsonDocument &)>;

  /**
   * @brief Callback para atualização de configuração
   */
  using ConfigUpdateCallback = std::function<void(const JsonDocument &)>;

  static DynamicConfigManager &getInstance();

  // IModule interface
  bool init() override;
  void process() override;
  void deinit() override;
  String getName() const override { return "DynamicConfigManager"; }
  int getPriority() const override { return 10; } // Alta prioridade
  bool isActive() const override { return true; }
  bool executeCommand(const String &command, JsonDocument &result) override {
    (void)command;
    (void)result;
    return false;
  }

  /**
   * @brief Carrega configuração do arquivo JSON
   * @param filename Caminho do arquivo (relativo ao SD card)
   * @return true se carregado com sucesso
   */
  bool loadConfigFromFile(const String &filename);

  /**
   * @brief Recarrega configuração atual
   * @return true se recarregado com sucesso
   */
  bool reloadConfig();

  /**
   * @brief Define validador de schema
   * @param validator Função de validação
   */
  void setSchemaValidator(SchemaValidator validator);

  /**
   * @brief Registra callback para atualização de configuração
   * @param callback Função a ser chamada quando config é atualizada
   */
  void setConfigUpdateCallback(ConfigUpdateCallback callback);

  /**
   * @brief Obtém configuração atual
   * @return Referência para documento JSON atual
   */
  const JsonDocument &getCurrentConfig() const { return currentConfig_; }

  /**
   * @brief Verifica se configuração está carregada
   * @return true se configuração válida está carregada
   */
  bool isConfigLoaded() const { return configLoaded_; }

  /**
   * @brief Salva configuração crítica no NVS
   * @param config Configuração crítica
   */
  void saveCriticalConfig(const CriticalConfig &config);

  /**
   * @brief Carrega configuração crítica do NVS
   * @return Configuração crítica ou valores padrão
   */
  CriticalConfig loadCriticalConfig();

  /**
   * @brief Comando serial para recarregar configuração
   */
  void reloadConfigCommand();

public:
  DynamicConfigManager() = default;
  ~DynamicConfigManager() = default;

  // Proibir cópia
  DynamicConfigManager(const DynamicConfigManager &) = delete;
  DynamicConfigManager &operator=(const DynamicConfigManager &) = delete;

  /**
   * @brief Verifica se arquivo foi modificado
   * @return true se arquivo foi modificado desde último carregamento
   */
  bool checkFileModified();

  /**
   * @brief Valida configuração com schema
   * @param config Documento JSON a validar
   * @return true se válido
   */
  bool validateConfig(const JsonDocument &config);

  /**
   * @brief Aplica configuração ao sistema
   * @param config Documento JSON a aplicar
   */
  void applyConfig(const JsonDocument &config);

  /**
   * @brief Carrega configuração padrão
   */
  void loadDefaultConfig();

  // Estado interno
  String configFilename_;
  JsonDocument currentConfig_;
  bool configLoaded_ = false;
  time_t lastModified_ = 0;
  SchemaValidator schemaValidator_;
  ConfigUpdateCallback updateCallback_;

  // Configurações críticas em NVS
  static constexpr const char *NVS_NAMESPACE = "dyn_config";
  static constexpr const char *CRITICAL_CONFIG_KEY = "critical";
};

#endif // __DYNAMIC_CONFIG_MANAGER_H__
