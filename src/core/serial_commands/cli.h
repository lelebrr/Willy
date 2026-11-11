#ifndef __SERIAL_CLI_H__
#define __SERIAL_CLI_H__

#include "../../lib/stubs/SimpleCLI.h"
#include "../advanced_logger.h"
#include <Arduino.h>

class SerialCli {
public:
  SerialCli();
  void setup(void);

  SimpleCLI getCli() { return _cli; };
  bool parse(const String &input) {
    _cli.parse(input);
    return true;
  }

  // Validação de entrada segura
  bool validateInput(const String &input);

private:
  SimpleCLI _cli;
  AdvancedLogger &logger = AdvancedLogger::getInstance();
};

void cliErrorCallback(cmd_error *e);



#endif

// Additional commands can be added following Flipper CLI conventions
// See: https://docs.flipper.net/development/cli
