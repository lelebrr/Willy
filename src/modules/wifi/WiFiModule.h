#ifndef __WIFI_MODULE_H__
#define __WIFI_MODULE_H__

#include "core/IModule.h"
#include "core/SecurityUtils.h"
#include "core/SystemModel.h"
#include "core/SystemView.h"
#include <ArduinoJson.h>
#include <memory>

/**
 * @brief Módulo WiFi do Willy Firmware - Análise e Ataques de Rede Wireless
 *
 * O WiFiModule implementa funcionalidades completas de análise e ataque
 * a redes WiFi 802.11a/b/g/n/ac/ax. Utiliza a arquitetura modular do sistema
 * através da interface IModule para integração perfeita com o framework.
 *
 * Capacidades principais:
 * - Escaneamento passivo de redes WiFi (Wardriving)
 * - Captura de handshakes WPA/WPA2
 * - Ataques de desautenticação (Deauth)
 * - Ataques Beacon Flood
 * - Análise de segurança de redes
 * - Suporte a múltiplos canais e bandas
 *
 * O módulo integra-se com o ESP32 WiFi stack e utiliza bibliotecas
 * especializadas para operações de baixo nível.
 *
 * @note Requer hardware ESP32-S3 com capacidades WiFi
 * @note Algumas operações podem interferir com conectividade normal
 * @note Compatível com padrões 802.11a/b/g/n/ac/ax
 *
 * @author Willy Firmware Team
 * @version 1.0.0
 * @date 2024
 *
 * @see IModule Interface base implementada
 * @see SystemModel Para acesso a configuração
 * @see SystemView Para interface do usuário
 */
class WiFiModule : public IModule {
public:
  /**
   * @brief Construtor do módulo WiFi
   *
   * Inicializa o módulo com referências para o modelo e visão do sistema.
   * Estas referências permitem acesso à configuração global e interface do
   * usuário.
   *
   * @param model Ponteiro compartilhado para o modelo do sistema
   * @param view Ponteiro compartilhado para a visão/interface do sistema
   *
   * @note O módulo não é inicializado automaticamente no construtor
   * @note Chamada a init() é necessária para operação
   *
   * @see init() Para inicialização completa
   */
  WiFiModule(std::shared_ptr<SystemModel> model,
             std::shared_ptr<SystemView> view);

  /**
   * @brief Destrutor do módulo WiFi
   *
   * Garante limpeza adequada de recursos WiFi e estado do sistema.
   */
  virtual ~WiFiModule() = default;

  // ========== Implementação da Interface IModule ==========

  /**
   * @brief Inicializa o módulo WiFi e recursos necessários
   *
   * Configura o stack WiFi do ESP32, inicializa buffers de captura,
   * configura canais de monitoramento e prepara para operações.
   *
   * @return true se inicialização bem-sucedida
   * @return false se falha (hardware indisponível, etc.)
   *
   * @note Pode desabilitar conectividade WiFi normal temporariamente
   * @note Configura modo promiscuo para captura de pacotes
   *
   * @see deinit() Para limpeza
   * @see IModule::init()
   */
  bool init() override;

  /**
   * @brief Desinicializa o módulo e libera recursos
   *
   * Restaura configuração WiFi normal, limpa buffers de captura,
   * finaliza operações em andamento e libera memória alocada.
   *
   * @note Pode ser chamado múltiplas vezes sem efeitos colaterais
   * @note Restaura conectividade WiFi normal se necessário
   *
   * @see init() Para reinicialização
   * @see IModule::deinit()
   */
  void deinit() override;

  /**
   * @brief Processa operações periódicas do módulo WiFi
   *
   * Executa processamento não-bloqueante incluindo:
   * - Captura de pacotes WiFi
   * - Análise de handshakes
   * - Atualização de status de ataques
   * - Gerenciamento de buffers
   *
   * @note Chamado frequentemente pelo loop principal
   * @note Deve ser não-bloqueante para manter responsividade
   *
   * @see IModule::process()
   */
  void process() override;

  /**
   * @brief Retorna o nome identificador do módulo
   *
   * @return String "WiFi"
   *
   * @see IModule::getName()
   */
  String getName() const override { return "WiFi"; }

  /**
   * @brief Verifica se o módulo WiFi está operacional
   *
   * @return true se módulo ativo e hardware disponível
   * @return false se inativo ou com problemas
   *
   * @see IModule::isActive()
   */
  bool isActive() const override { return active_; }

  /**
   * @brief Retorna a prioridade de processamento do módulo
   *
   * @return int valor da prioridade (maior = mais prioritário)
   *
   * @see IModule::getPriority()
   */
  int getPriority() const override { return 100; }

  /**
   * @brief Executa um comando genérico no módulo
   *
   * @param command Nome do comando a executar
   * @param result Documento JSON com resultado da operação
   * @return true se comando executado com sucesso
   */
  bool executeCommand(const String &command, JsonDocument &result) override {
    (void)command;
    (void)result;
    return false;
  }

  // ========== Funcionalidades WiFi Específicas ==========

  /**
   * @brief Exibe menu principal de ataques WiFi
   *
   * Apresenta interface para seleção de diferentes tipos de ataque
   * e análise WiFi disponíveis no módulo.
   *
   * @note Atualiza interface através do SystemView
   * @note Bloqueante até seleção do usuário
   *
   * @see SystemView Para renderização da interface
   */
  void wifi_atk_menu();

