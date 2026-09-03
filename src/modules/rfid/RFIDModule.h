#ifndef __RFID_MODULE_H__
#define __RFID_MODULE_H__

#include "core/IModule.h"
#include "core/SystemModel.h"
#include "core/SystemView.h"
#include <memory>

/**
 * @brief Módulo RFID - implementa funcionalidades RFID usando padrão IModule
 */
class RFIDModule : public IModule {
public:
  RFIDModule(std::shared_ptr<SystemModel> model,
             std::shared_ptr<SystemView> view);
  virtual ~RFIDModule() = default;

  // Implementação da interface IModule
  bool init() override;
  void deinit() override;
  void process() override;
  String getName() const override { return "RFID"; }
  bool isActive() const override { return active_; }
  int getPriority() const override { return 90; }
  bool executeCommand(const String &command, JsonDocument &result) override;

  // Funcionalidades RFID específicas
  bool readRFID();

private:
  std::shared_ptr<SystemModel> model_;
  std::shared_ptr<SystemView> view_;
  bool active_;

  // Estado interno do módulo
  bool initialized_;
};

#endif // __RFID_MODULE_H__
