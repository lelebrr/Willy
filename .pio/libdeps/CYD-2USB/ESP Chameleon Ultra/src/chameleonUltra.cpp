/**
 * @file chameleonUltra.cpp
 * @author Rennan Cockles (https://github.com/rennancockles)
 * @brief ESP Chameleon Ultra
 * @version 0.1
 * @date 2024-10-09
 */

#include "chameleonUltra.h"

#define MAX_DUMP_SIZE 160


std::vector<ChameleonUltra::CmdResponse> chameleonResponses;


uint8_t calculateLRC(const uint8_t *data, size_t length) {
    uint8_t lrc = 0;

    for (size_t i = 0; i < length; i++) {
        lrc += data[i];
    }
    lrc = 0x100 - (lrc & 0xff);

    return lrc;
}

#ifdef NIMBLE_V2_PLUS
#define NimBLEAdvertisedDeviceCallbacks NimBLEScanCallbacks
#endif

class scanCallbacks : public NimBLEAdvertisedDeviceCallbacks {
    void onResult(NimBLEAdvertisedDevice* advertisedDevice) {
        if (advertisedDevice->getName() == "ChameleonUltra") {
            NimBLEDevice::getScan()->stop();
        }
    }
};


void chameleonNotifyCB(NimBLERemoteCharacteristic* pRemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify){
    ChameleonUltra::CmdResponse rsp;

    rsp.length = length;
    memcpy(rsp.raw, pData, length);

    rsp.command = (pData[2] << 8) | pData[3];
    rsp.status = pData[5];
    rsp.dataSize = pData[7];

    if (rsp.dataSize > 0) {
        memcpy(rsp.data, pData+9, rsp.dataSize);
    }

    chameleonResponses.push_back(rsp);
}


ChameleonUltra::ChameleonUltra(bool debug) { _debug = debug;}


ChameleonUltra::~ChameleonUltra() {
    if (_debug) Serial.println("Killing Chameleon...");
#ifdef NIMBLE_V2_PLUS
    if(_device) {
        delete _device;
        _device = nullptr;
    }
    if (NimBLEDevice::isInitialized())
#else
    if (NimBLEDevice::getInitialized())
#endif
    {
        if (_debug) Serial.println("Deiniting ble...");
#if defined(CONFIG_IDF_TARGET_ESP32C5)
        esp_bt_controller_deinit();
#else
        BLEDevice::deinit();
#endif
    }
}


bool ChameleonUltra::searchChameleonDevice() {
    NimBLEDevice::init("");

    NimBLEScan* pScan = NimBLEDevice::getScan();

    #ifdef NIMBLE_V2_PLUS
    pScan->setScanCallbacks(new scanCallbacks());
    pScan->setActiveScan(true);

    BLEScanResults foundDevices = pScan->getResults(150);
    bool chameleonFound = false;

    for (int i=0; i<foundDevices.getCount(); i++) {
        const NimBLEAdvertisedDevice *advertisedDevice = foundDevices.getDevice(i);

        if (advertisedDevice->getName() == "ChameleonUltra") {
            chameleonFound = true;
            if(_device) {
                delete _device;
                _device = nullptr;
            }
            _device = new NimBLEAdvertisedDevice(*advertisedDevice);
        }
    }
    #else
    pScan->setAdvertisedDeviceCallbacks(new scanCallbacks());
    pScan->setActiveScan(true);

    BLEScanResults foundDevices = pScan->start(5);
    bool chameleonFound = false;

    for (int i=0; i<foundDevices.getCount(); i++) {
        NimBLEAdvertisedDevice advertisedDevice = foundDevices.getDevice(i);

        if (advertisedDevice.getName() == "ChameleonUltra") {
            chameleonFound = true;
            _device = advertisedDevice;
        }
    }
    #endif

    pScan->clearResults();

    return chameleonFound;
}


