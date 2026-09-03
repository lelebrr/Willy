#ifndef __SYSTEM_CONTROLLER_H__
#define __SYSTEM_CONTROLLER_H__

#include "SystemModel.h"
#include "SystemView.h"
#include <Arduino.h>

/**
 * @brief Controlador principal do sistema Willy Firmware - Implementação do
 * padrão MVC
 *
 * O SystemController é o componente central da arquitetura MVC
 * (Model-View-Controller) do Willy Firmware. Ele coordena a interação entre o
 * modelo de dados (SystemModel), a interface visual (SystemView) e os módulos
 * do sistema (através do SystemManager).
 *
 * Responsabilidades principais:
 * - Gerenciamento do ciclo de vida do sistema (inicialização/desinicialização)
 * - Coordenação do loop principal da aplicação
 * - Controle da navegação de menus e interface do usuário
 * - Gerenciamento do ciclo de vida de aplicações e módulos
 * - Processamento de entrada do usuário e eventos do sistema
 *
 * O controlador implementa o padrão Singleton para garantir uma única instância
 * global, acessível de qualquer parte do sistema.
 *
 * @note Esta classe é thread-safe para uso em ambiente FreeRTOS
 * @note Todas as operações são não-bloqueantes para manter responsividade
 *
 * @author Willy Firmware Team
 * @version 1.0.0
 * @date 2024
 *
 * @see SystemModel Para o modelo de dados do sistema
 * @see SystemView Para a interface visual
 * @see SystemManager Para gerenciamento de módulos
 * @see IModule Para interface de módulos
 */
class SystemController {
public:
  /**
   * @brief Obtém a instância singleton do controlador
   *
   * Implementa o padrão Singleton para garantir acesso global único
   * ao controlador do sistema.
   *
   * @return Referência para a instância única do SystemController
   *
   * @note Thread-safe para uso concorrente
   */
  static SystemController &getInstance();

  /**
   * @brief Inicializa o controlador e componentes do sistema
   *
   * Executa a sequência completa de inicialização:
   * 1. Inicializa o modelo de dados (SystemModel)
   * 2. Inicializa a interface visual (SystemView)
   * 3. Registra e inicializa todos os módulos via SystemManager
   * 4. Configura estado inicial do sistema
   *
   * @return true se inicialização bem-sucedida
   * @return false se ocorreu erro crítico durante inicialização
   *
   * @note Deve ser chamado apenas uma vez durante o boot do sistema
   * @note Em caso de falha, o sistema pode não funcionar corretamente
   *
   * @see deinit() Para limpeza de recursos
   * @see SystemManager::initAllModules()
   */
  bool init();

  /**
   * @brief Desinicializa o controlador e libera recursos
   *
   * Executa limpeza ordenada do sistema:
   * 1. Desinicializa todos os módulos ativos
   * 2. Libera recursos da interface visual
   * 3. Salva estado persistente se necessário
   * 4. Prepara sistema para shutdown
   *
   * @note Pode ser chamado múltiplas vezes sem efeitos colaterais
   */
  void deinit();

  /**
   * @brief Executa o loop principal do sistema
   *
   * Coordena o ciclo principal da aplicação:
   * 1. Processa entrada do usuário
   * 2. Atualiza estado do sistema
   * 3. Renderiza interface visual
   * 4. Executa módulos ativos
   * 5. Gerencia ciclo de vida de aplicações
   *
   * Esta função é chamada continuamente pelo loop() principal do
   * Arduino/FreeRTOS.
   *
   * @note Deve ser chamada frequentemente para manter responsividade
   * @note Implementa processamento não-bloqueante
   *
   * @see processMenuInput() Para processamento de entrada
   * @see SystemManager::processAllModules()
   */
  void runMainLoop();

  /**
   * @brief Exibe o menu principal do sistema
   *
   * Renderiza e ativa o menu principal na interface do usuário,
   * mostrando opções para acesso aos diferentes módulos e funcionalidades.
   *
   * @note Atualiza o estado visual através do SystemView
   * @note Pode ser chamado de qualquer contexto thread-safe
   *
   * @see SystemView::showMainMenu()
   */
  void showMainMenu();

  /**
   * @brief Processa entrada do usuário nos menus
   *
   * Captura e processa eventos de entrada (botões, toque, comandos seriais)
   * e atualiza o estado do sistema conforme necessário.
   *
   * @note Chamado automaticamente pelo runMainLoop()
   * @note Implementa debouncing e validação de entrada
   *
   * @see runMainLoop()
   */
  void processMenuInput();

  /**
   * @brief Inicia a aplicação de startup configurada
   *
   * Verifica a configuração do sistema e inicia a aplicação definida
   * como startup app, se configurada e disponível.
   *
   * @note Chamado durante a inicialização do sistema
   * @note Pode falhar silenciosamente se a app não estiver disponível
   *
   * @see SystemModel::getConfig()
   */
  void startStartupApp();

  /**
   * @brief Gerencia o ciclo de vida das aplicações
   *
   * Monitora e controla o estado das aplicações em execução,
   * incluindo inicialização, execução e finalização.
   *
   * @note Parte do sistema de gerenciamento de aplicações do Willy
   * @note Chamado periodicamente pelo runMainLoop()
   */
  void handleAppLifecycle();

  /**
   * @brief Atualiza o estado global do sistema
   *
   * Atualiza informações de estado baseadas em sensores, módulos ativos
   * e outras fontes de dados do sistema.
   *
   * @note Chamado periodicamente pelo runMainLoop()
   */
  void updateSystemState();

private:
  /**
   * @brief Construtor privado - padrão Singleton
   */
  SystemController() = default;

  /**
   * @brief Destrutor privado - padrão Singleton
   */
  ~SystemController() = default;

  /**
   * @brief Flag de inicialização do controlador
   *
   * Indica se o controlador foi inicializado corretamente.
   * Operações críticas verificam este flag antes de executar.
   */
  bool initialized_ = false;
};

#endif // __SYSTEM_CONTROLLER_H__