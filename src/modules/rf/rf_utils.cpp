#include "rf_utils.h"
#include "core/settings.h"
#include <Preferences.h>
#include <iostream>
#include <map>
#include <vector>

// CRC-64-ECMA constants
const uint64_t CRC64_ECMA_POLY =
    0x42F0E1EBA9EA3693; // Polynomial for CRC-64-ECMA
const uint64_t CRC64_ECMA_INIT = 0xFFFFFFFFFFFFFFFF; // Initial value

const int range_limits[4][2] = {
    {0, 23},  // 300-348 MHz
    {24, 47}, // 387-464 MHz
    {48, 56}, // 779-928 MHz
    {0, 56}   // All ranges
};
const char *subghz_frequency_ranges[] = {"300-348 MHz", "387-464 MHz",
                                         "779-928 MHz", "All ranges"};
const float subghz_frequency_list[] PROGMEM = {
    /* 300 - 348 MHz Frequency Range */
    300.000f, 302.757f, 303.875f, 303.900f, 304.250f, 307.000f, 307.500f,
    307.800f, 309.000f, 310.000f, 312.000f, 312.100f, 312.200f, 313.000f,
    313.850f, 314.000f, 314.350f, 314.980f, 315.000f, 318.000f, 330.000f,
    345.000f, 348.000f, 350.000f,

    /* 387 - 464 MHz Frequency Range */
    387.000f, 390.000f, 418.000f, 430.000f, 430.500f, 431.000f, 431.500f,
    433.075f, 433.220f, 433.420f, 433.657f, 433.889f, 433.920f, 434.075f,
    434.177f, 434.190f, 434.390f, 434.420f, 434.620f, 434.775f, 438.900f,
    440.175f, 464.000f, 467.750f,

    /* 779 - 928 MHz Frequency Range */
    779.000f, 868.350f, 868.400f, 868.800f, 868.950f, 906.400f, 915.000f,
    925.000f, 928.000f};

RfCodes recent_rfcodes[16];
int recent_rfcodes_last_used = 0;
bool rf_recent_loaded = false;

void loadRecentRfCodes() {
  if (rf_recent_loaded)
    return;
  Preferences prefs;
  prefs.begin("rf_recent", true);
  recent_rfcodes_last_used = prefs.getInt("last", 0);
  for (int i = 0; i < 16; i++) {
    char base[10];
    sprintf(base, "r%d_", i);
    char key_fp[8];
    sprintf(key_fp, "%sfp", base);
    recent_rfcodes[i].filepath = prefs.getString(key_fp, "");
    if (recent_rfcodes[i].filepath != "") {
      char key_pr[8];
      sprintf(key_pr, "%spr", base);
      recent_rfcodes[i].protocol = prefs.getString(key_pr, "");
      char key_ps[8];
      sprintf(key_ps, "%sps", base);
      recent_rfcodes[i].preset = prefs.getString(key_ps, "");
      char key_dt[8];
      sprintf(key_dt, "%sdt", base);
      recent_rfcodes[i].data = prefs.getString(key_dt, "");
      char key_fq[8];
      sprintf(key_fq, "%sfq", base);
      recent_rfcodes[i].frequency = prefs.getUInt(key_fq, 0);
      char key_ky[8];
      sprintf(key_ky, "%sky", base);
      recent_rfcodes[i].key = prefs.getULong64(key_ky, 0);
      char key_te[8];
      sprintf(key_te, "%ste", base);
      recent_rfcodes[i].te = prefs.getInt(key_te, 0);
      char key_bt[8];
      sprintf(key_bt, "%sbt", base);
      recent_rfcodes[i].Bit = prefs.getInt(key_bt, 0);
      char key_br[8];
      sprintf(key_br, "%sbr", base);
      recent_rfcodes[i].BitRAW = prefs.getInt(key_br, 0);
    }
  }
  prefs.end();
  rf_recent_loaded = true;
}

