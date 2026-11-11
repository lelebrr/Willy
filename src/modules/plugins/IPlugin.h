#ifndef __IPLUGIN_H__
#define __IPLUGIN_H__

#include <Arduino.h>
#include <ArduinoJson.h>

/**
 * @brief Interface base para todos os plugins do sistema Willy Firmware
 *
 * Esta interface define o contrato padrão que todos os plugins do sistema devem
 * implementar. Ela padroniza as operações de ciclo de vida dos plugins
 * (inicialização, processamento e desinicialização), permitindo que o
 * PluginManager coordene todos os plugins de forma uniforme.
 *
 * Plugins que implementam esta interface podem ser:
 * - Plugins nativos (C++ compilados)
 * - Plugins baseados em JSON (configurações)
 * - Plugins baseados em scripts (Lua/Python - futuro)
 *
 * @note Todos os métodos são virtuais puros, garantindo implementação
 * obrigatória
 *
 * @author Willy Firmware Team
 * @version 1.0.0
 * @date 2024
 *
 * @see PluginManager Para gerenciamento centralizado dos plugins
 * @see PluginRegistry Para descoberta automática
 */
class IPlugin {
public:
  /**
   * @brief Destrutor virtual padrão
   *
   * Garante limpeza adequada de recursos quando o plugin é destruído.
   * Implementações devem liberar recursos alocados durante init().
   */
  virtual ~IPlugin() = default;

  /**
   * @brief Inicializa o plugin e seus recursos
   *
   * Este método é chamado durante o carregamento do plugin para preparar
   * o plugin para operação. Deve inicializar estado interno, configurar
   * hooks se necessário e validar a configuração.
   *
   * @param config Configuração JSON do plugin (pode ser vazio para plugins
   * nativos)
   * @return true se a inicialização foi bem-sucedida
   * @return false se ocorreu erro durante inicialização
   *
   * @note Deve ser chamado apenas uma vez durante o ciclo de vida do plugin
   * @note Em caso de falha, o plugin não será registrado no PluginManager
   * @note Validações de segurança são aplicadas automaticamente
   *
   * @see deinit() Para limpeza de recursos
   * @see PluginManager::loadPlugin()
   * @see SecurityUtils Para validações de segurança
   */
  virtual bool init(const JsonDocument &config) = 0;

  /**
   * @brief Desinicializa o plugin e libera recursos
   *
   * Método chamado durante o descarregamento do plugin ou shutdown do sistema.
   * Deve liberar todos os recursos alocados durante init() e remover hooks.
   *
   * @note Pode ser chamado múltiplas vezes sem efeitos colaterais
   * @note Após chamada, o plugin pode ser destruído ou reinicializado
   *
   * @see init() Para inicialização de recursos
   * @see PluginManager::unloadPlugin()
   */
  virtual void deinit() = 0;

  /**
   * @brief Processa operações periódicas do plugin
   *
   * Executado a cada iteração do loop principal do sistema ou quando
   * hooks são disparados. Deve realizar processamento não-bloqueante.
   *
   * @note Deve ser implementado de forma não-bloqueante
   * @note Chamado frequentemente ou baseado em eventos
   *
   * @see PluginManager::processPlugins()
   */
  virtual void process() = 0;

  /**
   * @brief Retorna o nome identificador do plugin
   *
   * Fornece um nome único e descritivo para o plugin, usado para:
   * - Identificação em logs e debug
   * - Acesso via PluginManager::getPlugin()
   * - Gerenciamento via comandos seriais
   *
   * @return String contendo o nome do plugin
   *
   * @note Deve retornar sempre a mesma string para o mesmo plugin
   * @note Nomes devem ser únicos entre todos os plugins carregados
   *
   * @see PluginManager::getPlugin()
   */
  virtual String getName() const = 0;

  /**
   * @brief Retorna a versão do plugin
   *
   * @return String contendo a versão (ex: "1.0.0")
   */
  virtual String getVersion() const = 0;

  /**
   * @brief Retorna a descrição do plugin
   *
   * @return String contendo descrição do que o plugin faz
   */
  virtual String getDescription() const = 0;

  /**
   * @brief Verifica se o plugin está ativo e operacional
   *
   * @return true se o plugin está ativo
   * @return false se o plugin está inativo ou com problemas
   */
  virtual bool isActive() const = 0;

  /**
   * @brief Hook executado antes de uma operação crítica
   *
   * Chamado pelo sistema antes de operações importantes como:
   * - Inicialização de módulos
   * - Processamento de comandos
   * - Operações de rede
   *
   * @param operation Nome da operação
   * @param params Parâmetros da operação (JSON)
   * @return true para permitir a operação, false para bloquear
   */
  virtual bool onPreHook(const String &operation, const JsonDocument &params) {
    return true;
  }

  /**
   * @brief Hook executado após uma operação crítica
   *
   * Chamado pelo sistema após operações importantes.
   *
   * @param operation Nome da operação
   * @param success Se a operação foi bem-sucedida
   * @param result Resultado da operação (JSON)
   */
  virtual void onPostHook(const String &operation, bool success,
                          const JsonDocument &result) {}

  /**
   * @brief Hook para processamento de comandos customizados
   *
   * Permite que plugins interceptem e processem comandos específicos.
   *
   * @param command Comando recebido
   * @param args Argumentos do comando
   * @return true se o comando foi processado pelo plugin
   */
  virtual bool onCommand(const String &command,
                         const std::vector<String> &args) {
    return false;
  }
};

#endif // __IPLUGIN_H__