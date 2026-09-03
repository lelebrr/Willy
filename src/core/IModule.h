#ifndef __IMODULE_H__
#define __IMODULE_H__

#include <Arduino.h>
#include <ArduinoJson.h>

/**
 * @brief Interface base para todos os módulos do sistema Willy Firmware
 *
 * Esta interface define o contrato padrão que todos os módulos do sistema devem
 * implementar. Ela padroniza as operações de ciclo de vida dos módulos
 * (inicialização, processamento e desinicialização), permitindo que o
 * SystemManager coordene todos os módulos de forma uniforme.
 *
 * Módulos que implementam esta interface incluem:
 * - WiFiModule: Gerenciamento de redes WiFi e ataques
 * - RFModule: Operações com rádio frequência (RF 433MHz, 868MHz, etc.)
 * - RFIDModule: Leitura e escrita de tags RFID/NFC
 * - BLEModule: Operações Bluetooth Low Energy
 * - IRModule: Controle e captura de sinais infravermelhos
 *
 * @note Todos os métodos são virtuais puros, garantindo implementação
 * obrigatória
 * @note O padrão Singleton é usado para acesso global aos módulos
 *
 * @author Willy Firmware Team
 * @version 1.0.0
 * @date 2024
 *
 * @see SystemManager Para gerenciamento centralizado dos módulos
 * @see WiFiModule Exemplo de implementação concreta
 */
class IModule {
public:
  /**
   * @brief Destrutor virtual padrão
   *
   * Garante limpeza adequada de recursos quando o módulo é destruído.
   * Implementações devem liberar recursos alocados durante init().
   */
  virtual ~IModule() = default;

  /**
   * @brief Inicializa o módulo e seus recursos
   *
   * Este método é chamado durante o setup do sistema para preparar o módulo
   * para operação. Deve inicializar hardware, alocar recursos e configurar
   * estado interno.
   *
   * @return true se a inicialização foi bem-sucedida
   * @return false se ocorreu erro durante inicialização

   * @note Deve ser chamado apenas uma vez durante o ciclo de vida do módulo
   * @note Em caso de falha, o módulo não será registrado no SystemManager
   *
   * @see deinit() Para limpeza de recursos
   * @see SystemManager::initAllModules()
   */
  virtual bool init() = 0;

  /**
   * @brief Desinicializa o módulo e libera recursos
   *
   * Método chamado durante o shutdown do sistema ou quando o módulo
   * precisa ser removido. Deve liberar todos os recursos alocados
   * durante init() e deixar o hardware em estado seguro.
   *
   * @note Pode ser chamado múltiplas vezes sem efeitos colaterais
   * @note Após chamada, o módulo pode ser destruído ou reinicializado
   *
   * @see init() Para inicialização de recursos
   * @see SystemManager::deinitAllModules()
   */
  virtual void deinit() = 0;

  /**
   * @brief Processa operações periódicas do módulo
   *
   * Executado a cada iteração do loop principal do sistema. Deve realizar
   * processamento não-bloqueante, verificações de estado e atualizações
   * necessárias para funcionamento contínuo do módulo.
   *
   * @note Deve ser implementado de forma não-bloqueante
   * @note Chamado frequentemente (tipicamente 60+ vezes por segundo)
   * @note Não deve conter delays ou operações de longa duração
   *
   * @see SystemController::runMainLoop()
   * @see SystemManager::processAllModules()
   */
  virtual void process() = 0;

  /**
   * @brief Retorna o nome identificador do módulo
   *
   * Fornece um nome único e descritivo para o módulo, usado para:
   * - Identificação em logs e debug
   * - Acesso via SystemManager::getModule()
   * - Exibição na interface do usuário
   *
   * @return String contendo o nome do módulo (ex: "WiFi", "RFID")
   *
   * @note Deve retornar sempre a mesma string para o mesmo módulo
   * @note Nomes devem ser únicos entre todos os módulos do sistema
   *
   * @see SystemManager::getModule()
   */
  virtual String getName() const = 0;

  /**
   * @brief Verifica se o módulo está operacional
   *
   * Indica se o módulo está pronto para processar operações. Um módulo
   * pode estar inativo por diversos motivos:
   * - Falha na inicialização
   * - Hardware desconectado
   * - Modo de economia de energia
   * - Configuração desabilitada pelo usuário
   *
   * @return true se o módulo está ativo e operacional
   * @return false se o módulo está inativo ou com problemas
   *
   * @note Módulos inativos ainda são processados, mas podem ignorar operações
   * @note Usado pelo SystemManager para determinar quais módulos processar
   *
   * @see SystemManager::processAllModules()
   */
  virtual bool isActive() const = 0;

  /**
   * @brief Retorna a prioridade de processamento do módulo
   *
   * Define a ordem de processamento dos módulos no sistema. Módulos com
   * prioridade mais alta são processados primeiro.
   *
   * @return int valor da prioridade (maior = mais prioritário)
   *
   * @note Valores típicos: 0 = baixa, 100 = alta
   * @note Usado para ordenação no SystemManager
   */
  virtual int getPriority() const = 0;

  /**
   * @brief Executa um comando genérico no módulo
   *
   * Permite executar operações específicas do módulo através de uma interface
   * genérica, evitando necessidade de cast para tipos específicos.
   *
   * @param command Nome do comando a executar
   * @param result Documento JSON com resultado da operação
   * @return true se comando executado com sucesso
   *
   * @note Implementação deve validar comando e parâmetros
   */
  virtual bool executeCommand(const String &command, JsonDocument &result) = 0;
};

#endif // __IMODULE_H__
