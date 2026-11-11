#include <SPI.h>
#include <algorithm>
#include <globals.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <vector>

#include "display.h"
#include "modules/badusb_ble/ducky_typer.h"
#include "modules/bjs_interpreter/interpreter.h"
#include "modules/gps/wigle.h"
#include "modules/ir/custom_ir.h"
#include "modules/others/qrcode_menu.h"
#include "modules/rf/rf_send.h"
#include "mykeyboard.h"
#include "passwords.h"
#include "scrollableTextArea.h"
#include "sd_functions.h"
#include <MD5Builder.h>
#include <algorithm> // for std::sort
#include <driver/gpio.h>
#include <esp_rom_crc.h>

// SPIClass sdcardSPI;
String fileToCopy;
std::vector<FileList> fileList;

// Pino de detecção de cartão (CD) - pode ser configurado via definição
#ifndef SDCARD_CD_PIN
#define SDCARD_CD_PIN -1
#endif

/***************************************************************************************
** Function name: sdCardPresent
** Description:   Verifica se o cartão está fisicamente presente usando pino CD
***************************************************************************************/
bool sdCardPresent() {
#if SDCARD_CD_PIN >= 0
  // Se temos pino de detecção configurado, usamos ele
  // Cartão presente quando pino está em LOW (comum) ou HIGH (depende do
  // adaptador)
  pinMode(SDCARD_CD_PIN, INPUT_PULLUP);
  bool present = digitalRead(SDCARD_CD_PIN) == LOW; // Assumindo active LOW
  Serial.printf("[SD] Detecção de cartão (pino %d): %s\n", SDCARD_CD_PIN,
                present ? "PRESENTE" : "AUSENTE");
  return present;
#else
  // Sem pino de detecção, assumimos presente para tentar montar
  Serial.printf(
      "[SD] Sem pino de detecção CD, SDCARD_CD_PIN=%d, tentando montar\n",
      SDCARD_CD_PIN);
  return true;
#endif
}

/***************************************************************************************
** Function name: tryRecoverSdCard
** Description:   Tenta recuperar cartão com problemas de montagem
***************************************************************************************/
bool tryRecoverSdCard() {
  Serial.println("[SD] Tentando recuperar cartão SD...");

  // 1. Verifica se cartão está presente
  if (!sdCardPresent()) {
    Serial.println("[SD] Cartão não detectado fisicamente");
    return false;
  }

  // 2. Tenta reinicializar o SPI
  Serial.println("[SD] Reinicializando SPI...");
  if (willyConfigPins.SDCARD_bus.mosi != (gpio_num_t)TFT_MOSI ||
      willyConfigPins.SDCARD_bus.mosi == GPIO_NUM_NC) {
    sdcardSPI.end();
    delay(100);
    sdcardSPI.begin((int8_t)willyConfigPins.SDCARD_bus.sck,
                    (int8_t)willyConfigPins.SDCARD_bus.miso,
                    (int8_t)willyConfigPins.SDCARD_bus.mosi,
                    (int8_t)willyConfigPins.SDCARD_bus.cs);
    delay(100);
  }

  // 3. Tenta montar novamente com frequência baixa
  Serial.println("[SD] Tentando montar com 1MHz...");
  delay(100);
  SPI.endTransaction(); // Ensure SPI is free
  if (SD.begin((int8_t)willyConfigPins.SDCARD_bus.cs,
               willyConfigPins.SDCARD_bus.mosi == (gpio_num_t)TFT_MOSI
                   ? tft.getSPIinstance()
                   : sdcardSPI,
               1000000)) {
    Serial.println("[SD] Recuperado com sucesso a 1MHz!");
    return true;
  }

  Serial.println("[SD] Não foi possível recuperar o cartão");
  return false;
}

/***************************************************************************************
** Function name: getSdCardInfo
** Description:   Retorna informações detalhadas do cartão SD
***************************************************************************************/
String getSdCardInfo() {
  if (!sdcardMounted) {
    return "Cartão SD não montado";
  }

  String info = "Tipo: ";
  switch (SD.cardType()) {
  case CARD_NONE:
    info += "Nenhum";
    break;
  case CARD_MMC:
    info += "MMC";
    break;
  case CARD_SD:
    info += "SDSC";
    break;
  case CARD_SDHC:
    info += "SDHC";
    break;
  default:
    info += "Desconhecido";
    break;
  }

  uint64_t size = SD.cardSize();
  uint64_t used = SD.usedBytes();
  uint64_t free = size - used;

  info += "\nTamanho: " + String(size / 1073741824.0, 2) + " GB";
  info += "\nUsado: " + String(used / 1073741824.0, 2) + " GB";
  info += "\nLivre: " + String(free / 1073741824.0, 2) + " GB";

  return info;
}

/***************************************************************************************
** Function name: diagnoseSdCard
** Description:   Diagnóstico completo do cartão SD
***************************************************************************************/
void diagnoseSdCard() {
  Serial.println("\n=== DIAGNÓSTICO DO CARTÃO SD ===");

  // 1. Verifica pinos configurados
  Serial.printf(
      "Pinos: SCK=%d, MISO=%d, MOSI=%d, CS=%d\n",
      (int)willyConfigPins.SDCARD_bus.sck, (int)willyConfigPins.SDCARD_bus.miso,
      (int)willyConfigPins.SDCARD_bus.mosi, (int)willyConfigPins.SDCARD_bus.cs);

  // 2. Verifica conflitos
  bool conflict = false;
  if (willyConfigPins.CC1101_bus.checkConflict(willyConfigPins.SDCARD_bus.cs)) {
    Serial.println("CONFLITO: CS do SD conflita com CC1101!");
    conflict = true;
  }
  if (willyConfigPins.NRF24_bus.checkConflict(willyConfigPins.SDCARD_bus.cs)) {
    Serial.println("CONFLITO: CS do SD conflita com NRF24!");
    conflict = true;
  }
  if (!conflict) {
    Serial.println("Sem conflitos de CS detectados");
  }

  // 3. Verifica detecção física
#if SDCARD_CD_PIN >= 0
  pinMode(SDCARD_CD_PIN, INPUT_PULLUP);
  bool present = digitalRead(SDCARD_CD_PIN) == LOW;
  Serial.printf("Detecção CD (pino %d): %s\n", SDCARD_CD_PIN,
                present ? "PRESENTE" : "AUSENTE");
#else
  Serial.println("Pino CD não configurado");
#endif

  // 4. Tenta montar se ainda não montado
  if (!sdcardMounted) {
    Serial.println("Tentando montar cartão...");
    if (setupSdCard()) {
      Serial.println("Montagem: SUCESSO");
    } else {
      Serial.println("Montagem: FALHOU");
    }
  } else {
    Serial.println("Cartão já montado");
  }

  // 5. Informações do cartão se montado
  if (sdcardMounted) {
    Serial.printf("Tipo: %s\n", SD.cardType() == CARD_NONE   ? "Nenhum"
                                : SD.cardType() == CARD_MMC  ? "MMC"
                                : SD.cardType() == CARD_SD   ? "SDSC"
                                : SD.cardType() == CARD_SDHC ? "SDHC"
                                                             : "Desconhecido");
    Serial.printf("Tamanho: %llu bytes (%.2f GB)\n", SD.cardSize(),
                  SD.cardSize() / 1073741824.0);
    Serial.printf("Livre: %llu bytes (%.2f GB)\n",
                  SD.totalBytes() - SD.usedBytes(),
                  (SD.totalBytes() - SD.usedBytes()) / 1073741824.0);
  }

  Serial.println("=== FIM DO DIAGNÓSTICO ===\n");
}

