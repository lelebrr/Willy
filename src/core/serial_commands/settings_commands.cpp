#include "settings_commands.h"
#include <globals.h>

uint32_t settingsCallback(cmd *c) {
  Command cmd(c);

  Argument setting_name_arg = cmd.getArgument("setting_name");
  Argument setting_value_arg = cmd.getArgument("setting_value");
  String setting_name = setting_name_arg.getValue();
  String setting_value = setting_value_arg.getValue();
  setting_name.trim();
  setting_value.trim();

  JsonDocument jsonDoc = willyConfig.toJson();
  JsonObject setting = jsonDoc.as<JsonObject>();

  if (setting_name.length() == 0 && setting_value.length() == 0) {
    // no args, just prints current config
    serializeJsonPretty(jsonDoc, Serial);
    serialDevice->println("");
    return true;
  }

  if (setting[setting_name].isNull()) {
    serialDevice->println("Invalid field name: " + setting_name);
    return false;
  }

  if (setting_value.length() == 0) {
    serialDevice->print(setting_name + " = ");
    serialDevice->println(setting[setting_name].as<String>());
    return true;
  }

  // Use centralized setSetting method in WillyConfig
  if (willyConfig.setSetting(setting_name, setting_value)) {
    return true;
  }

  // Handle pin-specific settings that belong to willyConfigPins
  if (setting_name == "rot") {
    willyConfigPins.setRotation(setting_value.toInt());
    return true;
  }
  if (setting_name == "bleName") {
    willyConfigPins.setBleName(setting_value);
    return true;
  }
  if (setting_name == "irTx") {
    willyConfigPins.setIrTxPin(setting_value.toInt());
    return true;
  }
  if (setting_name == "irTxRepeats") {
    willyConfigPins.setIrTxRepeats(static_cast<uint8_t>(setting_value.toInt()));
    return true;
  }
  if (setting_name == "irRx") {
    willyConfigPins.setIrRxPin(setting_value.toInt());
    return true;
  }
  if (setting_name == "rfTx") {
    willyConfigPins.setRfTxPin(setting_value.toInt());
    return true;
  }
  if (setting_name == "rfRx") {
    willyConfigPins.setRfRxPin(setting_value.toInt());
    return true;
  }
  if (setting_name == "rfModule") {
    willyConfigPins.setRfModule(static_cast<RFModules>(setting_value.toInt()));
    return true;
  }
  if (setting_name == "rfFreq" && setting_value.toFloat()) {
    willyConfigPins.setRfFreq(setting_value.toFloat());
    return true;
  }
  if (setting_name == "rfFxdFreq") {
    willyConfigPins.setRfFxdFreq(setting_value.toInt());
    return true;
  }
  if (setting_name == "rfScanRange") {
    willyConfigPins.setRfScanRange(setting_value.toInt());
    return true;
  }
  if (setting_name == "rfidModule") {
    willyConfigPins.setRfidModule(
        static_cast<RFIDModules>(setting_value.toInt()));
    return true;
  }

  return false;
}

uint32_t factoryResetCallback(cmd *c) {
  willyConfig.factoryReset();
  serialDevice->println("Factory reset done");
  return true;
}

void createSettingsCommands(SimpleCLI *cli) {
  cli->addCommand("factory_reset", factoryResetCallback);

  Command cmd = cli->addCommand("set/tings", settingsCallback);
  cmd.addPosArg("setting_name", "");
  cmd.addPosArg("setting_value", "");
}
