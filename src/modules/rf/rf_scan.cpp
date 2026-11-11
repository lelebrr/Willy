#include "rf_scan.h"
#include "core/SecurityUtils.h"
#include "core/advanced_logger.h"
#include "core/led_control.h"
#include "core/sd_functions.h"
#include "core/type_convertion.h"
#include "record.h"
#include "rf_send.h"
#include "rf_utils.h"
#include <globals.h>
#include <sstream>

// Constantes otimizadas para rf_scan
static const char *TAG_RF_SCAN = "RFScan";
constexpr uint32_t SCAN_TIMEOUT_MS = 30000;     // 30 segundos timeout
constexpr uint32_t MAX_SIGNALS_PER_SCAN = 1000; // Limite de sinais por scan
constexpr uint32_t RSSI_UPDATE_INTERVAL_MS =
    100; // Atualização RSSI a cada 100ms

RFScan::RFScan() { setup(); }

RFScan::~RFScan() { deinitRfModule(); }

void RFScan::setup() {
  ADVANCED_LOGGER.log(LogLevel::INFO, TAG_RF_SCAN,
                      "Iniciando configuração do scan RF");

  // Validação de segurança: verificar acesso ao hardware RF
  if (!SecurityUtils::validateHardwareAccess("RF_SCAN")) {
    ADVANCED_LOGGER.log(LogLevel::ERROR, TAG_RF_SCAN,
                        "Acesso negado ao hardware RF para scan");
    displayError("Acesso RF negado");
    return;
  }

  // Validação de frequência
  if (!validateRfFrequency(willyConfigPins.rfFreq)) {
    ADVANCED_LOGGER.log(LogLevel::WARNING, TAG_RF_SCAN,
                        "Frequência RF inválida, usando padrão");
    willyConfigPins.rfFreq = 433.92f;
  }

  if (!initRfModule("rx", willyConfigPins.rfFreq)) {
    ADVANCED_LOGGER.log(LogLevel::ERROR, TAG_RF_SCAN,
                        "Falha na inicialização do módulo RF");
    displayError("Falha RF init");
    return;
  }

  RCSwitch_Enable_Receive(rcswitch);

  // Validações de configuração com limites seguros
  if (willyConfigPins.rfScanRange < 0 || willyConfigPins.rfScanRange > 3) {
    ADVANCED_LOGGER.log(LogLevel::WARNING, TAG_RF_SCAN,
                        "Range de scan inválido, definindo padrão");
    willyConfigPins.setRfScanRange(3);
  }

  if (willyConfigPins.rfModule != CC1101_SPI_MODULE) {
    willyConfigPins.setRfFxdFreq(1);
  }

  display_info(received, signals, ReadRAW, codesOnly, autoSave, title);

  if (willyConfigPins.rfFxdFreq)
    frequency = willyConfigPins.rfFreq;

  // Clear cache for RAW signal com validação
  rcswitch.resetAvailable();
  returnToMenu = false;
  restartScan = false;

  // Inicializar variáveis de controle otimizadas
  scan_start_time = millis();
  signals = 0;
  last_rssi_update = 0;

  ADVANCED_LOGGER.log(LogLevel::INFO, TAG_RF_SCAN,
                      "Configuração do scan RF concluída");
  return loop();
}

void RFScan::loop() {
  while (1) {
    if (check(EscPress) || returnToMenu)
      return;
    if (check(NextPress)) {
      select_menu_option();
      if (returnToMenu)
        return;
      return setup();
    }
    if (restartScan)
      return setup();

    if (willyConfigPins.rfFxdFreq)
      frequency = willyConfigPins.rfFreq;
    if (frequency <= 0) {
      frequency = rf_freq_scan();
      if (frequency > 0) {
        return setup(); // frequency found, reset
      }
    }

    if (rcswitch.available() && !ReadRAW) {
      read_rcswitch();
      if (autoSave && (lastSavedKey != received.key || received.key == 0))
        save_signal();
    }
    if (rcswitch.RAWavailable() && ReadRAW) {
      read_raw();
      if (autoSave && (lastSavedKey != received.key || received.key == 0))
        save_signal();
    }
  }
}