bool ChameleonUltra::connectToChamelon() {
    NimBLEClient *pClient = NimBLEDevice::createClient();
    bool chrFound = false;

#ifdef NIMBLE_V2_PLUS
    if (!pClient->connect(_device, false, false, false)) return false;
#else
    if (!pClient->connect(&_device, false)) return false;
#endif

    Serial.print("Connected to: ");
    Serial.println(pClient->getPeerAddress().toString().c_str());

    delay(200);

    NimBLERemoteService* pSvc = nullptr;
    NimBLERemoteCharacteristic* pChrWrite = nullptr;
    NimBLERemoteCharacteristic* pChrNotify = nullptr;

    pSvc = pClient->getService(serviceUUID);
    if (!pSvc) {
        Serial.println("Service does not exist");
        return false;
    }

    pChrWrite = pSvc->getCharacteristic(chrTxUUID);
    pChrNotify = pSvc->getCharacteristic(chrRxUUID);

    if (!pChrWrite || !pChrNotify) {
        Serial.println("Characteristics do not exist");
        return false;
    }

    writeChr = pChrWrite;
    pChrNotify->subscribe(true, chameleonNotifyCB);

    return true;
}


bool ChameleonUltra::chamelonServiceDiscovery() {
    NimBLEClient *pClient = NimBLEDevice::createClient();

#ifdef NIMBLE_V2_PLUS
    if (!pClient->connect(_device, false, false, false)) return false;
#else
    if (!pClient->connect(&_device)) return false;
#endif

    Serial.print("Connected to: ");
    Serial.println(pClient->getPeerAddress().toString().c_str());

    #ifdef NIMBLE_V2_PLUS
    const std::vector<NimBLERemoteService *> pSvcs = pClient->getServices(true);
    Serial.print(pSvcs.size()); Serial.println(" services found");

    for (NimBLERemoteService* pSvc : pSvcs) {
        Serial.println(pSvc->toString().c_str());

        std::vector<NimBLERemoteCharacteristic *> pChrs = pSvc->getCharacteristics(true);
        Serial.print(pChrs.size()); Serial.println(" characteristics found");

        if (pChrs.empty()) continue;

        for (NimBLERemoteCharacteristic* pChr : pChrs)
    #else

    std::vector<NimBLERemoteService *> * pSvcs = pClient->getServices(true);
    Serial.print(pSvcs->size()); Serial.println(" services found");

    for (NimBLERemoteService* pSvc : *pSvcs) {
        Serial.println(pSvc->toString().c_str());

        std::vector<NimBLERemoteCharacteristic *> * pChrs = pSvc->getCharacteristics(true);
        Serial.print(pChrs->size()); Serial.println(" characteristics found");

        if (pChrs->empty()) continue;

        for (NimBLERemoteCharacteristic* pChr : *pChrs) 
    #endif
        {
            Serial.println(pChr->toString().c_str());
            Serial.print("UID size: ");Serial.println(pChr->getUUID().toString().length());
            Serial.print("Value? ");Serial.println(pChr->getValue());
            Serial.print("Can read? ");Serial.println(pChr->canRead());
            Serial.print("Can write? ");Serial.println(pChr->canWrite());
            Serial.print("Can write no response? ");Serial.println(pChr->canWriteNoResponse());
            Serial.print("Can notify? ");Serial.println(pChr->canNotify());
            Serial.print("Can indicate? ");Serial.println(pChr->canIndicate());
            Serial.print("Can broadcast? ");Serial.println(pChr->canBroadcast());


        #ifdef NIMBLE_V2_PLUS
            std::vector<NimBLERemoteDescriptor *> pDscs = pChr->getDescriptors(true);
            Serial.print(pDscs.size()); Serial.println(" descriptors found");
            for (NimBLERemoteDescriptor* pDsc : pDscs) {
                Serial.println(pDsc->toString().c_str());
            }
        #else

            std::vector<NimBLERemoteDescriptor *> * pDscs = pChr->getDescriptors(true);
            Serial.print(pDscs->size()); Serial.println(" descriptors found");
            for (NimBLERemoteDescriptor* pDsc : *pDscs) {
                Serial.println(pDsc->toString().c_str());
            }
        #endif
        }

    }

    return true;
}


bool ChameleonUltra::writeCommand(Command cmd, uint8_t *data, size_t length) {
    uint8_t payload[200] = {
        0x11, 0xef,
        0x00, 0x00,  // command
        0x00, 0x00, 0x00, 0x00,  // data length
        0x00,  // LRC (command + data length)
        0x00  // LRC (data)
    };
    payload[2] = (cmd >> 8) & 0xFF;
    payload[3] = cmd & 0xFF;
    payload[6] = (length >> 8) & 0xFF;
    payload[7] = length & 0xFF;
    payload[8] = calculateLRC(payload+2, 6);
    if (length > 0) memcpy(payload+9, data, length);
    payload[9+length] = calculateLRC(payload+9, length);

    if (_debug) {
        Serial.print("Cmd:");
        for (int i=0; i<10+length; i++) {
            Serial.print(payload[i] < 0x10 ? " 0" : " ");
            Serial.print(payload[i], HEX);
        }
        Serial.println("");
    }

    bool writeRes = writeChr->writeValue(payload, 10+length, true);

    delay(100);

    bool res = checkResponse();

    return writeRes && res;
}


bool ChameleonUltra::checkResponse() {
    while(chameleonResponses.empty()) {delay(10);}

    cmdResponse = chameleonResponses[0];
    bool success = false;

    switch (cmdResponse.status) {
        case SUCCESS:
            success = true;
            break;
        case DEVICE_MODE_ERROR:
            Serial.println("Device mode error");
            success = false;
            break;
        case INVALID_CMD:
            Serial.println("Invalid command");
            success = false;
            break;
        case NOT_IMPLEMENTED:
            Serial.println("Command not implemented");
            success = false;
            break;

        case HF_TAG_OK:
        case LF_TAG_OK:
            success = true;
            break;
        case HF_TAG_NO:
        case EM410X_TAG_NO_FOUND:
            Serial.println("Tag not found");
            success = false;
            break;

        default:
            Serial.print("Response error: ");Serial.println(cmdResponse.status, HEX);
            success = false;
            break;
    }

    if (success && cmdResponse.command == EM410X_SCAN) {
        lfTagData.size = cmdResponse.dataSize;
        memcpy(lfTagData.uidByte, cmdResponse.data, cmdResponse.dataSize);
    }
    else if (success && cmdResponse.command == HF14A_SCAN) {
        hfTagData.size = cmdResponse.data[0];
        memcpy(hfTagData.uidByte, cmdResponse.data+1, hfTagData.size);

        hfTagData.atqaByte[1] = cmdResponse.data[1 + hfTagData.size];
        hfTagData.atqaByte[0] = cmdResponse.data[2 + hfTagData.size];

        hfTagData.sak = cmdResponse.data[3 + hfTagData.size];
    }

    if (_debug) {
        Serial.print("CMD Response: ");
        for (int i = 0; i < cmdResponse.length; i++) {
            Serial.print(cmdResponse.raw[i] < 0x10 ? " 0" : " ");
            Serial.print(cmdResponse.raw[i], HEX);
        }
        Serial.println();
    }

    chameleonResponses.clear();
    return success;
}


ChameleonUltra::TagType ChameleonUltra::getTagType(byte sak) {
    TagType tagType;

    switch (sak) {
        case 0x00: tagType = MF0ICU1; break;
        case 0x08: tagType = MIFARE_1024; break;
        case 0x09: tagType = MIFARE_Mini; break;
        // case 0x10: tagType = F("MIFARE Plus 2KB"); break;
        // case 0x11: tagType = F("MIFARE Plus 4KB"); break;
        case 0x18: tagType = MIFARE_4096; break;
        case 0x19: tagType = MIFARE_2048; break;
        case 0x20: tagType = ISO_14443; break;
        // case 0x28: tagType = F("SmartMX with MIFARE Classic 1KB"); break;
        // case 0x38: tagType = F("SmartMX with MIFARE Classic 4KB"); break;
        // case 0x40: tagType = F("PICC compliant with ISO/IEC 18092 (NFC)"); break;
        default: tagType = UNDEFINED; break;
    }

    if (tagVersion.size == 8) {
        bool isMikronUlev1 = tagVersion.data[1] == 0x34 && tagVersion.data[2] == 0x21;

        if (
            (tagVersion.data[2] == 3 || isMikronUlev1)
            && tagVersion.data[4] == 1
            && tagVersion.data[5] == 0
        ) {
            switch (tagVersion.data[6]) {
                case 0x0B: tagType = MF0UL11; break;
                case 0x0E: tagType = MF0UL21; break;
            }
        }
        else if (tagVersion.data[2] == 4 && tagVersion.data[4] == 1 && tagVersion.data[5] == 0) {
            switch (tagVersion.data[6]) {
                case 0x0B: tagType = NTAG_210; break;
                case 0x0E: tagType = NTAG_212; break;
                case 0x0F: tagType = NTAG_213; break;
                case 0x11: tagType = NTAG_215; break;
                case 0x13: tagType = NTAG_216; break;
            }
        }
    }

    return tagType;
}


String ChameleonUltra::getTagTypeStr(byte sak) {
    String tagType;

    switch (sak) {
        case 0x00: tagType = F("MIFARE Ultralight"); break;
        case 0x08: tagType = F("MIFARE 1KB"); break;
        case 0x09: tagType = F("MIFARE Mini, 320 bytes"); break;
        case 0x10: tagType = F("MIFARE Plus 2KB"); break;
        case 0x11: tagType = F("MIFARE Plus 4KB"); break;
        case 0x18: tagType = F("MIFARE 4KB"); break;
        case 0x19: tagType = F("MIFARE 2KB"); break;
        case 0x20: tagType = F("PICC compliant with ISO/IEC 14443-4"); break;
        case 0x28: tagType = F("SmartMX with MIFARE Classic 1KB"); break;
        case 0x38: tagType = F("SmartMX with MIFARE Classic 4KB"); break;
        case 0x40: tagType = F("PICC compliant with ISO/IEC 18092 (NFC)"); break;
        default: tagType = F("Unknown type"); break;
    }

    if (tagVersion.size == 8) {
        bool isMikronUlev1 = tagVersion.data[1] == 0x34 && tagVersion.data[2] == 0x21;

        if (
            (tagVersion.data[2] == 3 || isMikronUlev1)
            && tagVersion.data[4] == 1
            && tagVersion.data[5] == 0
        ) {
            switch (tagVersion.data[6]) {
                case 0x0B: tagType = F("Mifare Ultralight EV1 48b"); break;
                case 0x0E: tagType = F("Mifare Ultralight EV1 128b"); break;
            }
        }
        else if (tagVersion.data[2] == 4 && tagVersion.data[4] == 1 && tagVersion.data[5] == 0) {
            switch (tagVersion.data[6]) {
                case 0x0B: tagType = F("NTAG 210"); break;
                case 0x0E: tagType = F("NTAG 212"); break;
                case 0x0F: tagType = F("NTAG 213"); break;
                case 0x11: tagType = F("NTAG 215"); break;
                case 0x13: tagType = F("NTAG 216"); break;
            }
        }
    }

    return tagType;
}


// HW Commands

bool ChameleonUltra::cmdEnableSlot(uint8_t slot, TagSenseType freq) {
    if (slot < 1 || slot > 8) return false;

    Serial.printf("Enable %s on slot %d\n", (freq == RFID_HF ? "HF" : "LF"), slot);

    uint8_t cmd[3] = {slot-1, freq, 0x01};

    return writeCommand(SET_SLOT_ENABLE, cmd, sizeof(cmd));
}


bool ChameleonUltra::cmdChangeActiveSlot(uint8_t slot) {
    if (slot < 1 || slot > 8) return false;

    Serial.printf("Change active slot to %d\n", slot);

    uint8_t cmd[1] = {slot-1};

    return writeCommand(SET_ACTIVE_SLOT, cmd, sizeof(cmd));
}


bool ChameleonUltra::cmdChangeSlotType(uint8_t slot, TagType tagType) {
    if (slot < 1 || slot > 8) return false;

    Serial.printf("Change slot %d type\n", slot);

    uint8_t cmd[3] = {slot-1, (tagType >> 8) & 0xFF, tagType & 0xFF};

    return writeCommand(SET_SLOT_TAG_TYPE, cmd, sizeof(cmd));
}


bool ChameleonUltra::cmdChangeSlotNickName(uint8_t slot, TagSenseType freq, String name) {
    if (slot < 1 || slot > 8) return false;

    Serial.printf(
        "Change slot %d %s nick name to %s\n",
        slot,
        (freq == RFID_HF ? "HF" : "LF"),
        name.c_str()
    );

    size_t name_len = name.length();
    uint8_t cmd[2 + name_len] = {};
    cmd[0] = slot-1;
    cmd[1] = freq;
    name.toCharArray(reinterpret_cast<char*>(cmd+2), sizeof(name));

    return writeCommand(SET_SLOT_TAG_NICK, cmd, sizeof(cmd));
}


bool ChameleonUltra::cmdChangeMode(HwMode mode) {
    Serial.printf("Set %s mode\n", (mode == HW_MODE_READER ? "reader" : "emulator"));

    uint8_t cmd[1] = {mode};

    return writeCommand(CHANGE_DEVICE_MODE, cmd, sizeof(cmd));
}


bool ChameleonUltra::cmdBatteryInfo() {
    Serial.println("Battery Info");

    return writeCommand(GET_BATTERY_INFO);
}


bool ChameleonUltra::cmdFactoryReset() {
    Serial.println("Factory Reset");

    return writeCommand(WIPE_FDS);
}


// LF Commands

bool ChameleonUltra::cmdLFRead() {
    Serial.println("Read LF");

    return writeCommand(EM410X_SCAN);
}


bool ChameleonUltra::cmdLFWrite(byte *uid, size_t length) {
    Serial.println("Write LF");

    if (length != 5) return false;

    uint8_t cmd[17] = {
        0x00, 0x00, 0x00, 0x00, 0x00,
        0x20, 0x20, 0x66, 0x66, 0x51, 0x24, 0x36, 0x48, 0x19, 0x92, 0x04, 0x27
    };
    memcpy(cmd, uid, length);

    return writeCommand(EM410X_WRITE_TO_T55XX, cmd, sizeof(cmd));
}


bool ChameleonUltra::cmdLFEconfig(byte *uid, size_t length) {
    Serial.println("Set LF emulation config");

    if (length != 5) return false;

    uint8_t cmd[5] = {};
    memcpy(cmd, uid, length);

    return writeCommand(EM410X_SET_EMU_ID, cmd, sizeof(cmd));
}


// HF Commands

bool ChameleonUltra::cmd14aScan() {
    Serial.println("Scan 14a tags");

    return writeCommand(HF14A_SCAN);
}


bool ChameleonUltra::cmd14aRaw(RawOptions options, uint8_t timeout, uint8_t *data, size_t length, uint8_t bitlen) {
    Serial.println("14a raw");

    uint8_t optByte = (
        options.activateRfField << 7
        | options.waitResponse << 6
        | options.appendCrc << 5
        | options.autoSelect << 4
        | options.keepRfField << 3
        | options.checkResponseCrc << 2
        | 0 << 1
        | 0
    );


    if (bitlen == 0) bitlen = length * 8;
    else {
        if (length == 0) {
            Serial.println("bitlen=" + String(bitlen)+ " but missing data");
            return false;
        }
        if (bitlen <= (length - 1) * 8 || bitlen > length * 8) {
            Serial.println("bitlen=" + String(bitlen)+ " incompatible with provided data length=" + String(length));
            return false;
        }
    }


    uint8_t cmd[length + 5] = {optByte, 0x00, timeout, 0x00, bitlen};
    if (length > 0) memcpy(cmd+5, data, length);

    return writeCommand(HF14A_RAW, cmd, sizeof(cmd));
}


bool ChameleonUltra::cmdMfuVersion() {
    Serial.println("Get Ultralight tag version");

    ChameleonUltra::RawOptions opt;
    opt.waitResponse = true;
    opt.appendCrc = true;
    opt.autoSelect = true;
    opt.checkResponseCrc = true;

    uint8_t cmd[1] = {0x60};

    if (cmd14aRaw(opt, 200, cmd, sizeof(cmd))) {
        tagVersion.size = cmdResponse.dataSize;
        memcpy(tagVersion.data, cmdResponse.data, cmdResponse.dataSize);
        return true;
    }

    return false;
}


bool ChameleonUltra::cmdMfuReadPage(uint8_t page) {
    Serial.println("Read Ultralight page " + String(page));

    ChameleonUltra::RawOptions opt;
    opt.waitResponse = true;
    opt.appendCrc = true;
    opt.autoSelect = true;
    opt.checkResponseCrc = true;

    uint8_t cmd[2] = {0x30, page};

    return cmd14aRaw(opt, 200, cmd, sizeof(cmd));
}


bool ChameleonUltra::cmdMfuWritePage(uint8_t page, uint8_t *data, size_t length) {
    if (length == 0) return false;

    Serial.println("Write Ultralight page " + String(page));

    // uint8_t cmd[length + 7] = {0x70, 0x00, 0xc8, 0x00, 0x30, 0xa2, page};
    // memcpy(cmd+7, data, length);

    ChameleonUltra::RawOptions opt;
    opt.waitResponse = true;
    opt.appendCrc = true;
    opt.autoSelect = true;

    uint8_t cmd[length+2] = {0xa2, page};
    memcpy(cmd+2, data, length);

    return cmd14aRaw(opt, 200, cmd, sizeof(cmd));
}


bool ChameleonUltra::cmdMfReadBlock(uint8_t block, uint8_t *key) {
    Serial.println("Read Mifare block " + String(block));

    uint8_t cmd[8] = {0x60, block};
    if (sizeof(key) >= 6) memcpy(cmd+2, key, 6);
    else memcpy(cmd+2, mifareKey, 6);

    return writeCommand(MF1_READ_ONE_BLOCK, cmd, sizeof(cmd));
}


bool ChameleonUltra::cmdMfWriteBlock(uint8_t block, uint8_t *key, uint8_t *data, size_t length) {
    if (length != 16) return false;

    Serial.println("Write Mifare block " + String(block));

    uint8_t cmd[length+8] = {0x60, block};
    if (sizeof(key) >= 6) memcpy(cmd+2, key, 6);
    else memcpy(cmd+2, mifareKey, 6);

    memcpy(cmd+8, data, length);

    return writeCommand(MF1_WRITE_ONE_BLOCK, cmd, sizeof(cmd));
}


bool ChameleonUltra::cmdMfEload(String dumpData) {
    Serial.println("Upload dump data");

    uint8_t cmd[MAX_DUMP_SIZE+5] = {};

    int index = 0;
    int block = 0;
    for (size_t i = 0; i < dumpData.length(); i += 2) {
        cmd[1 + index++] = strtoul(dumpData.substring(i, i + 2).c_str(), NULL, 16);

        if (index == MAX_DUMP_SIZE || i+2 ==  dumpData.length()) {
            cmd[0] = block;

            if (!writeCommand(MF1_WRITE_EMU_BLOCK_DATA, cmd, index+1)) return false;

            block += index / 16;
            index = 0;
        }
    }

    return true;
}


bool ChameleonUltra::cmdMfEconfig(byte *uid, size_t length, byte *atqa, byte sak) {
    if ((length != 4 && length != 7) || sizeof(atqa) < 4) return false;

    Serial.println("Set HF emulation config");

    uint8_t cmd[length + 5] = {length};
    memcpy(cmd+1, uid, length);

    int index = length + 1;
    cmd[index++] = atqa[1];
    cmd[index++] = atqa[0];
    cmd[index++] = sak;
    cmd[index++] = 0x00;  // ats

    return writeCommand(HF14A_SET_ANTI_COLL_DATA, cmd, length + 5);
}


bool ChameleonUltra::cmdMfHalt() {
    Serial.println("HALT and close RF field");

    ChameleonUltra::RawOptions opt;
    opt.appendCrc = true;

    uint8_t cmd[2] = {0x50, 0x00};

    return cmd14aRaw(opt, 1, cmd, sizeof(cmd));
}


bool ChameleonUltra::cmdMfGen1aAuth() {
    Serial.println("Mifare Gen1a Auth");

    ChameleonUltra::RawOptions opt;
    opt.keepRfField = true;
    opt.waitResponse = true;

    uint8_t cmd[1] = {0x40};

    if (!cmd14aRaw(opt, 1, cmd, sizeof(cmd), 0x07)) return false;
    if (cmdResponse.dataSize < 1 || cmdResponse.data[0] != 0x0A) return false;

    uint8_t cmd2[1] = {0x43};

    if (!cmd14aRaw(opt, 1, cmd2, sizeof(cmd2))) return false;
    if (cmdResponse.dataSize < 1 || cmdResponse.data[0] != 0x0A) return false;

    return true;
}


bool ChameleonUltra::cmdMfGen1aWriteBlock(uint8_t block, uint8_t *data, size_t length) {
    if (length != 16) return false;

    Serial.println("Write Mifare Gen1a block " + String(block));

    ChameleonUltra::RawOptions opt;
    opt.appendCrc = true;
    opt.keepRfField = true;
    opt.waitResponse = true;

    uint8_t cmd[2] = {0xA0, block};

    if (!cmd14aRaw(opt, 1, cmd, sizeof(cmd))) return false;
    if (cmdResponse.dataSize < 1 || cmdResponse.data[0] != 0x0A) return false;

    if (!cmd14aRaw(opt, 1, data, length)) return false;
    if (cmdResponse.dataSize < 1 || cmdResponse.data[0] != 0x0A) return false;

    return true;
}


bool ChameleonUltra::cmdMfGen1aReadBlock(uint8_t block) {
    Serial.println("Read Mifare Gen1a block " + String(block));

    ChameleonUltra::RawOptions opt;
    opt.appendCrc = true;
    opt.checkResponseCrc = true;
    opt.keepRfField = true;
    opt.waitResponse = true;

    uint8_t cmd[2] = {0x30, block};

    return cmd14aRaw(opt, 1, cmd, sizeof(cmd));
}


bool ChameleonUltra::cmdMfSetUid(byte *uid, size_t length) {
    if (!cmdMfReadBlock(0, mifareKey)) return false;

    uint8_t blockData[cmdResponse.dataSize];
    memcpy(blockData, cmdResponse.data, cmdResponse.dataSize);
    memcpy(blockData, uid, length);

    byte bcc = 0;
    for (byte i = 0; i < length; i++) bcc = bcc ^ uid[i];
    blockData[length] = bcc;

    cmdMfHalt();

    bool success = cmdMfGen1aAuth() && cmdMfGen1aWriteBlock(0, blockData, sizeof(blockData));

    cmdMfHalt();

    return success;
}
