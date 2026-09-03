#ifndef __SYSTEM_MANAGER_H__
#define __SYSTEM_MANAGER_H__

#include "../modules/plugins/PluginManager.h"
#include "IModule.h"
#include <ArduinoJson.h>
#include <memory>
#include <vector>


/**
 * @brief Gerenciador central de módulos do sistema Willy Firmware
 *
 * O SystemManager é responsável por coordenar todos os módulos do sistema,
 * implementando o padrão de registro e gerenciamento centralizado. Ele mantém
 * um registro de todos os módulos disponíveis e coordena suas operações de
 * ciclo de vida (inicialização, processamento e desinicialização).
 *
 * Funcionalidades principais:
 * - Registro dinâmico de módulos que implementam IModule
 * - Inicialização ordenada de todos os módulos
 * - Processamento coordenado no loop principal
 * - Acesso aos módulos por nome/identificador
 * - Gerenciamento de ciclo de vida e cleanup
 *
 * O SystemManager implementa o padrão Singleton para garantir acesso global
 * consistente aos módulos do sistema.
 *
 * @note Gerencia o ownership dos módulos através de unique_ptr
 * @note Thread-safe para operações críticas em ambiente FreeRTOS
 * @note Suporta hot-plugging de módulos (registro em tempo de execução)
 *
 * @author Willy Firmware Team
 * @version 1.0.0
 * @date 2024
 *
 * @see IModule Interface que todos os módulos devem implementar
 * @see WiFiModule Exemplo de módulo gerenciado
 * @see SystemController Para coordenação de alto nível
 */
class SystemManager {
public:
  /**
   * @brief Obtém a instância singleton do gerenciador
   *
   * Implementa o padrão Singleton para acesso global único
   * ao gerenciador de módulos.
   *
   * @return Referência para a instância única do SystemManager
   *
   * @note Thread-safe para uso concorrente
   */
  static SystemManager &getInstance();

  // Proibir cópia e atribuição - padrão Singleton
  SystemManager(const SystemManager &) = delete;
  SystemManager &operator=(const SystemManager &) = delete;

  /**
   * @brief Registra um novo módulo no sistema
   *
   * Adiciona um módulo ao registro central do sistema. O módulo deve
   * implementar a interface IModule. O SystemManager assume ownership
   * do módulo através do unique_ptr.
   *
   * @param module Ponteiro único para a instância do módulo
   *
   * @note O módulo será inicializado automaticamente durante initAllModules()
   * @note Módulos com mesmo nome sobrescreverão registros anteriores
   * @note O módulo deve estar em estado não-inicializado
   *
   * @see IModule Interface do módulo
   * @see initAllModules() Para inicialização automática
   */
  void registerModule(std::unique_ptr<IModule> module);

  /**
   * @brief Inicializa todos os módulos registrados
   *
   * Executa init() em todos os módulos registrados, nesta ordem:
   * 1. WiFiModule (prioridade alta - conectividade)
   * 2. RFModule e RFIDModule (hardware dependente)
   * 3. Outros módulos (ordem de registro)
   *
   * @return true se todos os módulos inicializaram com sucesso
   * @return false se qualquer módulo falhou na inicialização
   *
   * @note Módulos que falham são marcados como inativos
   * @note Ordem de inicialização pode afetar dependências
   * @note Chamado automaticamente pelo SystemController
   *
   * @see IModule::init()
   * @see deinitAllModules() Para limpeza
   */
  bool initAllModules();

  /**
   * @brief Desinicializa todos os módulos registrados
   *
   * Executa deinit() em todos os módulos na ordem reversa de inicialização.
   * Garante limpeza adequada de recursos e estado consistente.
   *
   * @note Chamado durante shutdown do sistema
   * @note Pode ser chamado múltiplas vezes sem efeitos colaterais
   * @note Módulos permanecem registrados após desinicialização
   *
   * @see IModule::deinit()
   * @see initAllModules() Para reinicialização
   */
  void deinitAllModules();

  /**
   * @brief Processa todos os módulos ativos
   *
   * Executa process() em todos os módulos que estão ativos (isActive() ==
   * true). Esta função é chamada a cada iteração do loop principal.
   *
   * @note Apenas módulos ativos são processados
   * @note Chamado frequentemente pelo SystemController::runMainLoop()
   * @note Deve ser não-bloqueante para manter responsividade
   *
   * @see IModule::process()
   * @see IModule::isActive()
   */
  void processAllModules();

  /**
   * @brief Obtém referência a um módulo específico por nome
   *
   * Permite acesso direto a módulos registrados para operações específicas.
   * Útil para comunicação inter-módulos ou acesso desde código legado.
   *
   * @param name Nome do módulo (retornado por IModule::getName())
   * @return Ponteiro para o módulo ou nullptr se não encontrado
   *
   * @note Retorna nullptr se módulo não existe ou não está registrado
   * @note O ponteiro retornado é válido enquanto o módulo estiver registrado
   * @note Não transfere ownership - apenas acesso
   *
   * @see IModule::getName()
   * @see registerModule()
   */
  IModule *getModule(const String &name);

  /**
   * @brief Lista todos os módulos registrados no sistema
   *
   * Exibe informações sobre todos os módulos registrados, incluindo:
   * - Nome do módulo
   * - Status de atividade
   * - Tipo/classe do módulo
   *
   * Útil para debug e monitoramento do sistema.
   *
   * @note Saída é enviada para Serial/log do sistema
   * @note Inclui módulos ativos e inativos
   *
   * @see IModule::getName()
   * @see IModule::isActive()
   */
  void listModules() const;

private:
  /**
   * @brief Construtor privado - padrão Singleton
   */
  SystemManager() = default;

  /**
   * @brief Destrutor privado - padrão Singleton
   */
  ~SystemManager() = default;

  /**
   * @brief Container de módulos registrados
   *
   * Vetor de ponteiros únicos mantendo ownership de todos os módulos.
   * Os módulos são armazenados na ordem de registro.
   */
  std::vector<std::unique_ptr<IModule>> modules_;
};

#endif // __SYSTEM_MANAGER_H__