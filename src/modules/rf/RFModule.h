#ifndef __RF_MODULE_H__
#define __RF_MODULE_H__

#include "core/IModule.h"
#include "core/SystemModel.h"
#include "core/SystemView.h"
#include <memory>

/**
 * @brief Módulo RF otimizado - implementa funcionalidades RF usando padrão
 * IModule Inclui validações de segurança, tratamento de erros robusto, logging
 * avançado e conformidade com protocolos RF (433MHz, 868MHz, 915MHz, 2.4GHz)
 */
class RFModule : public IModule {
public:
  RFModule(std::shared_ptr<SystemModel> model,
           std::shared_ptr<SystemView> view);
  virtual ~RFModule() = default;

  // Implementação da interface IModule
  bool init() override;
  void deinit() override;
  void process() override;
  String getName() const override { return "RF"; }
  bool isActive() const override { return active_; }
  int getPriority() const override { return 80; }
  bool executeCommand(const String &command, JsonDocument &result) override;

  // Funcionalidades RF específicas otimizadas
  bool transmitRF();

private:
  std::shared_ptr<SystemModel> model_;
  std::shared_ptr<SystemView> view_;
  bool active_;
  bool initialized_;

  // Métodos auxiliares para otimização e segurança
  bool checkRfHealth();
  bool recoverRfModule();
  bool validateTransmissionParameters();
};

#endif // __RF_MODULE_H__