void RFScan::RCSwitch_Enable_Receive(RCSwitch rcswitch) {
  if (willyConfigPins.rfModule == CC1101_SPI_MODULE) {
    rcswitch.enableReceive(willyConfigPins.CC1101_bus.io0);
  } else {
    rcswitch.enableReceive(willyConfigPins.rfRx);
  }
}

void RFScan::read_rcswitch() {
  // Add decoded data only (if any) to the RCCode
  uint64_t decoded = rcswitch.getReceivedValue();

  if (decoded) { // if there is a value decoded by RCSwitch, show it
    Serial.println("Sinal RcSwitch capturado");
    blinkLed();
    ++signals;
    found_freq = frequency;
    received.frequency = long(frequency * 1000000);
    received.key = decoded;
    received.preset = String(rcswitch.getReceivedProtocol());
    received.protocol = "RcSwitch";
    received.te = rcswitch.getReceivedDelay();
    received.Bit = rcswitch.getReceivedBitlength();
    received.filepath = "signal_" + String(signals);
    received.data = "";

    frequency = 0;
    display_info(received, signals, ReadRAW, codesOnly, autoSave, title);
  }

  rcswitch.resetAvailable();
}

void RFScan::read_raw() {
  // Add RAW data (& decoded data if any) to the RCCode
  vTaskDelay(400 / portTICK_PERIOD_MS); // wait for all the signal to be read
  found_freq = frequency;
  unsigned int *raw = rcswitch.getRAWReceivedRawdata();
  uint64_t decoded = rcswitch.getReceivedValue();
  int transitions = 0;
  String _data = "";
  std::vector<int> durations;
  std::vector<int> indexed_durations;
  // uint64_t result = 0;
  uint8_t repetition = 0;

  received.te = 0;
  for (transitions = 0; transitions < RCSWITCH_RAW_MAX_CHANGES; transitions++) {
    if (raw[transitions] == 0)
      break;
    if (transitions > 0)
      _data += " ";
    signed int sign = (transitions % 2 == 0) ? 1 : -1;

    int duration = sign * (int)raw[transitions];
    if (duration < -5000 && repetition < 2) {
      repetition += 1;
    }
    _data += String(duration);
    if (received.te == 0 && duration > 0)
      received.te = duration;

    if (!decoded && repetition == 1 && duration >= -5000) {
      int index = find_pulse_index(indexed_durations, duration);
      if (index == -1) {
        indexed_durations.push_back(abs(duration));
        index = indexed_durations.size() - 1;
      }
      durations.push_back(index); // Store indexes for CRC calculation
    }
  }

  received.data = _data;
  received.filepath = "signal_" + String(signals);
  received.frequency = long(frequency * 1000000);

  // if there is a value decoded by RCSwitch, show it
  if (decoded) {
    Serial.println("Sinal RcSwitch capturado");
    blinkLed();
    ++signals;
    received.key = decoded;
    received.preset = String(rcswitch.getReceivedProtocol());
    received.protocol = "RcSwitch";
    received.indexed_durations = {};
    received.te = rcswitch.getReceivedDelay();
    received.Bit = rcswitch.getReceivedBitlength();
    frequency = 0;
    display_info(received, signals, ReadRAW, codesOnly, autoSave, title);
  }
  // if there is no value decoded by RCSwitch, but we calculated a CRC, show it
  else if (repetition >= 2 && !durations.empty()) {
    Serial.println("Sinal Raw capturado");
    blinkLed();
    ++signals;
    received.preset = "0";
    received.protocol = "RAW";
    received.key = crc64_ecma(durations); // Calculate CRC-64
    received.indexed_durations = indexed_durations;
    received.Bit = durations.size();
    frequency = 0;
    display_info(received, signals, ReadRAW, codesOnly, autoSave, title);
  }
  // If there is no decoded value and no CRC calculated, only show the data when
  // specified
  else if (!codesOnly) {
    Serial.println("Dados Raw capturados");
    blinkLed();
    ++signals;
    received.preset = "0";
    received.protocol = "RAW";
    received.key = 0;
    received.indexed_durations = {};
    received.Bit = 0;
    frequency = 0;
    display_info(received, signals, ReadRAW, codesOnly, autoSave, title);
  }

  rcswitch.resetAvailable();
}