/***************************************************************************************
** Function name: recoverSdCardInteractive
** Description:   Menu interativo de recuperação do cartão SD
***************************************************************************************/
void recoverSdCardInteractive() {
  if (!sdCardPresent()) {
    displayError("Cartão não detectado fisicamente", true);
    return;
  }

  std::vector<String> options = {"Tentar recuperação automática",
                                 "Ver diagnóstico serial", "Cancelar"};

  size_t selected = 0;
  while (true) {
    resetTftDisplay();
    drawMainBorderWithTitle("Recuperação SD");
    padprintln("");
    padprintln("Cartão detectado mas com erro.");
    padprintln("Escolha uma opção:");
    padprintln("");

    for (size_t i = 0; i < options.size(); i++) {
      if (i == selected) {
        tft.setTextColor(TFT_BLACK, TFT_GREEN);
        padprintln("> " + options[i]);
        tft.setTextColor(willyConfig.priColor, willyConfig.bgColor);
      } else {
        padprintln("  " + options[i]);
      }
    }

    // Verifica entrada do usuário
    if (check(AnyKeyPress)) {
      delay(200);
      if (check(UpPress)) {
        selected = (selected - 1 + options.size()) % options.size();
      } else if (check(DownPress)) {
        selected = (selected + 1) % options.size();
      } else if (check(SelPress)) {
        delay(200);
        switch (selected) {
        case 0: // Recuperação automática
          displayError("Tentando recuperar...", false);
          if (tryRecoverSdCard()) {
            displaySuccess("Cartão recuperado!", true);
          } else {
            displayError("Recuperação falhou", true);
          }
          return;

        case 1: // Diagnóstico
          diagnoseSdCard();
          displaySuccess("Diagnóstico completo no serial", true);
          return;

        case 2: // Cancelar
          return;
        }
      } else if (check(EscPress)) {
        return;
      }
    }
    delay(50);
  }
}