  /**
   * @brief Menu de ataques para alvo específico
   *
   * Exibe opções de ataque para uma rede WiFi específica,
   * incluindo informações detalhadas do alvo.
   *
   * @param tssid SSID da rede alvo
   * @param mac Endereço MAC do AP alvo
   * @param channel Canal WiFi da rede (1-14 para 2.4GHz)
   *
   * @note Valida parâmetros antes de exibir menu
   * @note Pode incluir verificações de segurança
   *
   * @see target_atk() Para execução de ataques
   */
  void target_atk_menu(String tssid, String mac, uint8_t channel);

  /**
   * @brief Executa ataque contra alvo específico
   *
   * Coordena e executa ataques selecionados contra uma rede WiFi.
   * Suporta múltiplos tipos de ataque baseados na configuração.
   *
   * @param tssid SSID da rede alvo
   * @param mac Endereço MAC do AP alvo
   * @param channel Canal WiFi da rede
   *
   * @note Operação pode interferir com outras atividades WiFi
   * @note Alguns ataques requerem permissões especiais
   *
   * @see capture_handshake() Para captura específica
   * @see beaconAttack() Para ataques beacon
   */
  void target_atk(String tssid, String mac, uint8_t channel);

  /**
   * @brief Captura handshake WPA de uma rede específica
   *
   * Executa ataque de desautenticação seguido de captura do
   * handshake de 4 vias WPA para cracking offline.
   *
   * @param tssid SSID da rede alvo
   * @param mac Endereço MAC do AP alvo
   * @param channel Canal WiFi da rede
   *
   * @note Salva handshake em arquivo para processamento posterior
   * @note Requer ferramenta externa (hashcat, aircrack) para cracking
   *
   * @see target_atk() Para ataques mais gerais
   */
  void capture_handshake(String tssid, String mac, uint8_t channel);

  /**
   * @brief Executa ataque Beacon Flood
   *
   * Inunda o ar com beacons falsos criando redes WiFi fantasma.
   * Útil para testes de resistência a ataques DoS em redes.
   *
   * @note Pode causar interferência significativa
   * @note Detectável por ferramentas de monitoramento
   *
   * @see target_atk() Para ataques direcionados
   */
  void beaconAttack();

  /**
   * @brief Classifica dispositivo WiFi usando Machine Learning
   *
   * Utiliza o módulo ML para identificar o tipo de dispositivo baseado
   * em características como RSSI, frequência e canal.
   *
   * @param rssi Nível de sinal RSSI
   * @param frequency Frequência do sinal
   * @param channel Canal WiFi
   * @param ssid Nome da rede (opcional)
   * @return Tipo de dispositivo identificado
   *
   * @note Requer módulo ML carregado e treinado
   * @see MLModule Para mais detalhes sobre classificação
   */
  int classifyDevice(float rssi, float frequency, int channel,
                     String ssid = "");

  int performWiFiScan();

  /**
   * @brief Inicia scanning WiFi assíncrono usando multi-core
   */
  void startAsyncScan();

  /**
   * @brief Para scanning assíncrono
   */
  void stopAsyncScan();

private:
  /**
   * @brief Ponteiro para o modelo do sistema
   *
   * Acesso à configuração global, estado do sistema e dados compartilhados.
   */
  std::shared_ptr<SystemModel> model_;

  /**
   * @brief Ponteiro para a visão/interface do sistema
   *
   * Acesso às funções de renderização e interação com usuário.
   */
  std::shared_ptr<SystemView> view_;

  /**
   * @brief Flag de atividade do módulo
   *
   * Indica se o módulo está pronto para processar operações.
   */
  bool active_;

  /**
   * @brief Flag de inicialização interna
   *
   * Controla estado detalhado da inicialização do módulo.
   */
  bool initialized_;

  /**
   * @brief Cache otimizado para resultados de scanning
   *
   * Armazena resultados recentes para reduzir escaneamentos desnecessários.
   */
  struct ScanCache {
    std::vector<String> ssids;
    std::vector<String> macs;
    std::vector<int> rssis;
    std::vector<uint8_t> channels;
    uint32_t lastScanTime;
    uint32_t cacheValidityMs;

    ScanCache() : lastScanTime(0), cacheValidityMs(30000) {}
  } scanCache_;

  /**
   * @brief Limiter de taxa para ataques
   *
   * Previne abuso e sobrecarga do sistema.
   */
  struct AttackLimiter {
    uint32_t lastActionTime;
    uint32_t minIntervalMs;
    uint32_t actionCount;
    uint32_t maxActionsPerMinute;

    AttackLimiter()
        : lastActionTime(0), minIntervalMs(1000), actionCount(0),
          maxActionsPerMinute(60) {}

    bool allowAction() {
      uint32_t now = millis();
      if (now - lastActionTime < minIntervalMs)
        return false;

      // Reset counter every minute
      if (now - lastActionTime > 60000) {
        actionCount = 0;
      }

      if (actionCount >= maxActionsPerMinute)
        return false;

      lastActionTime = now;
      actionCount++;
      return true;
    }
  } attackLimiter_;

  /**
   * @brief Estado do scanning assíncrono
   */
  bool asyncScanActive_;
  TaskHandle_t asyncScanTask_;

  // ========== Métodos privados otimizados ==========

  /**
   * @brief Aplica técnicas de stealth para reduzir detecção
   */
  void applyStealthTechniques();

  /**
   * @brief Otimiza recursos para ataques longos
   */
  void optimizeForLongAttacks();

  /**
   * @brief Muda para próximo canal WiFi
   */
  void nextChannel();

  /**
   * @brief Limpa cache de scanning expirado
   */
  void cleanupExpiredCache();

  /**
   * @brief Monitora uso de memória e otimiza alocação
   */
  void monitorMemoryUsage();

  /**
   * @brief Logging otimizado com níveis de severidade
   */
  void logWiFiEvent(const String &event, const String &details = "");
};

#endif // __WIFI_MODULE_H__