void RFScan::select_menu_option() {
#if !defined(T_EMBED_1101) && !defined(CONFIG_IDF_TARGET_ESP32C5)
  rcswitch.disableReceive(); // it is causing T-Embed to restart
#endif

  options = {};

  if (received.protocol != "")
    options.emplace_back("Replay", [this]() { set_option(REPLAY); });
  if (received.data != "" && received.protocol != "RAW")
    options.emplace_back("Replay como RAW",
                         [this]() { set_option(REPLAY_RAW); });

  if (received.protocol != "")
    options.emplace_back("Salvar Sinal", [this]() { set_option(SAVE); });
  if (received.data != "" && received.protocol != "RAW")
    options.emplace_back("Salvar como RAW", [this]() { set_option(SAVE_RAW); });

  if (received.protocol != "")
    options.emplace_back("Resetar Sinal", [this]() { set_option(RESET); });

  if (willyConfigPins.rfModule == CC1101_SPI_MODULE)
    options.emplace_back("Alcance", [this]() { set_option(RANGE); });
  if (willyConfigPins.rfModule == CC1101_SPI_MODULE &&
      !willyConfigPins.rfFxdFreq)
    options.emplace_back("Limiar", [this]() { set_option(THRESHOLD); });

  if (ReadRAW)
    options.emplace_back("Modo = RAW", [&]() {
      ReadRAW = false;
      return select_menu_option();
    });
  else
    options.emplace_back("Modo = Decodif", [&]() {
      ReadRAW = true;
      return select_menu_option();
    });

  if (ReadRAW && codesOnly)
    options.emplace_back("Filtro = Codigo", [&]() {
      codesOnly = false;
      return select_menu_option();
    });
  else if (ReadRAW)
    options.emplace_back("Filtro = Todos", [&]() {
      codesOnly = true;
      return select_menu_option();
    });

  if (autoSave)
    options.emplace_back("Salvar = Auto", [&]() {
      autoSave = false;
      return select_menu_option();
    });
  else
    options.emplace_back("Salvar = Manual", [&]() {
      autoSave = true;
      return select_menu_option();
    });

  options.emplace_back("Fechar Menu", [this]() { set_option(CLOSE_MENU); });
  options.emplace_back("Menu Principal", [this]() { set_option(MAIN_MENU); });

  loopOptions(options);
}

void RFScan::set_option(RFMenuOption option) {
  switch (option) {
  case REPLAY:
  case REPLAY_RAW:
    replay_signal(option == REPLAY_RAW);
    break;

  case SAVE:
  case SAVE_RAW:
    save_signal(option == SAVE_RAW);
    break;

  case RANGE:
    rf_range_selection();
    break; // using a common function to other features
  case RESET:
    reset_signals();
    break;
  case THRESHOLD:
    set_threshold();
    break;

  case CLOSE_MENU:
    break;

  case MAIN_MENU:
    returnToMenu = true;
    return;
  }

  restartScan = true;
  deinitRfModule();
  vTaskDelay(1000 / portTICK_PERIOD_MS);
}

void RFScan::replay_signal(bool asRaw) {
  String actualProtocol = received.protocol;
  if (asRaw) {
    received.protocol = "RAW";
  }
  sendRfCommand(received);
  addToRecentCodes(received);
  received.protocol = actualProtocol;
}