/***************************************************************************************
** Function name: setupSdCard
** Description:   Start SD Card
***************************************************************************************/
bool setupSdCard() {
  Serial.println("[DEBUG] setupSdCard() iniciado");
#ifndef USE_SD_MMC
  // Verifica se os pinos do SD estão configurados
  if (willyConfigPins.SDCARD_bus.sck < 0 ||
      willyConfigPins.SDCARD_bus.miso < 0 ||
      willyConfigPins.SDCARD_bus.mosi < 0 ||
      willyConfigPins.SDCARD_bus.cs < 0) {
    Serial.println("[SD] Pinos do SD não configurados");
    sdcardMounted = false;
    return false;
  }

  // Verifica conflitos de pinos com outros dispositivos SPI
  Serial.printf(
      "[SD] Pinos configurados: SCK=%d, MISO=%d, MOSI=%d, CS=%d\n",
      (int)willyConfigPins.SDCARD_bus.sck, (int)willyConfigPins.SDCARD_bus.miso,
      (int)willyConfigPins.SDCARD_bus.mosi, (int)willyConfigPins.SDCARD_bus.cs);

  // Verifica conflitos
  if (willyConfigPins.CC1101_bus.checkConflict(willyConfigPins.SDCARD_bus.cs) ||
      willyConfigPins.NRF24_bus.checkConflict(willyConfigPins.SDCARD_bus.cs)) {
    Serial.println(
        "[SD] AVISO: Pino CS do SD conflita com outro dispositivo SPI!");
  }

  // Explicitly set CS pin to OUTPUT and HIGH to ensure it's in a known state
  // before starting SPI
  if (willyConfigPins.SDCARD_bus.cs >= 0) {
    pinMode(willyConfigPins.SDCARD_bus.cs, OUTPUT);
    digitalWrite(willyConfigPins.SDCARD_bus.cs, HIGH);
    Serial.printf("[SD] CS pin %d set to HIGH\n",
                  (int)willyConfigPins.SDCARD_bus.cs);
  }

  // SPI CS Safeguard: Ensure shared devices on the same bus have CS HIGH
  if (willyConfigPins.CC1101_bus.cs >= 0) {
    pinMode(willyConfigPins.CC1101_bus.cs, OUTPUT);
    digitalWrite(willyConfigPins.CC1101_bus.cs, HIGH);
    Serial.printf("[SD] CC1101 CS pin %d set to HIGH\n",
                  (int)willyConfigPins.CC1101_bus.cs);
  }
  delay(100); // Delay to ensure CS pins are stable
  if (willyConfigPins.NRF24_bus.cs >= 0) {
    pinMode(willyConfigPins.NRF24_bus.cs, OUTPUT);
    digitalWrite(willyConfigPins.NRF24_bus.cs, HIGH);
    Serial.printf("[SD] NRF24 CS pin %d set to HIGH\n",
                  (int)willyConfigPins.NRF24_bus.cs);
  }
#if !defined(LITE_VERSION)
  if (willyConfigPins.W5500_bus.cs >= 0) {
    // Corrigir pin inválido no ESP32-S3 (GPIO23 não existe)
    if (willyConfigPins.W5500_bus.cs == 23) {
      willyConfigPins.W5500_bus.cs = (gpio_num_t)33;
    }
    pinMode(willyConfigPins.W5500_bus.cs, OUTPUT);
    digitalWrite(willyConfigPins.W5500_bus.cs, HIGH);
    Serial.printf("[SD] W5500 CS pin %d set to HIGH\n",
                  (int)willyConfigPins.W5500_bus.cs);
  }
#endif
#endif

  // avoid unnecessary remounting
  if (sdcardMounted) {
    Serial.println("[SD] SD já montado, retornando true");
    return true;
  }

  // Verifica se cartão está presente antes de tentar montar
  if (!sdCardPresent()) {
    Serial.println(
        "[SD] Cartão SD não detectado fisicamente, pulando montagem");
    sdcardMounted = false;
    return false;
  }

  Serial.println("[SD] Tentando montar cartão SD...");

  bool result = false;
  bool task = false; // devices that doesn't use InputHandler task
#ifdef USE_TFT_eSPI_TOUCH
  task = true;
#endif

#ifdef USE_SD_MMC
  Serial.println("[SD] Usando modo SDMMC");
  if (!SD.begin("/sdcard", true)) {
    Serial.println("[SD] SD.begin SDMMC falhou");
    sdcardMounted = false;
    result = false;
    SPI.endTransaction();
  } else {
    result = true;
  }
#else
  // Not using InputHandler (SdCard on default &SPI bus)
  if (task) {
    // Check if SDCARD shares bus with TFT
    if (willyConfigPins.SDCARD_bus.mosi == (gpio_num_t)TFT_MOSI &&
        willyConfigPins.SDCARD_bus.mosi != GPIO_NUM_NC) {
#if TFT_MOSI > 0
      Serial.println("[SD] Touch device: SD shares TFT SPI bus");
      Serial.println("[SD] Tentando montar com SPI dedicado (mesmos pinos)...");
      Serial.println(
          "[SD] DIAGNÓSTICO: SPI compartilhado pode causar conflitos");
      delay(100); // Delay to allow SPI bus to stabilize
      // Use dedicated SPI instance for better control
      sdcardSPI.begin((int8_t)willyConfigPins.SDCARD_bus.sck,
                      (int8_t)willyConfigPins.SDCARD_bus.miso,
                      (int8_t)willyConfigPins.SDCARD_bus.mosi,
                      (int8_t)willyConfigPins.SDCARD_bus.cs);
      delay(10);
      SPI.endTransaction(); // Ensure SPI is free
      if (!SD.begin(willyConfigPins.SDCARD_bus.cs, sdcardSPI)) {
        Serial.println("[SD] SD.begin (sdcardSPI) falhou, tentando 4MHz...");
        delay(100);
        SPI.endTransaction();
        if (!SD.begin(willyConfigPins.SDCARD_bus.cs, sdcardSPI, 4000000)) {
          Serial.println(
              "[SD] SD.begin (sdcardSPI 4MHz) falhou, tentando 1MHz...");
          delay(100);
          SPI.endTransaction();
          if (!SD.begin(willyConfigPins.SDCARD_bus.cs, sdcardSPI, 1000000)) {
            Serial.println("[SD] SD.begin (TFT SPI 1MHz) falhou");
            Serial.println("[SD] DIAGNÓSTICO: Possível cartão não inserido ou "
                           "wiring ruim");
            SPI.endTransaction();
            result = false;
          } else {
            Serial.println("[SD] SD montado com sucesso a 1MHz via sdcardSPI");
            result = true;
          }
        } else {
          Serial.println("[SD] SD montado com sucesso a 4MHz via sdcardSPI");
          result = true;
        }
      } else {
        Serial.println(
            "[SD] SD montado com sucesso via sdcardSPI (freq padrão)");
        result = true;
      }
#else
      Serial.println("[SD] ERRO: TFT_MOSI não definido, impossível montar");
      result = false;
#endif
    } else {
      // SD on its own bus, use default SPI
      Serial.println("[SD] SD em barramento próprio, usando SPI padrão");
      if (!SD.begin((int8_t)willyConfigPins.SDCARD_bus.cs)) {
        Serial.println("[SD] SD.begin padrão falhou, tentando 4MHz...");
        SPI.endTransaction();
        if (!SD.begin((int8_t)willyConfigPins.SDCARD_bus.cs, SPI, 4000000)) {
          Serial.println("[SD] SD.begin 4MHz falhou, tentando 1MHz...");
          SPI.endTransaction();
          if (!SD.begin((int8_t)willyConfigPins.SDCARD_bus.cs, SPI, 1000000)) {
            Serial.println("[SD] SD.begin 1MHz falhou");
            SPI.endTransaction();
            result = false;
          } else {
            Serial.println("[SD] SD montado com sucesso a 1MHz");
            result = true;
          }
        } else {
          Serial.println("[SD] SD montado com sucesso a 4MHz");
          result = true;
        }
      } else {
        Serial.println("[SD] SD montado com sucesso na frequência padrão");
        result = true;
      }
    }
  }
  // SDCard in the same Bus as TFT, in this case we call the SPI TFT Instance
  else if (willyConfigPins.SDCARD_bus.mosi == (gpio_num_t)TFT_MOSI &&
           willyConfigPins.SDCARD_bus.mosi != GPIO_NUM_NC) {
    Serial.println("[SD] SDCard no mesmo barramento que TFT, tentando SPI "
                   "dedicado primeiro");
#if TFT_MOSI > 0 // condition for Headless and 8bit displays (no SPI bus)
    // CORREÇÃO TEMPORÁRIA: Tentar SPI dedicado para evitar anomalia de MISO
    Serial.println("[SD] Tentando SPI dedicado para SD...");
    sdcardSPI.begin((int8_t)willyConfigPins.SDCARD_bus.sck,
                    (int8_t)willyConfigPins.SDCARD_bus.miso,
                    (int8_t)willyConfigPins.SDCARD_bus.mosi,
                    (int8_t)willyConfigPins.SDCARD_bus.cs);
    delay(10);
    if (!SD.begin((int8_t)willyConfigPins.SDCARD_bus.cs, sdcardSPI)) {
      Serial.println("[SD] SPI dedicado falhou, tentando TFT SPI...");
      if (!SD.begin(willyConfigPins.SDCARD_bus.cs, tft.getSPIinstance())) {
        Serial.println("[SD] SD.begin (TFT SPI) falhou, tentando 4MHz...");
        Serial.println(
            "[SD] DIAGNÓSTICO: Frequência padrão muito alta, tentando reduzir");
        delay(100);
        SPI.endTransaction();
        if (!SD.begin(willyConfigPins.SDCARD_bus.cs, tft.getSPIinstance(),
                      4000000)) {
          Serial.println(
              "[SD] SD.begin (TFT SPI 4MHz) falhou, tentando 1MHz...");
          delay(100);
          SPI.endTransaction();
          if (!SD.begin(willyConfigPins.SDCARD_bus.cs, tft.getSPIinstance(),
                        1000000)) {
            Serial.println("[SD] SD.begin (TFT SPI 1MHz) falhou");
            result = false;
            Serial.println("[SD] SDCard no mesmo barramento que TFT, mas "
                           "falhou ao montar");
          } else {
            Serial.println("[SD] SD montado com sucesso a 1MHz via TFT SPI");
            result = true;
          }
        } else {
          Serial.println("[SD] SD montado com sucesso a 4MHz via TFT SPI");
          result = true;
        }
      } else {
        Serial.println("[SD] SD montado com sucesso via TFT SPI");
        result = true;
      }
    } else {
      Serial.println("[SD] SD montado com sucesso via SPI dedicado!");
      result = true;
    }
#else
    Serial.println("[SD] ERRO: TFT_MOSI não definido, impossível montar");
    result = false;
#endif

  }
  // If not using TFT Bus, use a specific bus
  else {
    Serial.printf(
        "[SD] Iniciando SPI dedicado: SCK=%d, MISO=%d, MOSI=%d, CS=%d\n",
        (int)willyConfigPins.SDCARD_bus.sck,
        (int)willyConfigPins.SDCARD_bus.miso,
        (int)willyConfigPins.SDCARD_bus.mosi,
        (int)willyConfigPins.SDCARD_bus.cs);
    sdcardSPI.begin(
        (int8_t)willyConfigPins.SDCARD_bus.sck,
        (int8_t)willyConfigPins.SDCARD_bus.miso,
        (int8_t)willyConfigPins.SDCARD_bus.mosi,
        (int8_t)willyConfigPins.SDCARD_bus.cs); // start SPI communications
    delay(10);
    Serial.println("[SD] SPI iniciado, tentando SD.begin...");
    if (!SD.begin((int8_t)willyConfigPins.SDCARD_bus.cs, sdcardSPI)) {
      Serial.println("[SD] SD.begin (sdcardSPI) falhou, tentando 4MHz...");
      delay(100);
      SPI.endTransaction();
      if (!SD.begin((int8_t)willyConfigPins.SDCARD_bus.cs, sdcardSPI,
                    4000000)) {
        Serial.println(
            "[SD] SD.begin (sdcardSPI 4MHz) falhou, tentando 1MHz...");
        delay(100);
        SPI.endTransaction();
        if (!SD.begin((int8_t)willyConfigPins.SDCARD_bus.cs, sdcardSPI,
                      1000000)) {
          Serial.println("[SD] SD.begin (sdcardSPI 1MHz) falhou - cartão não "
                         "detectado ou danificado");
          result = false;
#if defined(ARDUINO_M5STICK_C_PLUS) || defined(ARDUINO_M5STICK_C_PLUS2)
          if (willyConfigPins.SDCARD_bus.miso !=
              willyConfigPins.CC1101_bus.miso)
            sdcardSPI.end();
#endif
        } else {
          Serial.println("[SD] SDCARD montado com sucesso a 1MHz");
          result = true;
        }
      } else {
        Serial.println("[SD] SDCARD montado com sucesso a 4MHz");
        result = true;
      }
    } else {
      Serial.println("[SD] SDCARD montado com sucesso na frequência padrão");
      result = true;
    }
    Serial.println(
        "[SD] SDCard em barramento diferente, usando instância sdcardSPI");
  }
#endif

  if (result == false) {
    Serial.println(
        "[SD] ERRO: SDCARD NÃO montado - verifique wiring e formato");
    Serial.println("[SD] Dicas de diagnóstico:");

    // Verifica se cartão está presente
    if (!sdCardPresent()) {
      Serial.println("[SD]   - Cartão não detectado fisicamente (pino CD)");
    } else {
      Serial.println("[SD]   - Cartão detectado mas não montou");
      Serial.println(
          "[SD]   - Possível cartão corrompido ou formato incompatível");
    }

    Serial.println("[SD] Soluções sugeridas:");
    Serial.println(
        "[SD]   1. Verifique se o cartão está inserido corretamente");
    Serial.println("[SD]   2. Teste o cartão em outro dispositivo");
    Serial.println("[SD]   3. Tente formatar o cartão como FAT32");
    Serial.println(
        "[SD]   4. Verifique se os pinos SPI estão corretos e sem conflitos");
    Serial.println("[SD]   5. Tente usar frequência mais baixa (1MHz)");
    Serial.println("[SD]   6. Verifique se o adaptador SD está funcionando");

    sdcardMounted = false;
    return false;
  } else {
    Serial.println("[SD] SUCCESS: SDCARD montado com sucesso!");
    sdcardMounted = true;

    // Verifica informações do cartão
    Serial.printf("[SD] Tipo: %s\n", SD.cardType() == CARD_NONE   ? "Nenhum"
                                     : SD.cardType() == CARD_MMC  ? "MMC"
                                     : SD.cardType() == CARD_SD   ? "SDSC"
                                     : SD.cardType() == CARD_SDHC ? "SDHC"
                                                                  : "SDXC");
    Serial.printf("[SD] Tamanho: %llu bytes (%.2f GB)\n", SD.cardSize(),
                  SD.cardSize() / 1073741824.0);

    // Cria diretórios necessários
    if (spiMutex && xSemaphoreTake(spiMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
      Serial.println("[SD] Criando diretórios...");
      SD.mkdir("/WillyPCAP");
      SD.mkdir("/WillyLogs");
      SD.mkdir("/WillyRFID");
      SD.mkdir("/WillyGPS");
      SD.mkdir("/WPS");
      SD.mkdir("/WillyWebUI");
      xSemaphoreGive(spiMutex);
      Serial.println("[SD] Diretórios criados/verificados");
    }

    Serial.println("[SD] Cartão SD pronto para uso");
    return true;
  }
}

/***************************************************************************************
** Function name: getHierarchicalPath
** Description:   get path in YYYY/MM/DD format
***************************************************************************************/
String getHierarchicalPath(String baseDir) {
  // Basic implementation using current date if available, or just keeping it
  // organized For now, let's just ensure the base directory exists
  if (!sdcardMounted)
    return baseDir;

  if (!SD.exists(baseDir))
    SD.mkdir(baseDir);

  // Future: Add YYYY/MM/DD subfolders here if RTC is available
  return baseDir;
}

/***************************************************************************************
** Function name: closeSdCard
** Description:   Turn Off SDCard, set sdcardMounted state to false
***************************************************************************************/
void closeSdCard() {
  if (spiMutex && xSemaphoreTake(spiMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
    SD.end();
    xSemaphoreGive(spiMutex);
  }
  Serial.println("SD Card Unmounted...");
  sdcardMounted = false;
}

/***************************************************************************************
** Function name: ToggleSDCard
** Description:   Turn Off or On the SDCard, return sdcardMounted state
***************************************************************************************/
bool ToggleSDCard() {
  if (sdcardMounted == true) {
    closeSdCard();
    sdcardMounted = false;
    return false;
  } else {
    sdcardMounted = setupSdCard();
    return sdcardMounted;
  }
}
/***************************************************************************************
** Function name: deleteFromSd
** Description:   delete file or folder
***************************************************************************************/
bool deleteFromSd(fs::FS &fs, String path) {
  if (static_cast<fs::FS *>(&fs) == static_cast<fs::FS *>(&SD) && spiMutex &&
      xSemaphoreTake(spiMutex, pdMS_TO_TICKS(1000)) != pdTRUE) {
    return false;
  }

  File dir = fs.open(path);
  Serial.printf("Deleting: %s\n", path.c_str());
  if (!dir.isDirectory()) {
    dir.close();
    bool res = fs.remove(path.c_str());
    if (static_cast<fs::FS *>(&fs) == static_cast<fs::FS *>(&SD) && spiMutex)
      xSemaphoreGive(spiMutex);
    return res;
  }

  dir.rewindDirectory();
  bool success = true;

  bool isDir;
  String fileName = dir.getNextFileName(&isDir);
  while (fileName != "") {
    if (isDir) {
      success &= deleteFromSd(fs, fileName);
    } else {
      success &= fs.remove(fileName.c_str());
    }
    fileName = dir.getNextFileName(&isDir);
  }

  dir.close();
  // Apaga a própria pasta depois de apagar seu conteúdo
  success &= fs.rmdir(path.c_str());
  if (static_cast<fs::FS *>(&fs) == static_cast<fs::FS *>(&SD) && spiMutex)
    xSemaphoreGive(spiMutex);
  return success;
}

/***************************************************************************************
** Function name: renameFile
** Description:   rename file or folder
***************************************************************************************/
bool renameFile(fs::FS &fs, String path, String filename) {
  String newName = keyboard(filename, 76, "Digite o novo Nome:");
  // Rename the file of folder
  if (fs.rename(path,
                path.substring(0, path.lastIndexOf('/')) + "/" + newName)) {
    // Serial.println("Renamed from " + filename + " to " + newName);
    return true;
  } else {
    // Serial.println("Fail on rename.");
    return false;
  }
}
/***************************************************************************************
** Function name: copyToFs
** Description:   copy file from SD or LittleFS to LittleFS or SD
***************************************************************************************/
bool copyToFs(fs::FS &from, fs::FS &to, String path, bool draw) {
  // Using Global Buffer
  bool result = false;
  if (!sdcardMounted) {
    if (!setupSdCard()) {
      sdcardMounted = false;
      Serial.println("SD Card not mounted");
      return false;
    }
  }

  if (!LittleFS.begin()) {
    Serial.println("LittleFS not mounted");
    return false;
  }

  File source = from.open(path, FILE_READ);
  if (!source) {
    Serial.println("Fail opening Source file");
    return false;
  }
  path = path.substring(path.lastIndexOf('/'));
  if (!path.startsWith("/"))
    path = "/" + path;
  File dest = to.open(path, FILE_WRITE);
  if (!dest) {
    Serial.println("Fail creating destination file");
    return false;
  }
  size_t bytesRead;
  size_t tot = source.size();
  size_t prog = 0;

  if (&to == static_cast<fs::FS *>(&LittleFS) &&
      (LittleFS.totalBytes() - LittleFS.usedBytes()) < tot) {
    displayError("Espaco insuficiente", true);
    return false;
  }
  size_t bufSize = 1024;
  uint8_t *buff = nullptr;

  if (psramFound()) {
    bufSize = 65536; // 64KB for faster copy
    buff = (uint8_t *)ps_malloc(bufSize);
  }

  if (!buff) {
    bufSize = 4096; // 4KB fallback on SRAM
    buff = (uint8_t *)malloc(bufSize);
  }

  if (!buff) {
    displayError("Sem memoria para buffer", true);
    source.close();
    dest.close();
    return false;
  }

  while ((bytesRead = source.read(buff, bufSize)) > 0) {
    if (dest.write(buff, bytesRead) != bytesRead) {
      source.close();
      dest.close();
      free(buff);
      Serial.println("Error 5: Falha ao escrever no arquivo de destino");
      return false;
    } else {
      prog += bytesRead;
      float rad = 360.0f * (float)prog / (float)tot;
      if (draw)
        tft.drawArc(tftWidth / 2, tftHeight / 2, tftHeight / 4, tftHeight / 5,
                    0, int(rad), ALCOLOR, willyConfig.bgColor, true);
    }
  }
  free(buff);

  if (prog == tot)
    result = true;
  else {
    displayError("Falha ao Copiar Arquivo", true);
    return false;
  }

  return result;
}

/***************************************************************************************
** Function name: copyFile
** Description:   copy file address to memory
***************************************************************************************/
bool copyFile(fs::FS &fs, String path) {
  File file = fs.open(path, FILE_READ);
  if (!file.isDirectory()) {
    fileToCopy = path;
    file.close();
    return true;
  } else {
    displayRedStripe("Nao e possivel copiar Pasta");
    file.close();
    return false;
  }
}

/***************************************************************************************
** Function name: pasteFile
** Description:   paste file to new folder
***************************************************************************************/
bool pasteFile(fs::FS &fs, String path) {
  // Using Global Buffer

  // Abrir o arquivo original
  File sourceFile = fs.open(fileToCopy, FILE_READ);
  if (!sourceFile) {
    // Serial.println("Falha ao abrir o arquivo original para leitura");
    return false;
  }

  // Criar o arquivo de destino
  File destFile = fs.open(
      path + "/" + fileToCopy.substring(fileToCopy.lastIndexOf('/') + 1),
      FILE_WRITE);
  if (!destFile) {
    // Serial.println("Falha ao criar o arquivo de destino");
    sourceFile.close();
    return false;
  }

  // Ler dados do arquivo original e escrever no arquivo de destino
  size_t bytesRead;
  int tot = sourceFile.size();
  size_t prog = 0;
  size_t bufSize = 1024;
  uint8_t *buff = nullptr;

  if (psramFound()) {
    bufSize = 65536; // 64KB for faster copy
    buff = (uint8_t *)ps_malloc(bufSize);
  }

  if (!buff) {
    bufSize = 4096; // 4KB fallback on SRAM
    buff = (uint8_t *)malloc(bufSize);
  }

  if (!buff) {
    displayError("Sem memoria para buffer", true);
    sourceFile.close();
    destFile.close();
    return false;
  }

  while ((bytesRead = sourceFile.read(buff, bufSize)) > 0) {
    if (destFile.write(buff, bytesRead) != bytesRead) {
      sourceFile.close();
      destFile.close();
      free(buff);
      return false;
    } else {
      prog += bytesRead;
      float rad = 360.0f * (float)prog / (float)tot;
      tft.drawArc(tftWidth / 2, tftHeight / 2, tftHeight / 4, tftHeight / 5, 0,
                  int(rad), ALCOLOR, willyConfig.bgColor, true);
    }
  }

  free(buff);
  // Fechar ambos os arquivos
  sourceFile.close();
  destFile.close();
  return true;
}

/***************************************************************************************
** Function name: createFolder
** Description:   create new folder
***************************************************************************************/
bool createFolder(fs::FS &fs, String path) {
  String foldername = keyboard("", 76, "Nome da Pasta: ");
  if (!fs.mkdir(path + "/" + foldername)) {
    displayRedStripe("Nao foi possivel criar pasta");
    return false;
  }
  return true;
}

/**********************************************************************
**  Function: readLineFromFile
**  Read the line of the config file until the ';'
**********************************************************************/
String readLineFromFile(File myFile) {
  String line = "";
  char character;

  while (myFile.available()) {
    character = myFile.read();
    if (character == ';') {
      break;
    }
    line += character;
  }
  return line;
}

/***************************************************************************************
** Function name: readSmallFile
** Description:   read a small (<3KB) file and return its contents as a single
* string
**                on any error returns an empty string
***************************************************************************************/
String readSmallFile(fs::FS &fs, String filepath) {
  String fileContent = "";
  File file;

  if (static_cast<fs::FS *>(&fs) == static_cast<fs::FS *>(&SD) && spiMutex &&
      xSemaphoreTake(spiMutex, pdMS_TO_TICKS(1000)) != pdTRUE) {
    return "";
  }
  file = fs.open(filepath, FILE_READ);
  if (!file) {
    if (static_cast<fs::FS *>(&fs) == static_cast<fs::FS *>(&SD) && spiMutex)
      xSemaphoreGive(spiMutex);
    return "";
  }

  size_t fileSize = file.size();
  if (fileSize > SAFE_STACK_BUFFER_SIZE || fileSize > ESP.getFreeHeap()) {
    displayError("Arquivo muito grande", true);
    file.close();
    if (static_cast<fs::FS *>(&fs) == static_cast<fs::FS *>(&SD) && spiMutex)
      xSemaphoreGive(spiMutex);
    return "";
  }

  // Use PSRAM if available to save SRAM
  char *buffer = nullptr;
  if (psramFound()) {
    buffer = (char *)heap_caps_malloc(fileSize + 1,
                                      MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  } else {
    buffer = (char *)malloc(fileSize + 1);
  }

  if (buffer != nullptr) {
    file.read((uint8_t *)buffer, fileSize);
    buffer[fileSize] = '\0';
    fileContent = String(buffer);
    free(buffer);
  } else {
    Serial.println("[SD] Falha ao alocar buffer para leitura de arquivo");
    // Fallback to slow readString() if malloc failed
    fileContent = file.readString();
  }

  file.close();
  if (static_cast<fs::FS *>(&fs) == static_cast<fs::FS *>(&SD) && spiMutex)
    xSemaphoreGive(spiMutex);
  return fileContent;
}

/***************************************************************************************
** Function name: readFile
** Description:   read file and return its contents as a char*
**                caller needs to call free()
***************************************************************************************/
char *readBigFile(fs::FS *fs, String filepath, bool binary, size_t *fileSize) {
  if (static_cast<fs::FS *>(fs) == static_cast<fs::FS *>(&SD) && spiMutex &&
      xSemaphoreTake(spiMutex, pdMS_TO_TICKS(1000)) != pdTRUE) {
    return NULL;
  }
  File file = fs->open(filepath);
  if (!file) {
    Serial.printf("Could not open file: %s\n", filepath.c_str());
    if (static_cast<fs::FS *>(fs) == static_cast<fs::FS *>(&SD) && spiMutex)
      xSemaphoreGive(spiMutex);
    return NULL;
  }

  size_t fileLen = file.size();
  char *buf =
      (char *)(psramFound() ? ps_malloc(fileLen + 1) : malloc(fileLen + 1));
  if (fileSize != NULL) {
    *fileSize = file.size();
  }

  if (!buf) {
    Serial.printf("Could not allocate memory for file: %s\n", filepath.c_str());
    if (static_cast<fs::FS *>(fs) == static_cast<fs::FS *>(&SD) && spiMutex)
      xSemaphoreGive(spiMutex);
    return NULL;
  }

  size_t bytesRead = 0;
  while (bytesRead < fileLen && file.available()) {
    size_t toRead = fileLen - bytesRead;
    if (toRead > 512) {
      toRead = 512;
    }
    file.read((uint8_t *)(buf + bytesRead), toRead);
    bytesRead += toRead;
  }
  buf[bytesRead] = '\0';
  file.close();
  if (static_cast<fs::FS *>(fs) == static_cast<fs::FS *>(&SD) && spiMutex)
    xSemaphoreGive(spiMutex);

  return buf;
}
/***************************************************************************************
** Function name: getFileSize
** Description:   get a file size without opening
***************************************************************************************/
size_t getFileSize(fs::FS &fs, String filepath) {
  File file = fs.open(filepath, FILE_READ);
  if (!file)
    return 0;
  size_t fileSize = file.size();
  file.close();
  return fileSize;
}

String md5File(fs::FS &fs, String filepath) {
  File file = fs.open(filepath, FILE_READ);
  if (!file)
    return "";

  md5_context_t ctx;
  esp_rom_md5_init(&ctx);

  uint8_t buffer[512];
  while (file.available()) {
    size_t len = file.read(buffer, sizeof(buffer));
    esp_rom_md5_update(&ctx, buffer, len);
  }

  uint8_t hash[16];
  esp_rom_md5_final(hash, &ctx);
  file.close();

  char s[33];
  for (int i = 0; i < 16; i++) {
    sprintf(&s[i * 2], "%02x", hash[i]);
  }
  return String(s);
}

String crc32File(fs::FS &fs, String filepath) {
  File file = fs.open(filepath, FILE_READ);
  if (!file)
    return "";

  uint32_t crc = 0xFFFFFFFF;
  uint8_t buffer[512];
  while (file.available()) {
    size_t len = file.read(buffer, sizeof(buffer));
    crc = esp_rom_crc32_le(crc, buffer, len);
  }
  crc ^= 0xFFFFFFFF;
  file.close();

  char s[10];
  sprintf(s, "%08lX", crc);
  return String(s);
}

/***************************************************************************************
** Function name: sortList
** Description:   sort files/folders by name
***************************************************************************************/
bool sortList(const FileList &a, const FileList &b) {
  if (a.folder != b.folder) {
    return a.folder > b.folder; // true if a is a folder and b is not
  }
  // Order items alphabetically case-insensitive
  String fa = a.filename.c_str();
  String fb = b.filename.c_str();
  fa.toUpperCase();
  fb.toUpperCase();
  return fa < fb;
}

/***************************************************************************************
** Function name: checkExt
** Description:   check file extension
***************************************************************************************/
bool checkExt(String ext, String pattern) {
  ext.toUpperCase();
  pattern.toUpperCase();
  if (ext == pattern)
    return true;

  // If the pattern is a list of extensions (e.g., "TXT|JPG|PNG"), split and
  // check
  int start = 0;
  int end = pattern.indexOf('|');
  while (end != -1) {
    String currentExt = pattern.substring(start, end);
    if (ext == currentExt) {
      return true;
    }
    start = end + 1;
    end = pattern.indexOf('|', start);
  }

  // Check the last extension in the list
  String lastExt = pattern.substring(start);
  return ext == lastExt;
}

/***************************************************************************************
** Function name: readFs
** Description:   read files/folders from a folder
***************************************************************************************/
void readFs(fs::FS &fs, String folder, String allowed_ext) {
  if (static_cast<fs::FS *>(&fs) == static_cast<fs::FS *>(&SD) && spiMutex &&
      xSemaphoreTake(spiMutex, pdMS_TO_TICKS(1000)) != pdTRUE) {
    return;
  }
  fileList.clear();
  FileList object;

  File root = fs.open(folder);
  if (!root || !root.isDirectory()) {
    if (static_cast<fs::FS *>(&fs) == static_cast<fs::FS *>(&SD) && spiMutex)
      xSemaphoreGive(spiMutex);
    return;
  }

  while (true) {
    bool isDir;
    String fullPath = root.getNextFileName(&isDir);
    String nameOnly = fullPath.substring(fullPath.lastIndexOf("/") + 1);
    if (fullPath == "") {
      break;
    }
    // Serial.printf("Path: %s (isDir: %d)\n", fullPath.c_str(), isDir);

    if (isDir) {
      object.filename = nameOnly;
      object.folder = true;
      object.operation = false;
      fileList.push_back(object);
    } else {
      int dotIndex = nameOnly.lastIndexOf(".");
      String ext = dotIndex >= 0 ? nameOnly.substring(dotIndex + 1) : "";
      if (allowed_ext == "*" || checkExt(ext, allowed_ext)) {
        object.filename = nameOnly;
        object.folder = false;
        object.operation = false;
        fileList.push_back(object);
      }
    }
  }
  root.close();
  if (static_cast<fs::FS *>(&fs) == static_cast<fs::FS *>(&SD) && spiMutex)
    xSemaphoreGive(spiMutex);

  // Sort folders/files
  std::sort(fileList.begin(), fileList.end(), sortList);

  Serial.println("Files listed with: " + String(fileList.size()) +
                 " files/folders found");

  // Adds Operational btn at the botton
  object.filename = "> Voltar";
  object.folder = false;
  object.operation = true;

  fileList.push_back(object);
}

/*********************************************************************
**  Function: loopSD
**  Where you choose what to do with your SD Files
**********************************************************************/
String loopSD(fs::FS &fs, bool filePicker, String allowed_ext,
              String rootPath) {
  delay(10);

  // Ensure SD is mounted before checking if paths exist!
  if (&fs == static_cast<fs::FS *>(&SD)) {
    if (!setupSdCard()) {
      displayError("Falha ao Montar SD", true);
      return "";
    }
  }

  if (!fs.exists(rootPath)) {
    Serial.println("loopSD-> 1st exist test failed, retrying after delay...");
    delay(150); // Give FS some time to stabilize
    if (!fs.exists(rootPath)) {
      rootPath = "/";
      if (!fs.exists(rootPath)) {
        Serial.println("loopSD-> 2nd exist test failed");
        if (&fs == static_cast<fs::FS *>(&SD))
          sdcardMounted = false;
        return "";
      }
    }
  }

  Opt_Coord coord;
  String result = "";
  const short PAGE_JUMP_SIZE = (tftHeight - 20) / (LH * FM);
  bool reload = false;
  bool redraw = true;
  int index = 0;
  int maxFiles = 0;
  String Folder = rootPath;
  String PreFolder = rootPath;
  tft.drawPixel(0, 0, 0);
  tft.fillScreen(
      willyConfig.bgColor); // TODO: Does only the T-Embed CC1101 need this?
  tft.drawRoundRect(5, 5, tftWidth - 10, tftHeight - 10, 5,
                    willyConfig.priColor);

  bool exit = false;
  // returnToMenu=true;  // make sure menu is redrawn when quitting in any point

  readFs(fs, Folder, allowed_ext);

  maxFiles = fileList.size() - 1; // discount the >back operator
  LongPress = false;
  unsigned long LongPressTmp = millis();
  while (1) {
    delay(10);
    // if(returnToMenu) break; // stop this loop and retur to the previous loop
    if (exit)
      break; // stop this loop and retur to the previous loop

    if (redraw) {
      if (strcmp(PreFolder.c_str(), Folder.c_str()) != 0 || reload) {
        index = 0;
        tft.fillScreen(willyConfig.bgColor);
        tft.drawRoundRect(5, 5, tftWidth - 10, tftHeight - 10, 5,
                          willyConfig.priColor);
        Serial.println("reload to read: " + Folder);
        readFs(fs, Folder, allowed_ext);
        PreFolder = Folder;
        maxFiles = fileList.size() - 1;
        if (strcmp(PreFolder.c_str(), Folder.c_str()) != 0 || index > maxFiles)
          index = 0;
        reload = false;
      }
      if (fileList.size() < 2)
        readFs(fs, Folder, allowed_ext);

      coord = listFiles(index, fileList);
#if defined(HAS_TOUCH)
      TouchFooter();
#endif
      redraw = false;
    }
    displayScrollingText(fileList[index].filename, coord);

    // !PrevPress enables EscPress on 3Btn devices to be used in Serial
    // Navigation This condition is important for StickCPlus, Core and other 3
    // Btn devices
    if (EscPress && PrevPress)
      EscPress = false;
    if (check(EscPress))
      goto BACK_FOLDER;

#ifdef HAS_KEYBOARD
    char pressed_letter = checkLetterShortcutPress();

    // check letter shortcuts
    if (pressed_letter > 0) {
      // Serial.println(pressed_letter);
      if (tolower(fileList[index].filename.c_str()[0]) == pressed_letter) {
        // already selected, go to the next
        index += 1;
        // check if index is still valid
        if (index <= maxFiles &&
            tolower(fileList[index].filename.c_str()[0]) == pressed_letter) {
          redraw = true;
          continue;
        }
      }
      // else look again from the start
      for (int i = 0; i < maxFiles; i++) {
        if (tolower(fileList[i].filename.c_str()[0]) ==
            pressed_letter) { // check if 1st char matches
          index = i;
          redraw = true;
          break; // quit on 1st match
        }
      }
    }
#endif

    if (check(PrevPress) || check(UpPress)) {
      if (index == 0)
        index = maxFiles;
      else if (index > 0)
        index--;
      redraw = true;
    }
    /* DW Btn to next item */
    if (check(NextPress) || check(DownPress)) {
      if (index == maxFiles)
        index = 0;
      else
        index++;
      redraw = true;
    }
    if (check(NextPagePress)) {
      index += PAGE_JUMP_SIZE;
      if (index > maxFiles)
        index = maxFiles - 1; // check bounds
      redraw = true;
      continue;
    }
    if (check(PrevPagePress)) {
      index -= PAGE_JUMP_SIZE;
      if (index < 0)
        index = 0; // check bounds
      redraw = true;
      continue;
    }
    /* Select to install */
    if (LongPress || SelPress) {
      if (!LongPress) {
        LongPress = true;
        LongPressTmp = millis();
      }
      if (LongPress && millis() - LongPressTmp < 500)
        goto WAITING;
      LongPress = false;

      if (check(SelPress)) {
        if (fileList[index].folder == true &&
            fileList[index].operation == false) {
          options = {
              Option{"Nova Pasta", [&]() { createFolder(fs, Folder); }},
              Option{"Renomear",
                     [&]() {
                       renameFile(fs, Folder + fileList[index].filename,
                                  fileList[index].filename);
                     }},
              Option{"Deletar",
                     [&]() {
                       deleteFromSd(fs,
                                    Folder + "/" + fileList[index].filename);
                     }},
              Option{"Fechar Menu", [&]() { yield(); }},
              Option{"Menu Principal", [&]() { exit = true; }},
          };
          loopOptions(options);
          tft.drawRoundRect(5, 5, tftWidth - 10, tftHeight - 10, 5,
                            willyConfig.priColor);
          reload = true;
          redraw = true;
        } else if (fileList[index].folder == false &&
                   fileList[index].operation == false) {
          goto Files;
        } else {
          options = {
              Option{"Nova Pasta", [&]() { createFolder(fs, Folder); }},
          };
          if (fileToCopy != "")
            options.push_back(
                Option{"Colar", [&]() { pasteFile(fs, Folder); }});
          options.push_back(Option{"Fechar Menu", [&]() { yield(); }});
          options.push_back(Option{"Menu Principal", [&]() { exit = true; }});
          loopOptions(options);
          tft.drawRoundRect(5, 5, tftWidth - 10, tftHeight - 10, 5,
                            willyConfig.priColor);
          reload = true;
          redraw = true;
        }
      } else {
      Files:
        if (fileList[index].folder == true &&
            fileList[index].operation == false) {
          Folder = Folder + (Folder == "/" ? "" : "/") +
                   fileList[index].filename; // Folder=="/"? "":"/" +
          // Debug viewer
          Serial.println(Folder);
          redraw = true;
        } else if (fileList[index].folder == false &&
                   fileList[index].operation == false) {
          // Save the file/folder info to Clear memory to allow other functions
          // to work better
          String filepath =
              Folder + (Folder == "/" ? "" : "/") + fileList[index].filename; //
          String filename = fileList[index].filename;
          // Debug viewer
          Serial.println(filepath + " --> " + filename);
          fileList
              .clear(); // Clear memory to allow other functions to work better

          options = {
              Option{"Ver Arquivo", [&]() { viewFile(fs, filepath); }},
              Option{"Info do Arquivo", [&]() { fileInfo(fs, filepath); }},
              Option{"Renomear", [&]() { renameFile(fs, filepath, filename); }},
              Option{"Copiar", [&]() { copyFile(fs, filepath); }},
              Option{"Deletar", [&]() { deleteFromSd(fs, filepath); }},
              Option{"Nova Pasta", [&]() { createFolder(fs, Folder); }},
          };
          if (fileToCopy != "")
            options.push_back(
                Option{"Colar", [&]() { pasteFile(fs, Folder); }});
          if (static_cast<fs::FS *>(&fs) == static_cast<fs::FS *>(&SD))
            options.push_back(Option{"Copiar->LittleFS", [&]() {
                                       copyToFs(SD, LittleFS, filepath);
                                     }});
          if (static_cast<fs::FS *>(&fs) == static_cast<fs::FS *>(&LittleFS) &&
              sdcardMounted)
            options.push_back(Option{
                "Copiar->SD", [&]() { copyToFs(LittleFS, SD, filepath); }});

          // custom file formats commands added in front
          if (filepath.endsWith(".jpg") || filepath.endsWith(".gif") ||
              filepath.endsWith(".bmp") || filepath.endsWith(".png"))
            options.insert(options.begin(),
                           Option{"Ver Imagem", [&]() {
                                    drawImg(fs, filepath, 0, 0, true, -1);
                                    delay(750);
                                    while (!check(AnyKeyPress))
                                      vTaskDelay(10 / portTICK_PERIOD_MS);
                                  }});
          if (filepath.endsWith(".ir")) {
            options.insert(options.begin(), Option{"Escolher cmd IR", [&]() {
                                                     delay(200);
                                                     chooseCmdIrFile(&fs,
                                                                     filepath);
                                                   }});
            options.insert(options.begin(), Option{"IR Tx SpamTodos", [&]() {
                                                     delay(200);
                                                     txIrFile(&fs, filepath);
                                                   }});
          }
          if (filepath.endsWith(".sub"))
            options.insert(options.begin(), Option{"Subghz Tx", [&]() {
                                                     delay(200);
                                                     txSubFile(&fs, filepath);
                                                   }});
          if (filepath.endsWith(".csv")) {
            options.insert(options.begin(), Option{"Upload Wigle", [&]() {
                                                     delay(200);
                                                     Wigle wigle;
                                                     wigle.upload(&fs,
                                                                  filepath);
                                                   }});
            options.insert(options.begin(), Option{"Upload Wigle Tudo", [&]() {
                                                     delay(200);
                                                     Wigle wigle;
                                                     wigle.upload_all(&fs,
                                                                      Folder);
                                                   }});
          }
#if !defined(LITE_VERSION) && !defined(DISABLE_INTERPRETER)
          if (filepath.endsWith(".bjs") || filepath.endsWith(".js")) {
            options.insert(options.begin(),
                           Option{"Executar Script JS", [&]() {
                                    delay(200);
                                    run_js_script_headless(fs, filepath);
                                    exit = true;
                                  }});
          }
#endif
#if defined(USB_as_HID)
          if (filepath.endsWith(".txt")) {
            options.push_back(Option{"Executar BadUSB", [&]() {
                                       ducky_startKb(hid_usb, false);
                                       key_input(fs, filepath, hid_usb);
                                       delete hid_usb;
                                       hid_usb = nullptr;
                                       Serial.begin(
                                           115200); // Reinit serial port
                                     }});
            options.push_back(Option{"USB HID Digitar", [&]() {
                                       String t = readSmallFile(fs, filepath);
                                       displayRedStripe("Digitando");
                                       key_input_from_string(t);
                                     }});
          }
          if (filepath.endsWith(".enc")) { // encrypted files
            options.insert(options.begin(),
                           Option{"Descript+Digitar", [&]() {
                                    String plaintext =
                                        readDecryptedFile(fs, filepath);
                                    if (plaintext.length() == 0)
                                      return displayError(
                                          "Falha na descriptografia",
                                          true); // file is too big or cannot
                                                 // read, or cancelled
                                    // else
                                    plaintext.trim(); // remove newlines
                                    key_input_from_string(plaintext);
                                  }});
          }
#endif
          if (filepath.endsWith(".enc")) { // encrypted files
            options.insert(
                options.begin(),
                Option{"Decrypt+Show", [&]() {
                         String plaintext = readDecryptedFile(fs, filepath);
                         delay(200);
                         if (plaintext.length() == 0)
                           return displayError("Decryption failed", true);
                         plaintext.trim(); // remove newlines

                         // Show decrypted text: use text viewer for longer text
                         if (plaintext.length() > 200) {
                           // Create temporary file in the same FS
                           String tmpPath = "/tmp_decrypted_view.txt";
                           File tmpFile = fs.open(tmpPath, FILE_WRITE);
                           if (!tmpFile) {
                             displayError("Failed to create temp file", true);
                             return;
                           }
                           tmpFile.print(plaintext);
                           tmpFile.close();

                           // Show in text viewer
                           viewFile(fs, tmpPath);

                           // Clean up
                           fs.remove(tmpPath);
                         } else {
                           // Short text: show directly
                           displaySuccess(plaintext, true);
                         }
                       }});
          }
#if defined(HAS_NS4168_SPKR)
          if (isAudioFile(filepath))
            options.insert(options.begin(), Option{"Play Audio", [&]() {
                                                     delay(200);
                                                     check(AnyKeyPress);
                                                     // playAudioFile(&fs,
                                                     // filepath);
                                                     musicPlayerUI(&fs,
                                                                   filepath);
                                                   }});
#endif
          // generate qr codes from small files (<3K)
          size_t filesize = getFileSize(fs, filepath);
          // Serial.println(filesize);
          if (filesize < SAFE_STACK_BUFFER_SIZE && filesize > 0) {
            options.push_back(Option{"QR code", [&]() {
                                       delay(200);
                                       qrcode_display(
                                           readSmallFile(fs, filepath));
                                     }});
            options.push_back(Option{"CRC32", [&]() {
                                       delay(200);
                                       displaySuccess(crc32File(fs, filepath),
                                                      true);
                                     }});
            options.push_back(Option{"MD5", [&]() {
                                       delay(200);
                                       displaySuccess(md5File(fs, filepath),
                                                      true);
                                     }});
          }
          options.push_back(Option{"Close Menu", [&]() { yield(); }});
          options.push_back({"Main Menu", [&]() { exit = true; }});
          if (!filePicker)
            loopOptions(options);
          else {
            result = filepath;
            break;
          }
          tft.drawRoundRect(5, 5, tftWidth - 10, tftHeight - 10, 5,
                            willyConfig.priColor);
          reload = true;
          redraw = true;
        } else {
        BACK_FOLDER:
          if (Folder == "/")
            break;
          Folder = Folder.substring(0, Folder.lastIndexOf('/'));
          if (Folder == "")
            Folder = "/";
          Serial.println("Going to folder: " + Folder);
          index = 0;
          redraw = true;
        }
        redraw = true;
      }
    WAITING:
      delay(10);
    }
  }
  fileList.clear();
  return result;
}

/*********************************************************************
**  Function: viewFile
**  Display file content
**********************************************************************/
void viewFile(FS &fs, String filepath) {
  File file = fs.open(filepath, FILE_READ);
  if (!file)
    return;

  ScrollableTextArea area = ScrollableTextArea("VIEW FILE");
  area.fromFile(file);

  file.close();

  area.show();
}

/*********************************************************************
**  Function: checkLittleFsSize
**  Check if there are more then 4096 bytes available for storage
**********************************************************************/
bool checkLittleFsSize() {
  if ((LittleFS.totalBytes() - LittleFS.usedBytes()) < 4096) {
    displayError("LittleFS is Full", true);
    return false;
  } else
    return true;
}

/*********************************************************************
**  Function: checkLittleFsSize
**  Check if there are more then 4096 bytes available for storage
**********************************************************************/
bool checkLittleFsSizeNM() {
  return (LittleFS.totalBytes() - LittleFS.usedBytes()) >= 4096;
}

/*********************************************************************
**  Function: getFsStorage
**  Function will return true and FS will point to SDFS if available
**  and LittleFS otherwise. If LittleFS is full it wil return false.
**********************************************************************/
bool getFsStorage(FS *&fs) {
  // don't try to mount SD Card if not previously mounted
  if (sdcardMounted)
    fs = &SD;
  else if (checkLittleFsSize())
    fs = &LittleFS;
  else
    return false;

  return true;
}

/*********************************************************************
**  Function: fileInfo
**  Display file info
**********************************************************************/
void fileInfo(FS &fs, String filepath) {
  File file = fs.open(filepath, FILE_READ);
  if (!file)
    return;

  int bytesize = file.size();
  float filesize = bytesize;
  String unit = "B";

  time_t modifiedTime = file.getLastWrite();

  if (filesize >= 1000000) {
    filesize /= 1000000.0;
    unit = "MB";
  } else if (filesize >= 1000) {
    filesize /= 1000.0;
    unit = "kB";
  }

  drawMainBorderWithTitle("FILE INFO");
  padprintln("");
  padprintln("Path: " + filepath);
  padprintln("");
  padprintf("Bytes: %d\n", bytesize);
  padprintln("");
  padprintf("Size: %.02f %s\n", filesize, unit.c_str());
  padprintln("");
  padprintf("Modified: %s\n", ctime(&modifiedTime));

  file.close();
  delay(100);

  while (!check(EscPress) && !check(SelPress)) {
    delay(100);
  }

  return;
}

/*********************************************************************
**  Function: createNewFile
**  Function will save a file into FS. If file already exists it will
**  append a version number to the file name.
**********************************************************************/
File createNewFile(FS *&fs, String filepath, String filename) {
  int extIndex = filename.lastIndexOf('.');
  String name = filename.substring(0, extIndex);
  String ext = filename.substring(extIndex);

  if (filepath.endsWith("/"))
    filepath = filepath.substring(0, filepath.length() - 1);
  if (!(*fs).exists(filepath))
    (*fs).mkdir(filepath);

  name = filepath + "/" + name;

  if ((*fs).exists(name + ext)) {
    int i = 1;
    name += "_";
    while ((*fs).exists(name + String(i) + ext))
      i++;
    name += String(i);
  }

  Serial.println("Creating file: " + name + ext);
  File file = (*fs).open(name + ext, FILE_WRITE);
  return file;
}
