#ifndef __PLUGIN_REGISTRY_H__
#define __PLUGIN_REGISTRY_H__

#include "IPlugin.h"
#include <ArduinoJson.h>
#include <functional>
#include <map>
#include <memory>

/**
 * @brief Registro de fábricas de plugins nativos
 *
 * O PluginRegistry mantém um registro de fábricas para criação de plugins
 * nativos (C++). Permite que plugins sejam registrados em tempo de compilação
 * ou execução e criados dinamicamente pelo PluginManager.
 *
 * Funcionalidades principais:
 * - Registro de fábricas de plugins por nome
 * - Criação dinâmica de plugins nativos
 * - Descoberta automática de plugins disponíveis
 *
 * O PluginRegistry implementa o padrão Singleton para acesso global
 * consistente às fábricas de plugins.
 *
 * @author Willy Firmware Team
 * @version 1.0.0
 * @date 2024
 *
 * @see PluginManager Para gerenciamento dos plugins criados
 * @see IPlugin Interface que os plugins devem implementar
 */
class PluginRegistry {
public:
  /**
   * @brief Tipo da função fábrica para criação de plugins
   */
  using PluginFactory = std::function<std::unique_ptr<IPlugin>(
      const String &name, const JsonDocument &config)>;

  /**
   * @brief Obtém a instância singleton do registro
   *
   * Implementa o padrão Singleton para acesso global único
   * ao registro de fábricas de plugins.
   *
   * @return Referência para a instância única do PluginRegistry
   *
   * @note Thread-safe para uso concorrente
   */
  static PluginRegistry &getInstance();

  /**
   * @brief Registra uma fábrica de plugin
   *
   * Registra uma função fábrica para criação de um tipo específico de plugin.
   *
   * @param name Nome identificador do plugin
   * @param factory Função fábrica que cria instâncias do plugin
   * @return true se registro bem-sucedido, false se já existe
   *
   * @note O nome deve ser único entre todos os plugins registrados
   */
  bool registerPlugin(const String &name, PluginFactory factory);

  /**
   * @brief Remove registro de uma fábrica de plugin
   *
   * @param name Nome do plugin a remover
   * @return true se remoção bem-sucedida
   */
  bool unregisterPlugin(const String &name);

  /**
   * @brief Cria um plugin usando sua fábrica registrada
   *
   * @param name Nome do plugin
   * @param config Configuração JSON
   * @return Ponteiro único para o plugin criado ou nullptr se falha
   */
  std::unique_ptr<IPlugin> createPlugin(const String &name,
                                        const JsonDocument &config) const;

  /**
   * @brief Verifica se um plugin está registrado
   *
   * @param name Nome do plugin
   * @return true se registrado
   */
  bool isRegistered(const String &name) const;

  /**
   * @brief Lista todos os plugins registrados
   *
   * @return Vetor com nomes dos plugins registrados
   */
  std::vector<String> listRegisteredPlugins() const;

private:
  /**
   * @brief Construtor privado (Singleton)
   */
  PluginRegistry() = default;

  /**
   * @brief Destrutor privado
   */
  ~PluginRegistry() = default;

  /**
   * @brief Impede cópia
   */
  PluginRegistry(const PluginRegistry &) = delete;
  PluginRegistry &operator=(const PluginRegistry &) = delete;

  // Membros privados
  std::map<String, PluginFactory> factories_; ///< Fábricas registradas
};

// Macros auxiliares para registro de plugins

/**
 * @brief Macro para registrar um plugin automaticamente
 *
 * Deve ser usado em arquivos .cpp para registrar plugins em tempo de
 * compilação.
 *
 * @param PLUGIN_CLASS Classe do plugin
 * @param PLUGIN_NAME Nome do plugin
 */
#define REGISTER_PLUGIN(PLUGIN_CLASS, PLUGIN_NAME)                             \
  static bool PLUGIN_CLASS##_registered = []() {                               \
    return PluginRegistry::getInstance().registerPlugin(                       \
        PLUGIN_NAME,                                                           \
        [](const String &name,                                                 \
           const JsonDocument &config) -> std::unique_ptr<IPlugin> {           \
          return std::unique_ptr<IPlugin>(new PLUGIN_CLASS(name, config));     \
        });                                                                    \
  }();

#endif // __PLUGIN_REGISTRY_H__