void RFScan::save_signal(bool asRaw) {
  asRaw = asRaw || received.protocol == "RAW";
  Serial.println(asRaw ? "RCSwitch_SaveSignal RAW true"
                       : "RCSwitch_SaveSignal RAW false");
  decimalToHexString(received.key, hexString);
  RCSwitch_SaveSignal(found_freq, received, asRaw, hexString, autoSave);
  lastSavedKey = received.key;
}

void RFScan::reset_signals() {
  received.Bit = 0;
  received.data = "";
  received.key = 0;
  received.preset = "";
  received.protocol = "";
  signals = 0;
}

void RFScan::set_threshold() {
  options = {
      {"(-55) Mais Preciso", [&]() { rssiThreshold = -55; }},
      {"(-60)", [&]() { rssiThreshold = -60; }},
      {"(-65) Padrao ", [&]() { rssiThreshold = -65; }},
      {"(-70)", [&]() { rssiThreshold = -70; }},
      {"(-75)", [&]() { rssiThreshold = -75; }},
      {"(-80) Menos Preciso", [&]() { rssiThreshold = -80; }},
  };
  loopOptions(options);
}
/*
// Using similar function from rf_utils.h
void RFScan::set_range() {
    bool chooseFixedOpt = false;

    options = {
        {String("Fxd [" + String(willyConfigPins.rfFreq) + "]").c_str(),
         [=]() { willyConfigPins.setRfScanRange(willyConfigPins.rfScanRange, 1);
} },
        {"Choose Fxd",                                                   [&]() {
chooseFixedOpt = true; } }, {subghz_frequency_ranges[0], [=]() {
willyConfigPins.setRfScanRange(0); }}, {subghz_frequency_ranges[1], [=]() {
willyConfigPins.setRfScanRange(1); }}, {subghz_frequency_ranges[2], [=]() {
willyConfigPins.setRfScanRange(2); }}, {subghz_frequency_ranges[3], [=]() {
willyConfigPins.setRfScanRange(3); }},
    };

    loopOptions(options);

    if (chooseFixedOpt) { // Range
        options.clear();
        int ind = 0;
        int arraySize = sizeof(subghz_frequency_list) /
sizeof(subghz_frequency_list[0]); for (int i = 0; i < arraySize; i++) { String
tmp = String(subghz_frequency_list[i], 2) + "Mhz";
            options.emplace_back(tmp.c_str(), [=]() { willyConfigPins.rfFreq =
subghz_frequency_list[i]; }); if (int(frequency * 100) ==
int(subghz_frequency_list[i] * 100)) ind = i;
        }
        loopOptions(options, ind);
        options.clear();
        willyConfigPins.setRfScanRange(willyConfigPins.rfScanRange, 1);
    }

    if (willyConfigPins.rfFxdFreq) displayTextLine("Scan freq set to " +
String(willyConfigPins.rfFreq)); else displayTextLine("Range set to " +
String(subghz_frequency_ranges[willyConfigPins.rfScanRange]));
}
*/
void display_info(RfCodes received, int signals, bool ReadRAW, bool codesOnly,
                  bool autoSave, String title) {
  if (title != "")
    drawMainBorderWithTitle(title);
  else
    drawMainBorder();

  if (received.protocol != "")
    display_signal_data(received);

  tft.setTextColor(getColorVariation(willyConfig.priColor),
                   willyConfig.bgColor);

  if (!ReadRAW)
    padprintln("Gravando: Apenas codigos RCSwitch.");
  else if (codesOnly)
    padprintln("Gravando: RAW com CRC ou RCSwitch.");
  else
    padprintln("Gravando: Qualquer sinal RAW.");

  if (autoSave)
    padprintln("Auto salvar: Ativado");

  if (willyConfigPins.rfFxdFreq)
    padprintln("Escaneando: " + String(willyConfigPins.rfFreq) + " MHz");
  else
    padprintln("Escaneando: " +
               String(subghz_frequency_ranges[willyConfigPins.rfScanRange]));

  padprintln("Total sinais enc.: " + String(signals));

  tft.setTextColor(willyConfig.priColor, willyConfig.bgColor);

  padprintln("");
  padprintln("Pressione [NEXT] para opcoes.");
}

