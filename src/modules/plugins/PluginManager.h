#ifndef __PLUGIN_MANAGER_H__
#define __PLUGIN_MANAGER_H__

#include "IPlugin.h"
#include <ArduinoJson.h>
#include <map>
#include <memory>
#include <vector>


/**
 * @brief Gerenciador central de plugins do sistema Willy Firmware
 *
 * O PluginManager é responsável por coordenar todos os plugins do sistema,
 * implementando carregamento dinâmico, gerenciamento de ciclo de vida e
 * sistema de hooks para extensibilidade.
 *
 * Funcionalidades principais:
 * - Carregamento dinâmico de plugins (nativos, JSON, scripts)
 * - Gerenciamento de ciclo de vida dos plugins
 * - Sistema de hooks para operações críticas
 * - Processamento coordenado no loop principal
 * - Acesso aos plugins por nome/identificador
 * - Integração com sistema de segurança
 *
 * O PluginManager implementa o padrão Singleton para acesso global
 * consistente aos plugins do sistema.
 *
 * @note Gerencia o ownership dos plugins através de unique_ptr
 * @note Thread-safe para operações críticas em ambiente FreeRTOS
 * @note Suporta hot-plugging de plugins em tempo de execução
 *
 * @author Willy Firmware Team
 * @version 1.0.0
 * @date 2024
 *
 * @see IPlugin Interface que todos os plugins devem implementar
 * @see PluginRegistry Para descoberta automática de plugins
 * @see SecurityUtils Para validação de plugins
 */
class PluginManager {
public:
  /**
   * @brief Obtém a instância singleton do gerenciador
   *
   * Implementa o padrão Singleton para acesso global único
   * ao gerenciador de plugins.
   *
   * @return Referência para a instância única do PluginManager
   *
   * @note Thread-safe para uso concorrente
   */
  static PluginManager &getInstance();

  /**
   * @brief Inicializa o sistema de plugins
   *
   * Deve ser chamado durante o setup do sistema para preparar
   * o gerenciador de plugins.
   *
   * @return true se inicialização bem-sucedida
   */
  bool init();

  /**
   * @brief Desinicializa o sistema de plugins
   *
   * Chamado durante o shutdown do sistema para liberar recursos.
   */
  void deinit();

  /**
   * @brief Carrega um plugin pelo nome
   *
   * Carrega um plugin da registry ou SD card baseado no nome.
   *
   * @param name Nome do plugin
   * @param config Configuração JSON (opcional)
   * @return true se carregamento bem-sucedido
   */
  bool loadPlugin(const String &name,
                  const JsonDocument &config = JsonDocument());

  /**
   * @brief Descarrega um plugin
   *
   * Remove um plugin carregado e libera seus recursos.
   *
   * @param name Nome do plugin
   * @return true se descarregamento bem-sucedido
   */
  bool unloadPlugin(const String &name);

  /**
   * @brief Obtém um plugin pelo nome
   *
   * @param name Nome do plugin
   * @return Ponteiro para o plugin ou nullptr se não encontrado
   */
  IPlugin *getPlugin(const String &name) const;

  /**
   * @brief Lista todos os plugins carregados
   *
   * @return Vetor com nomes dos plugins carregados
   */
  std::vector<String> listLoadedPlugins() const;

  /**
   * @brief Lista todos os plugins disponíveis
   *
   * Inclui plugins na registry e na SD card.
   *
   * @return Vetor com nomes dos plugins disponíveis
   */
  std::vector<String> listAvailablePlugins() const;

  /**
   * @brief Processa todos os plugins carregados
   *
   * Chamado no loop principal para dar oportunidade aos plugins
   * de processarem suas operações.
   */
  void processPlugins();

  /**
   * @brief Executa hooks pré-operação
   *
   * Chama todos os plugins que implementam onPreHook para a operação.
   *
   * @param operation Nome da operação
   * @param params Parâmetros da operação
   * @return true se todos os plugins permitem a operação
   */
  bool executePreHooks(const String &operation, const JsonDocument &params);

  /**
   * @brief Executa hooks pós-operação
   *
   * Chama todos os plugins que implementam onPostHook para a operação.
   *
   * @param operation Nome da operação
   * @param success Se a operação foi bem-sucedida
   * @param result Resultado da operação
   */
  void executePostHooks(const String &operation, bool success,
                        const JsonDocument &result);

  /**
   * @brief Processa comando através dos plugins
   *
   * Permite que plugins interceptem comandos customizados.
   *
   * @param command Comando
   * @param args Argumentos
   * @return true se algum plugin processou o comando
   */
  bool processCommand(const String &command, const std::vector<String> &args);

  /**
   * @brief Carrega plugins da SD card
   *
   * Escaneia o diretório de plugins na SD card e carrega configurações JSON.
   *
   * @return Número de plugins carregados
   */
  int loadPluginsFromSD();

  /**
   * @brief Recarrega todos os plugins
   *
   * Descarrega e recarrega todos os plugins ativos.
   */
  void reloadAllPlugins();

private:
  /**
   * @brief Construtor privado (Singleton)
   */
  PluginManager() = default;

  /**
   * @brief Destrutor privado
   */
  ~PluginManager() = default;

  /**
   * @brief Impede cópia
   */
  PluginManager(const PluginManager &) = delete;
  PluginManager &operator=(const PluginManager &) = delete;

  /**
   * @brief Cria instância de plugin baseado no tipo
   *
   * Factory method para criar plugins de diferentes tipos.
   *
   * @param type Tipo do plugin ("native", "json", "lua", "python")
   * @param name Nome do plugin
   * @param config Configuração
   * @return Ponteiro único para o plugin criado
   */
  std::unique_ptr<IPlugin> createPlugin(const String &type, const String &name,
                                        const JsonDocument &config);

  /**
   * @brief Valida plugin com sistema de segurança
   *
   * @param plugin Plugin a validar
   * @param config Configuração do plugin
   * @return true se válido
   */
  bool validatePlugin(IPlugin *plugin, const JsonDocument &config);

  // Membros privados
  std::map<String, std::unique_ptr<IPlugin>>
      loadedPlugins_;        ///< Plugins carregados
  bool initialized_ = false; ///< Flag de inicialização
};

#endif // __PLUGIN_MANAGER_H__