void saveRecentRfCodes() {
  Preferences prefs;
  prefs.begin("rf_recent", false);
  prefs.putInt("last", recent_rfcodes_last_used);
  for (int i = 0; i < 16; i++) {
    char base[10];
    sprintf(base, "r%d_", i);
    char key_fp[8];
    sprintf(key_fp, "%sfp", base);
    prefs.putString(key_fp, recent_rfcodes[i].filepath);
    if (recent_rfcodes[i].filepath != "") {
      char key_pr[8];
      sprintf(key_pr, "%spr", base);
      prefs.putString(key_pr, recent_rfcodes[i].protocol);
      char key_ps[8];
      sprintf(key_ps, "%sps", base);
      prefs.putString(key_ps, recent_rfcodes[i].preset);
      char key_dt[8];
      sprintf(key_dt, "%sdt", base);
      prefs.putString(key_dt, recent_rfcodes[i].data);
      char key_fq[8];
      sprintf(key_fq, "%sfq", base);
      prefs.putUInt(key_fq, recent_rfcodes[i].frequency);
      char key_ky[8];
      sprintf(key_ky, "%sky", base);
      prefs.putULong64(key_ky, recent_rfcodes[i].key);
      char key_te[8];
      sprintf(key_te, "%ste", base);
      prefs.putInt(key_te, recent_rfcodes[i].te);
      char key_bt[8];
      sprintf(key_bt, "%sbt", base);
      prefs.putInt(key_bt, recent_rfcodes[i].Bit);
      char key_br[8];
      sprintf(key_br, "%sbr", base);
      prefs.putInt(key_br, recent_rfcodes[i].BitRAW);
    }
  }
  prefs.end();
}
bool rmtInstalled = true;
static bool cc1101_spi_ready = false;