void display_signal_data(RfCodes received) {
  std::string txt = received.data.c_str();
  std::stringstream ss(txt);
  std::string palavra;
  int transitions = 0;
  char hexString[64];

  while (ss >> palavra)
    transitions++;

  if (received.preset != "")
    padprintln("Protocolo: " + String(received.protocol) + "(" +
               received.preset + ")");
  else
    padprintln("Protocolo: " + String(received.protocol));

  if (received.key > 0) {
    decimalToHexString(received.key, hexString);
    if (received.protocol == "RAW") {
      padprintln("Comprim.: " + String(received.Bit) + " transitions");
      // tft.setCursor(tft.getCursorX(), tft.getCursorY() + 2);
      padprintln("Comp. gravacao: " + String(transitions) + " transitions");
    } else {
      padprintln("Comprim.: " + String(received.Bit) + " bits");
      const char *b = dec2binWzerofill(received.key, min(received.Bit, 40));
      // tft.setCursor(tft.getCursorX(), tft.getCursorY() + 2);
      padprintln("Binario: " + String(b));
    }
  } else {
    strcpy(hexString, "Codigo nao identif.");
    padprintln("Comprim.: Codigo nao identif.");
    padprintln("Comp. gravacao: " + String(transitions) + " transitions");
  }

  if (received.protocol == "RAW")
    padprintln("CRC: " + String(hexString));
  else
    padprintln("Key: " + String(hexString));

  // if (willyConfigPins.rfModule == CC1101_SPI_MODULE) {
  //     int rssi = ELECHOUSE_cc1101.getRssi();
  //     tft.drawPixel(0, 0, 0);
  //     padprintln("Rssi: " + String(rssi));
  // }

  // if (!received.indexed_durations.empty()) {
  //     padprint("PulseLenghts: ");
  //     for (int i = 0; i < received.indexed_durations.size(); i++) {
  //         if (i < received.indexed_durations.size() - 1)
  //             tft.print(String(received.indexed_durations[i]) + "us, ");
  //         else tft.println(String(received.indexed_durations[i]) + "us");
  //     }
  // } else if (received.te) padprintln("PulseLenght: " + String(received.te) +
  // "us"); else padprintln("PulseLenght: unknown");

  // padprintln("Frequency: " + String(received.frequency) + " Hz");
  padprintln("");
}

bool RCSwitch_SaveSignal(float frequency, RfCodes codes, bool raw, char *key,
                         bool autoSave) {
  FS *fs;
  String filename = "";

  if (!getFsStorage(fs)) {
    displayError("No space left on device", true);
    return false;
  }

  if (!codes.key && codes.data == "") {
    Serial.println("Empty signal, it was not saved.");
    return false;
  }

  String subfile_out = "Filetype: Willy SubGhz File\nVersion 1\n";
  subfile_out += "Frequency: " + String(int(frequency * 1000000)) + "\n";
  if (!raw) {
    subfile_out += "Preset: " + String(codes.preset) + "\n";
    subfile_out += "Protocol: RcSwitch\n";
    subfile_out += "Bit: " + String(codes.Bit) + "\n";
    subfile_out += "Key: " + String(key) + "\n";
    subfile_out += "TE: " + String(codes.te) + "\n";
    filename = "rcs.sub";
    // subfile_out += "RAW_Data: " + codes.data;
  } else {
    // save as raw
    if (codes.preset == "1") {
      codes.preset = "FuriHalSubGhzPresetOok270Async";
    } else if (codes.preset == "2") {
      codes.preset = "FuriHalSubGhzPresetOok650Async";
    }

    subfile_out += "Preset: " + String(codes.preset) + "\n";
    subfile_out += "Protocol: RAW\n";
    subfile_out += "RAW_Data: " + codes.data;
    filename = "raw.sub";
  }

  String filepath = "/WillyRF";
  if (autoSave)
    filepath += "/autoSaved";
  File file = createNewFile(fs, filepath, filename);

  if (file) {
    file.println(subfile_out);
    if (!autoSave)
      displaySuccess(file.path());
  } else {
    displayError("Erro ao salvar arquivo", true);
  }

  file.close();
  return true;
}

