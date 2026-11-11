#ifndef __HARDWARE_PROFILES_H__
#define __HARDWARE_PROFILES_H__

#include "HardwareDetector.h"
#include "PeripheralAbstraction.h"
#include "PinAbstraction.h"
#include <Arduino.h>
#include <map>
#include <vector>


/**
 * @brief Perfil de hardware para uma placa específica
 */
struct HardwareProfile {
  String name;
  String description;
  ESP32Variant variant;
  std::vector<PinConfig> pin_configs;
  std::map<String, JsonDocument> peripheral_configs;
  std::map<String, String> metadata;

  HardwareProfile(const String &n = "", const String &desc = "")
      : name(n), description(desc), variant(ESP32Variant::UNKNOWN) {}
};

/**
 * @brief Gerenciador de profiles de hardware
 *
 * Carrega e aplica configurações de hardware específicas para
 * placas populares do ESP32.
 */
class HardwareProfiles {
public:
  static HardwareProfiles &getInstance();

  /**
   * @brief Carrega profiles padrão
   */
  void loadDefaultProfiles();

  /**
   * @brief Detecta e aplica profile automático baseado no hardware
   * @return true se profile aplicado com sucesso
   */
  bool autoDetectAndApplyProfile();

  /**
   * @brief Aplica profile específico
   * @param profile_name Nome do profile
   * @return true se aplicado com sucesso
   */
  bool applyProfile(const String &profile_name);

  /**
   * @brief Lista profiles disponíveis
   * @return Vetor de nomes de profiles
   */
  std::vector<String> listProfiles();

  /**
   * @brief Obtém informações de um profile
   * @param name Nome do profile
   * @return Profile ou nullptr se não encontrado
   */
  const HardwareProfile *getProfile(const String &name) const;

  /**
   * @brief Cria profile personalizado
   * @param profile Profile a ser adicionado
   * @return true se criado com sucesso
   */
  bool createCustomProfile(const HardwareProfile &profile);

  /**
   * @brief Salva profiles em arquivo
   * @param path Caminho do arquivo
   * @return true se salvo com sucesso
   */
  bool saveProfilesToFile(const String &path);

  /**
   * @brief Carrega profiles de arquivo
   * @param path Caminho do arquivo
   * @return true se carregado com sucesso
   */
  bool loadProfilesFromFile(const String &path);

  /**
   * @brief Gera relatório do profile atual
   * @return String com relatório
   */
  String generateProfileReport();

private:
  HardwareProfiles();
  ~HardwareProfiles() = default;

  HardwareProfiles(const HardwareProfiles &) = delete;
  HardwareProfiles &operator=(const HardwareProfiles &) = delete;

  /**
   * @brief Cria profiles padrão para placas populares
   */
  void createESP32S3Profiles();
  void createESP32S2Profiles();
  void createESP32C3Profiles();
  void createGenericProfiles();

  /**
   * @brief Aplica configuração de pinos de um profile
   * @param profile Profile a aplicar
   * @return true se aplicado com sucesso
   */
  bool applyPinConfiguration(const HardwareProfile &profile);

  /**
   * @brief Aplica configuração de periféricos de um profile
   * @param profile Profile a aplicar
   * @return true se aplicado com sucesso
   */
  bool applyPeripheralConfiguration(const HardwareProfile &profile);

  std::map<String, HardwareProfile> profiles_;
  String current_profile_;
};

#endif // __HARDWARE_PROFILES_H__