bool initRfModule(String mode, float frequency) {
  // use default frequency if no one is passed
  if (!frequency)
    frequency = willyConfigPins.rfFreq;

  if (willyConfigPins.rfModule == CC1101_SPI_MODULE) { // CC1101 in use
    if (willyConfigPins.CC1101_bus.mosi == (gpio_num_t)TFT_MOSI &&
        willyConfigPins.CC1101_bus.mosi !=
            GPIO_NUM_NC) { // (T_EMBED), CORE2 and others
#if TFT_MOSI > 0
      initCC1101once(&tft.getSPIinstance());
#else
      yield();
#endif
    } else if (willyConfigPins.CC1101_bus.mosi ==
               willyConfigPins.SDCARD_bus
                   .mosi) { // (CARDPUTER) and (ESP32S3DEVKITC1) and devices
                            // that share CC1101 pin with only SDCard
      initCC1101once(&sdcardSPI);
    } else if (willyConfigPins.NRF24_bus.mosi ==
                   willyConfigPins.CC1101_bus.mosi &&
               willyConfigPins.CC1101_bus.mosi !=
                   willyConfigPins.SDCARD_bus
                       .mosi) { // This board uses the same Bus for NRF and
                                // CC1101, but with different CS pins, different
                                // from Stick_Cs down below..

      CC_NRF_SPI
          .end(); // Closes in case it was already in use, it will overwrite the
                  // attempt of SD start over to save configurations
      delay(10);
      if (!CC_NRF_SPI.begin(willyConfigPins.CC1101_bus.sck,
                            willyConfigPins.CC1101_bus.miso,
                            willyConfigPins.CC1101_bus.mosi)) {
        Serial.println("Failed to start CC1101 SPI on NRF24 pins!");
      }

      initCC1101once(&CC_NRF_SPI);
    } else {
      // (STICK_C_PLUS) || (STICK_C_PLUS2) and others that doesn´t share SPI
      // with other devices (need to change it when Willy board comes to shore)
      // make sure to use BeginEndLogic for StickCs in the shared pins (not bus)
      // config
      ELECHOUSE_cc1101.setBeginEndLogic(true);
      initCC1101once(NULL);
    }
    if (spiMutex && xSemaphoreTake(spiMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
      ELECHOUSE_cc1101.Init();
      if (ELECHOUSE_cc1101.getCC1101()) { // Check the CC1101 Spi connection.
        Serial.println("cc1101 Connection OK");
      } else {
        displayError("CC1101 not found");
        Serial.println("cc1101 Connection Error");
        xSemaphoreGive(spiMutex);
        return false;
      }

      // make sure it is in idle state when changing frequency and other
      // parameters
      if (!((frequency >= 280 && frequency <= 350) ||
            (frequency >= 387 && frequency <= 468) ||
            (frequency >= 779 && frequency <= 928))) {
        Serial.println("Invalid Frequency, setting default");
        frequency = 433.92;
        displayWarning("Wrong freq, set to 433.92", true);
      }

      // Otimizado: ajustar parâmetros baseado na frequência para melhor
      // processamento
      if (frequency >= 300 && frequency <= 500) {
        ELECHOUSE_cc1101.setRxBW(162); // Otimizado para 300-500MHz
        ELECHOUSE_cc1101.setDRate(
            100); // Taxa de dados mais alta para bandas baixas
      } else if (frequency >= 779 && frequency <= 928) {
        ELECHOUSE_cc1101.setRxBW(325); // Banda mais larga para 779-928MHz
        ELECHOUSE_cc1101.setDRate(50); // Taxa padrão
      } else {
        ELECHOUSE_cc1101.setRxBW(256); // Padrão
        ELECHOUSE_cc1101.setDRate(50);
      }
      ELECHOUSE_cc1101.setClb(1, 13, 15); // Calibration Offset
      ELECHOUSE_cc1101.setClb(2, 16, 19); // Calibration Offset
      ELECHOUSE_cc1101.setModulation(2);
      ELECHOUSE_cc1101.setPktFormat(3);
      setMHZ(frequency);
      Serial.println("cc1101 setMHZ(frequency);");

      if (mode == "tx") {
        ioExpander.turnPinOnOff(IO_EXP_CC_RX, LOW);
        ioExpander.turnPinOnOff(IO_EXP_CC_TX, HIGH);
        pinMode(willyConfigPins.CC1101_bus.io0, OUTPUT);
        ELECHOUSE_cc1101.setPA(12);
        ELECHOUSE_cc1101.SetTx();
      } else if (mode == "rx") {
        ioExpander.turnPinOnOff(IO_EXP_CC_RX, HIGH);
        ioExpander.turnPinOnOff(IO_EXP_CC_TX, LOW);
        pinMode(willyConfigPins.CC1101_bus.io0, INPUT);
        ELECHOUSE_cc1101.SetRx();
      }
      xSemaphoreGive(spiMutex);
    } else if (spiMutex == NULL) {
      // Fallback for non-RTOS or before mutex init if necessary, though
      // unlikely
      return false;
    }
    cc1101_spi_ready = true;
  } else {
    // single-pinned module
    if (abs(frequency - willyConfigPins.rfFreq) > 1) {
      Serial.print("warn: unsupported frequency, trying anyway...");
      // return false;
    }

    if (mode == "tx") {
      gsetRfTxPin(false);
      if (willyConfigPins.SDCARD_bus.checkConflict(willyConfigPins.rfTx))
        sdcardSPI.end();
      gpio_reset_pin((gpio_num_t)willyConfigPins.rfTx);
      pinMode(willyConfigPins.rfTx, OUTPUT);
      digitalWrite(willyConfigPins.rfTx, LOW);

    } else if (mode == "rx") {
      // Rx Mode
      gsetRfRxPin(false);
      if (willyConfigPins.SDCARD_bus.checkConflict(willyConfigPins.rfRx))
        sdcardSPI.end();
      gpio_reset_pin((gpio_num_t)willyConfigPins.rfRx);
      pinMode(willyConfigPins.rfRx, INPUT);
    }
  }
  // no error
  return true;
}

void deinitRfModule() {
  if (willyConfigPins.rfModule == CC1101_SPI_MODULE) {
    if (cc1101_spi_ready) {
      if (spiMutex && xSemaphoreTake(spiMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        ELECHOUSE_cc1101.setSidle();
        xSemaphoreGive(spiMutex);
      }
      cc1101_spi_ready = false;
    }
    digitalWrite(willyConfigPins.CC1101_bus.io0, LOW);
    digitalWrite(willyConfigPins.CC1101_bus.cs, HIGH);
    ioExpander.turnPinOnOff(IO_EXP_CC_RX, LOW);
    ioExpander.turnPinOnOff(IO_EXP_CC_TX, LOW);
  } else
    digitalWrite(willyConfigPins.rfTx, LED_OFF);
}

void initCC1101once(SPIClass *SSPI) {
  // the init (); command may only be executed once in the entire program
  // sequence. Otherwise problems can arise.
  // https://github.com/LSatan/SmartRC-CC1101-Driver-Lib/issues/65

  // derived from
  // https://github.com/LSatan/SmartRC-CC1101-Driver-Lib/blob/master/examples/Rc-Switch%20examples%20cc1101/ReceiveDemo_Advanced_cc1101/ReceiveDemo_Advanced_cc1101.ino
  if (SSPI != NULL)
    ELECHOUSE_cc1101.setSPIinstance(
        SSPI); // New, to use the SPI instance we want.
  else
    ELECHOUSE_cc1101.setSPIinstance(nullptr);
  ELECHOUSE_cc1101.setSpiPin(
      willyConfigPins.CC1101_bus.sck, willyConfigPins.CC1101_bus.miso,
      willyConfigPins.CC1101_bus.mosi, willyConfigPins.CC1101_bus.cs);
  ELECHOUSE_cc1101.setGDO0(
      willyConfigPins.CC1101_bus.io0); // use Gdo0 for both Tx and Rx

  return;
}

void setMHZ(float frequency) {
  if (frequency > 928 || frequency < 280) {
    frequency = 433.92;
    Serial.println("Frequency out of band");
  }
  if (willyConfigPins.rfModule == CC1101_SPI_MODULE) {
#if defined(T_EMBED)
    static uint8_t antenna = 200; // 0=(<300), 1=(350-468), 2=(>778), 200=start
                                  // to settle at the fisrt time
    bool change = true;
#if !defined(T_EMBED_1101)
    // there's one version of T-Embed (White whith orange wheel) that has CC1101
    // which antenna has the same circuit as the new CC1101 version with
    // different pinouts this device uses 17 for CS
    if (willyConfigPins.CC1101_bus.cs != 17)
      change = false;
#endif

    // SW1:1  SW0:0 --- 315MHz
    // SW1:0  SW0:1 --- 868/915MHz
    // SW1:1  SW0:1 --- 434MHz
    if (frequency <= 350 && antenna != 0 && change) {
      digitalWrite(CC1101_SW1_PIN, HIGH);
      digitalWrite(CC1101_SW0_PIN, LOW);
      antenna = 0;
      vTaskDelay(10 / portTICK_PERIOD_MS); // time to settle the antenna signal
    } else if (frequency > 350 && frequency < 468 && antenna != 1 && change) {
      digitalWrite(CC1101_SW1_PIN, HIGH);
      digitalWrite(CC1101_SW0_PIN, HIGH);
      antenna = 1;
      vTaskDelay(10 / portTICK_PERIOD_MS); // time to settle the antenna signal
    } else if (frequency > 778 && antenna != 2 && change) {
      digitalWrite(CC1101_SW1_PIN, LOW);
      digitalWrite(CC1101_SW0_PIN, HIGH);
      antenna = 2;
      vTaskDelay(10 / portTICK_PERIOD_MS); // time to settle the antenna signal
    }
#endif
    // Note: setMHZ is usually called from within another mutex-protected block
    // in rf_utils but adding local protection here just in case it's called
    // independently
    bool hasLock = false;
    if (spiMutex && xSemaphoreTake(spiMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
      hasLock = true;
    }
    ELECHOUSE_cc1101.setMHZ(frequency);
    if (hasLock)
      xSemaphoreGive(spiMutex);
  }
}

int find_pulse_index(const std::vector<int> &indexed_durations, int duration) {
  int abs_duration = abs(duration);
  int closest_index = -1;
  int closest_diff = 999999; // Large number to find minimum difference

  for (size_t i = 0; i < indexed_durations.size(); i++) {
    int diff = abs(indexed_durations[i] - abs_duration);
    if (diff <= 50) { // ±50µs tolerance
      return i;       // Found a close match, return its index
    }
    if (diff < closest_diff) {
      closest_diff = diff;
      closest_index = i; // Store closest match
    }
  }

  // If there's space for a new duration, return -1 to signal adding it
  if (indexed_durations.size() < 4) {
    return -1;
  }

  return closest_index; // Otherwise, return the closest match
}

// Function to compute CRC-64-ECMA
uint64_t crc64_ecma(const std::vector<int> &data) {
  uint64_t crc = CRC64_ECMA_INIT;

  for (int value : data) {
    crc ^= (uint64_t)value << 56; // Use the value as the high byte
    for (int i = 0; i < 8; i++) {
      if (crc & 0x8000000000000000) {
        crc = (crc << 1) ^ CRC64_ECMA_POLY;
      } else {
        crc <<= 1;
      }
    }
  }

  return crc;
}

void addToRecentCodes(struct RfCodes rfcode) {
  if (!rf_recent_loaded)
    loadRecentRfCodes();
  recent_rfcodes[recent_rfcodes_last_used] = rfcode;
  recent_rfcodes_last_used += 1;
  if (recent_rfcodes_last_used == 16)
    recent_rfcodes_last_used = 0; // cycle
  saveRecentRfCodes();
}

struct RfCodes selectRecentRfMenu() {
  if (!rf_recent_loaded)
    loadRecentRfCodes();
  options = {};
  bool exit = false;
  struct RfCodes selected_code;

  for (int i = 0; i < 16; i++) {
    if (recent_rfcodes[i].filepath == "")
      continue; // not inited

    options.emplace_back(
        recent_rfcodes[i].filepath.c_str(),
        [i, &selected_code]() { selected_code = recent_rfcodes[i]; });
  }
  options.emplace_back("Menu Principal", [&]() { exit = true; });

  loopOptions(options);
  options.clear();

  return selected_code;
}
rmt_channel_handle_t setup_rf_rx() {
  if (!initRfModule("rx", willyConfigPins.rfFreq))
    return NULL;
  setMHZ(willyConfigPins.rfFreq);
  rmt_rx_channel_config_t rx_channel_cfg = {};
  rx_channel_cfg.gpio_num =
      willyConfigPins.rfModule == CC1101_SPI_MODULE
          ? gpio_num_t(willyConfigPins.CC1101_bus.io0)
          : gpio_num_t(willyConfigPins.rfRx);   // GPIO number
  rx_channel_cfg.clk_src = RMT_CLK_SRC_DEFAULT; // select source clock
  rx_channel_cfg.resolution_hz =
      1 * 1000 * 1000; // 1 MHz tick resolution, i.e., 1 tick = 1 µs
  rx_channel_cfg.mem_block_symbols =
      64;                           // memory block size, 64 * 4 = 256 Bytes
  rx_channel_cfg.intr_priority = 0; // interrupt priority
  rx_channel_cfg.flags.invert_in = false; // do not invert input signal
  rx_channel_cfg.flags.with_dma = false;  // do not need DMA backend
  rx_channel_cfg.flags.allow_pd =
      false; // do not allow power domain to be powered off in sleep mode
  rx_channel_cfg.flags.io_loop_back = false; // do not loop back output to input

  rmt_channel_handle_t rx_channel = NULL;
  ESP_ERROR_CHECK(rmt_new_rx_channel(&rx_channel_cfg, &rx_channel));
  return rx_channel;
}

bool setMHZMenu() {
  if (willyConfigPins.rfModule != CC1101_SPI_MODULE)
    return false;
  if (check(SelPress)) {
    options = {};
    int ind = 0;
    int arraySize =
        sizeof(subghz_frequency_list) / sizeof(subghz_frequency_list[0]);
    for (int i = 0; i < arraySize; i++) {
      if (subghz_frequency_list[i] - willyConfigPins.rfFreq < 0.1)
        ind = i;
      char tmp[20];
      sprintf(tmp, "%.2fMhz", subghz_frequency_list[i]);
      options.push_back(
          {tmp, [=]() { willyConfigPins.rfFreq = subghz_frequency_list[i]; }});
    }
    loopOptions(options, ind);
    options.clear();
    setMHZ(willyConfigPins.rfFreq);
    return true;
  }
  return false;
}

void rf_range_selection(float currentFrequency) {
  int option = 0;
  char fixed_buf[50];
  sprintf(fixed_buf, "Fixed [%.2f]", willyConfigPins.rfFreq);
  options = {
      {fixed_buf,
       [=]() { willyConfigPins.setRfFreq(willyConfigPins.rfFreq, 2); }},
      {"Escolher Fixa", [&]() { option = 1; }},
      {subghz_frequency_ranges[0],
       [=]() { willyConfigPins.setRfScanRange(0); }},
      {subghz_frequency_ranges[1],
       [=]() { willyConfigPins.setRfScanRange(1); }},
      {subghz_frequency_ranges[2],
       [=]() { willyConfigPins.setRfScanRange(2); }},
      {subghz_frequency_ranges[3],
       [=]() { willyConfigPins.setRfScanRange(3); }},
  };

  loopOptions(options);
  options.clear();

  if (option == 1) { // Fixed Frequency Selector
    options = {};
    int ind = 0;
    int arraySize =
        sizeof(subghz_frequency_list) / sizeof(subghz_frequency_list[0]);
    for (int i = 0; i < arraySize; i++) {
      char tmp[20];
      sprintf(tmp, "%.2fMhz", subghz_frequency_list[i]);
      options.push_back(
          {tmp,
           [=]() { willyConfigPins.setRfFreq(subghz_frequency_list[i], 2); }});
      if (int(currentFrequency * 100) == int(subghz_frequency_list[i] * 100))
        ind = i;
    }
    loopOptions(options, ind);
    options.clear();
  }

  char buf[100];
  if (willyConfigPins.rfFxdFreq) {
    sprintf(buf, "Freq. de Scan definida para %.2f", willyConfigPins.rfFreq);
    displayTextLine(buf);
  } else {
    sprintf(buf, "Faixa definida para %s",
            subghz_frequency_ranges[willyConfigPins.rfScanRange]);
    displayTextLine(buf);
  }
}

// ========== OTIMIZAÇÕES ADICIONADAS ==========

// Função auxiliar para validação de frequência RF
bool validateRfFrequency(float frequency) {
  // Faixas permitidas: 300-348MHz, 387-464MHz, 779-928MHz
  return ((frequency >= 300.0f && frequency <= 348.0f) ||
          (frequency >= 387.0f && frequency <= 464.0f) ||
          (frequency >= 779.0f && frequency <= 928.0f));
}

// Função auxiliar para configuração de parâmetros RF otimizados
void configureRfParameters(float frequency) {
  if (frequency >= 300.0f && frequency <= 500.0f) {
    ELECHOUSE_cc1101.setRxBW(162); // Otimizado para 300-500MHz
    ELECHOUSE_cc1101.setDRate(
        100); // Taxa de dados mais alta para bandas baixas
  } else if (frequency >= 779.0f && frequency <= 928.0f) {
    ELECHOUSE_cc1101.setRxBW(325); // Banda mais larga para 779-928MHz
    ELECHOUSE_cc1101.setDRate(50); // Taxa padrão
  } else {
    ELECHOUSE_cc1101.setRxBW(256); // Padrão
    ELECHOUSE_cc1101.setDRate(50);
  }
}

// Função auxiliar para configuração de modo RF
bool configureRfMode(String mode) {
  if (mode == "tx") {
    ioExpander.turnPinOnOff(IO_EXP_CC_RX, LOW);
    ioExpander.turnPinOnOff(IO_EXP_CC_TX, HIGH);
    pinMode(willyConfigPins.CC1101_bus.io0, OUTPUT);
    ELECHOUSE_cc1101.setPA(12);
    ELECHOUSE_cc1101.SetTx();
  } else if (mode == "rx") {
    ioExpander.turnPinOnOff(IO_EXP_CC_RX, HIGH);
    ioExpander.turnPinOnOff(IO_EXP_CC_TX, LOW);
    pinMode(willyConfigPins.CC1101_bus.io0, INPUT);
    ELECHOUSE_cc1101.SetRx();
  } else {
    Serial.println("Modo RF inválido");
    return false;
  }
  return true;
}

// Função auxiliar para configuração de modo single-pin
bool configureSinglePinMode(String mode) {
  if (mode == "tx") {
    gsetRfTxPin(false);
    if (willyConfigPins.SDCARD_bus.checkConflict(willyConfigPins.rfTx))
      sdcardSPI.end();
    gpio_reset_pin((gpio_num_t)willyConfigPins.rfTx);
    pinMode(willyConfigPins.rfTx, OUTPUT);
    digitalWrite(willyConfigPins.rfTx, LOW);
  } else if (mode == "rx") {
    gsetRfRxPin(false);
    if (willyConfigPins.SDCARD_bus.checkConflict(willyConfigPins.rfRx))
      sdcardSPI.end();
    gpio_reset_pin((gpio_num_t)willyConfigPins.rfRx);
    pinMode(willyConfigPins.rfRx, INPUT);
  } else {
    Serial.println("Modo single-pin inválido");
    return false;
  }
  return true;
}