String rf_scan(float start_freq, float stop_freq, int max_loops) {
  // derived from
  // https://github.com/mcore1976/cc1101-tool/blob/main/cc1101-tool-esp32.ino#L480

  if (willyConfigPins.rfModule != CC1101_SPI_MODULE) {
    displayError("rf scanning is available with CC1101 only", true);
    return ""; // only CC1101 is supported for this
  }
  if (!initRfModule("rx", start_freq))
    return "";

  ELECHOUSE_cc1101.setRxBW(256);

  float settingf1 = start_freq;
  float settingf2 = stop_freq;
  float freq = 0;
  long compare_freq = 0;
  float mark_freq;
  int rssi;
  int mark_rssi = -100;
  String out = "";

  while (max_loops || !check(EscPress)) {
    vTaskDelay(1 / portTICK_PERIOD_MS);
    max_loops -= 1;

    setMHZ(freq);

    rssi = ELECHOUSE_cc1101.getRssi();
    if (rssi > -75) {
      if (rssi > mark_rssi) {
        mark_rssi = rssi;
        mark_freq = freq;
      };
    };

    freq += 0.01;

    if (freq > settingf2) {
      freq = settingf1;

      if (mark_rssi > -75) {
        long fr = mark_freq * 100;
        if (fr == compare_freq) {
          Serial.print(F("\r\nSignal found at  "));
          Serial.print(F("Freq: "));
          Serial.print(mark_freq);
          Serial.print(F(" Rssi: "));
          Serial.println(mark_rssi);
          mark_rssi = -100;
          compare_freq = 0;
          mark_freq = 0;
          out += String(mark_freq) + ",";
        } else {
          compare_freq = mark_freq * 100;
          freq = mark_freq - 0.10;
          mark_freq = 0;
          mark_rssi = -100;
        };
      };
    }; // end of IF freq>stop frequency
  }; // End of While

  deinitRfModule();
  return out;
}

