#include "badusb_commands.h"
#include "core/sd_functions.h"
#include "helpers.h"
#include "modules/badusb_ble/ducky_typer.h"
#include <SPIFFS.h>

uint32_t badusbFileCallback(cmd *c) {
#ifndef LITE_VERSION
  // badusb run_from_file HelloWorld.txt

  Command cmd(c);

  Argument arg = cmd.getArgument("filepath");
  String filepath = arg.getValue();
  filepath.trim();

  if (filepath.indexOf(".txt") == -1) {
    serialDevice->println("Invalid filename");
    return false;
  }
  if (!filepath.startsWith("/"))
    filepath = "/" + filepath;

  FS *fs;
  if (!getFsStorage(fs))
    return false;

  if (!(*fs).exists(filepath)) {
    serialDevice->println("File does not exist");
    return false;
  }

#ifdef USB_as_HID
  ducky_startKb(hid_usb, false);
  key_input(*fs, filepath, hid_usb);
  hid_usb->end();
  delete hid_usb;
  hid_usb = nullptr;

  // Reinit serial after USB HID usage
  Serial.begin(115200);

  return true;
#else
  return false;
#endif
#else
  return false;
#endif
}

uint32_t badusbBufferCallback(cmd *c) {
#ifndef LITE_VERSION
  if (!(_setupPsramFs()))
    return false;

  char *txt = _readFileFromSerial();
  if (!txt)
    return false;

  // Write buffer to temporary file for compatibility with key_input
  String tmpfilepath = "/tmpramfile";
  File f = ((FS&)SPIFFS).open(tmpfilepath, FILE_WRITE);
  if (!f) {
    free(txt);
    return false;
  }

  f.write((const uint8_t *)txt, strlen(txt));
  f.close();
  free(txt);

#ifdef USB_as_HID
  ducky_startKb(hid_usb, false);
  key_input((FS&)SPIFFS, tmpfilepath, hid_usb);
  delete hid_usb;
  hid_usb = nullptr;

  ((FS&)SPIFFS).remove(tmpfilepath);
  return true;
#else
  ((FS&)SPIFFS).remove(tmpfilepath);
  return false;
#endif
#else
  return false;
#endif
}

void createBadUsbCommands(SimpleCLI *cli) {
#ifndef LITE_VERSION
  Command badusbCmd = cli->addCompositeCmd("bu,badusb");

  Command fileCmd = badusbCmd.addCommand("run_from_file", badusbFileCallback);
  fileCmd.addPosArg("filepath");

  Command bufferCmd =
      badusbCmd.addCommand("run_from_buffer", badusbBufferCallback);
#endif
}
