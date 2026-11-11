/**
 * @file amiibolink.cpp
 * @author Rennan Cockles (https://github.com/rennancockles)
 * @brief Emulate amiibo using an amiibolink
 * @version 0.2
 * @date 2024-10-11
 */
#ifndef LITE_VERSION
#include "amiibo.h"
#include "core/display.h"
#include "core/mykeyboard.h"
#include "globals.h"

Amiibo::Amiibo() { setup(); }

Amiibo::~Amiibo() {}

void Amiibo::setup() {
    displayBanner();

    if (!connect()) return;

    vTaskDelay(pdMS_TO_TICKS(500));
    selectMode();
}

bool Amiibo::connect() {
    displayInfo("Ligue o dispositivo Amiibolink", true);

    displayBanner();
    padprintln("");
    padprintln("Buscando dispositivo Amiibolink...");

    if (!amiibolink.searchDevice()) {
        displayError("Amiibolink nao encontrado");
        delayWithReturn(1000);
        return false;
    }

    if (!amiibolink.connectToDevice()) {
        displayError("Erro de conexao Amiibolink", true);
        return false;
    }

    displaySuccess("Amiibolink Conectado");
    delayWithReturn(1000);

    return true;
}

void Amiibo::displayBanner(AppMode mode) {
    drawMainBorderWithTitle("AMIIBOLINK");

    switch (mode) {
        case AMIIBO_UPLOAD: printSubtitle("UPLOAD AMIIBO"); break;
        case CHANGE_UID_MODE: printSubtitle("DEFINIR MODO UID"); break;
        default: padprintln(""); break;
    }

    tft.setTextSize(FP);
    padprintln("");
}

void Amiibo::selectMode() {
    options = {
        {"Enviar Amiibo", [this]() { uploadAmiibo(); } },
        {"Modo UID",  [this]() { changeUIDMode(); }},
    };

    loopOptions(options);
}

void Amiibo::uploadAmiibo() {
    if (!openDumpFile()) return;

    if (!checkEmulationTagType()) {
        displayError("Tipo de tag invalido", true);
        return;
    }

    displayBanner(AMIIBO_UPLOAD);
    displayInfo("Enviando comandos...");

    bool success =
        (amiibolink.cmdPreUploadDump() && amiibolink.cmdUploadDumpData(strDump) &&
         amiibolink.cmdPostUploadDump());

    if (success) {
        displaySuccess("Sucesso");
        delayWithReturn(500);
    } else {
        displayError("Erro de comunicacao Amiibolink", true);
    }

    delayWithReturn(500);
}

void Amiibo::changeUIDMode() {
    Amiibolink::UIDMode uidMode;

    options = {
        {"Aleatorio Auto",   [&]() { uidMode = Amiibolink::UIDMode_Auto; }  },
        {"Aleatorio Manual", [&]() { uidMode = Amiibolink::UIDMode_Manual; }},
    };
    loopOptions(options);

    displayBanner(CHANGE_UID_MODE);

    if (amiibolink.cmdSetUIDMode(uidMode)) {
        displaySuccess("Sucesso");
        delayWithReturn(500);
    } else {
        displayError("Erro de comunicacao Amiibolink", true);
    }

    delayWithReturn(500);
}

bool Amiibo::openDumpFile() {
    String filepath;
    File file;
    FS *fs;

    if (!getFsStorage(fs)) {
        displayError("Erro de armazenamento", true);
        return false;
    }

    if (!(*fs).exists("/WillyRFID")) (*fs).mkdir("/WillyRFID");
    filepath = loopSD(*fs, true, "RFID|NFC", "/WillyRFID");
    file = fs->open(filepath, FILE_READ);

    if (!file) {
        displayError("Erro no arquivo dump", true);
        return false;
    }

    String line;
    String strData;
    bool pageReadSuccess = true;
    strDump = "";

    while (file.available()) {
        line = file.readStringUntil('\n');
        strData = line.substring(line.indexOf(":") + 1);
        strData.trim();
        if (line.startsWith("Device type:")) printableUID.picc_type = strData;
        if (line.startsWith("UID:")) printableUID.uid = strData;
        if (line.startsWith("SAK:")) printableUID.sak = strData;
        if (line.startsWith("ATQA:")) printableUID.atqa = strData;
        if (line.startsWith("Pages read:")) pageReadSuccess = false;
        if (line.startsWith("Page ")) strDump += strData;
    }

    file.close();
    vTaskDelay(pdMS_TO_TICKS(100));

    if (!pageReadSuccess) {
        displayError("Arquivo dump incompleto", true);
        return false;
    }

    printableUID.uid.trim();
    printableUID.uid.replace(" ", "");
    printableUID.sak.trim();
    printableUID.sak.replace(" ", "");
    printableUID.atqa.trim();
    printableUID.atqa.replace(" ", "");
    strDump.trim();
    strDump.replace(" ", "");

    Serial.print("Uid: ");
    Serial.println(printableUID.uid);
    Serial.print("Sak: ");
    Serial.println(printableUID.sak);
    Serial.print("Data: ");
    Serial.println(strDump);
    // byte sak = strtoul(printableUID.sak.c_str(), NULL, 16);

    return true;
}

bool Amiibo::checkEmulationTagType() {
    if (strDump.length() == 0) return false;
    byte sak = strtoul(printableUID.sak.c_str(), NULL, 16);
    if (sak != 0x00) return false;

    if (strDump.substring(0, 8) == strDump.substring(strDump.length() - 8)) {
        strDump = strDump.substring(0, strDump.length() - 8);
    }

    if (strDump.length() / 2 != 540) return false; // Not an NTAG_215

    return true;
}

void Amiibo::delayWithReturn(uint32_t ms) {
    auto tm = millis();
    while (millis() - tm < ms && !returnToMenu) { vTaskDelay(pdMS_TO_TICKS(50)); }
}

#endif