String RCSwitch_Read(float frequency, int max_loops, bool raw) {
  RCSwitch rcswitch = RCSwitch();
  RfCodes received;

  if (!frequency)
    frequency = willyConfigPins.rfFreq; // default from config

  char hexString[64];

RestartRec:
  drawMainBorder();
  tft.setCursor(10, 28);
  tft.setTextSize(FP);
  tft.println("Aguardando um " + String(frequency) + " MHz " + "sinal.");

  // init receive
  if (!initRfModule("rx", frequency))
    return "";
  if (willyConfigPins.rfModule == CC1101_SPI_MODULE) { // CC1101 in use
    rcswitch.enableReceive(willyConfigPins.CC1101_bus.io0);
    Serial.println("CC1101 enableReceive()");

  } else {
    rcswitch.enableReceive(willyConfigPins.rfRx);
  }
  while (!check(EscPress)) {
    if (rcswitch.available()) {
      // Serial.println("Available");
      long value = rcswitch.getReceivedValue();
      // Serial.println("getReceivedValue()");
      if (value) {
        // Serial.println("has value");
        unsigned int *_raw = rcswitch.getReceivedRawdata();
        received.frequency = long(frequency * 1000000);
        received.key = rcswitch.getReceivedValue();
        received.protocol = "RcSwitch";
        received.preset = rcswitch.getReceivedProtocol();
        received.te = rcswitch.getReceivedDelay();
        received.Bit = rcswitch.getReceivedBitlength();
        received.filepath = "unsaved";
        // Serial.println(received.te*2);
        //  derived from
        //  https://github.com/sui77/rc-switch/tree/master/examples/ReceiveDemo_Advanced
        received.data = "";
        int sign = +1;
        // if(received.preset.invertedSignal) sign = -1;
        for (int i = 0; i < received.Bit * 2; i++) {
          if (i > 0)
            received.data += " ";
          if (i % 2 == 0)
            sign = +1;
          else
            sign = -1;
          received.data += String(sign * (int)_raw[i]);
        }
        // Serial.println(received.protocol);
        // Serial.println(received.data);
        decimalToHexString(received.key, hexString);

        display_info(received, 1, raw);
      }
      rcswitch.resetAvailable();
    }
    if (raw && rcswitch.RAWavailable()) {
      // if no value were decoded, show raw data to be saved
      vTaskDelay(
          100 /
          portTICK_PERIOD_MS); // give it time to process and store all signal

      unsigned int *_raw = rcswitch.getRAWReceivedRawdata();
      int transitions = 0;
      signed int sign = 1;
      for (transitions = 0; transitions < RCSWITCH_RAW_MAX_CHANGES;
           transitions++) {
        if (_raw[transitions] == 0)
          break;
        if (transitions > 0)
          received.data += " ";
        if (transitions % 2 == 0)
          sign = +1;
        else
          sign = -1;
        received.data += String(sign * (int)_raw[transitions]);
      }
      if (transitions > 20) {
        received.frequency = long(frequency * 1000000);
        received.protocol = "RAW";
        received.preset = "0"; // ????
        received.filepath = "unsaved";
        received.data = "";

        display_info(received, 1, raw);
      }
      // ResetSignal:
      rcswitch.resetAvailable();
    }

    if (received.key > 0 ||
        received.data.length() >
            20) { // RAW data does not have "key", 20 is more than 5 transitions
      // switch to raw mode if decoding failed
      if (received.preset == 0) {
        Serial.println("signal decoding failed, switching to RAW mode");
        displayWarning("Decoding failed. Saving as RAW.", true);
        raw = true;
      }
      String subfile_out = "Filetype: Willy SubGhz File\nVersion 1\n";
      subfile_out += "Frequency: " + String(int(frequency * 1000000)) + "\n";
      if (!raw) {
        subfile_out += "Preset: " + String(received.preset) + "\n";
        subfile_out += "Protocol: RcSwitch\n";
        subfile_out += "Bit: " + String(received.Bit) + "\n";
        subfile_out += "Key: " + String(hexString) + "\n";
        subfile_out += "TE: " + String(received.te) + "\n";
      } else {
        // save as raw
        if (received.preset == "1")
          received.preset = "FuriHalSubGhzPresetOok270Async";
        else if (received.preset == "2")
          received.preset = "FuriHalSubGhzPresetOok650Async";
        subfile_out += "Preset: " + String(received.preset) + "\n";
        subfile_out += "Protocol: RAW\n";
        subfile_out += "RAW_Data: " + received.data;
      }
      // headless mode
      return subfile_out;

      if (check(SelPress)) {
        int chosen = 0;
        options = {
            {"Replay signal", [&]() { chosen = 1; }},
            {"Save signal", [&]() { chosen = 2; }},
        };
        loopOptions(options);
        if (chosen == 1) {
          rcswitch.disableReceive();
          sendRfCommand(received);
          addToRecentCodes(received);
          goto RestartRec;
        } else if (chosen == 2) {
          decimalToHexString(received.key, hexString);
          RCSwitch_SaveSignal(frequency, received, raw, hexString);
          vTaskDelay(1000 / portTICK_PERIOD_MS);
          drawMainBorder();
          tft.setCursor(10, 28);
          tft.setTextSize(FP);
          tft.println("Aguardando um " + String(frequency) + " MHz " +
                      "sinal.");
        }
      }
    }
    if (max_loops > 0) {
      // headless mode, quit if nothing received after max_loops
      max_loops -= 1;
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      if (max_loops == 0) {
        Serial.println("timeout");
        return "";
      }
    }
  }
  // Exit:
  vTaskDelay(1 / portTICK_PERIOD_MS);

  deinitRfModule();

  return "";
}
