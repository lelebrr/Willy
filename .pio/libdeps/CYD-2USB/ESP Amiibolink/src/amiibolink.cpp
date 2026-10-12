/**
 * @file amiibolink.cpp
 * @author Rennan Cockles (https://github.com/rennancockles)
 * @brief ESP Amiibolink
 * @version 0.1
 * @date 2024-10-11
 */

#include "amiibolink.h"

#define CMD_DELAY 500
#define MAX_DUMP_SIZE 20


std::vector<Amiibolink::CmdResponse> amiibolinkResponses;

#ifdef NIMBLE_V2_PLUS
#define NimBLEAdvertisedDeviceCallbacks NimBLEScanCallbacks
#endif

class scanCallbacks : public NimBLEAdvertisedDeviceCallbacks {
    void onResult(NimBLEAdvertisedDevice* advertisedDevice) {
        if (advertisedDevice->getName() == "amiibolink") {
            NimBLEDevice::getScan()->stop();
        }
    }
};


void amiibolinkNotifyCB(NimBLERemoteCharacteristic* pRemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify){
    Amiibolink::CmdResponse rsp;

    rsp.length = length;
    memcpy(rsp.raw, pData, length);

    // rsp.command = (pData[2] << 8) | pData[3];
    // rsp.status = pData[5];
    // rsp.dataSize = pData[7];

    // if (rsp.dataSize > 0) {
    //     memcpy(rsp.data, pData+9, rsp.dataSize);
    // }

    amiibolinkResponses.push_back(rsp);
}


Amiibolink::Amiibolink(bool debug) { _debug = debug;}


Amiibolink::~Amiibolink() {
    if (_debug) Serial.println("Killing Amiibolink...");
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


bool Amiibolink::searchDevice() {
    NimBLEDevice::init("");

    NimBLEScan* pScan = NimBLEDevice::getScan();
    #ifdef NIMBLE_V2_PLUS
    pScan->setScanCallbacks(new scanCallbacks());
    BLEScanResults foundDevices = pScan->getResults(150);
    bool deviceFound = false;

    for (int i=0; i<foundDevices.getCount(); i++) {
        const NimBLEAdvertisedDevice* advertisedDevice = foundDevices.getDevice(i);

        if (advertisedDevice->getName() == "amiibolink") {
            deviceFound = true;
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
    bool deviceFound = false;

    for (int i=0; i<foundDevices.getCount(); i++) {
        NimBLEAdvertisedDevice advertisedDevice = foundDevices.getDevice(i);

        if (advertisedDevice.getName() == "amiibolink") {
            deviceFound = true;
            _device = advertisedDevice;
        }
    }
    #endif

    pScan->clearResults();

    return deviceFound;
}


bool Amiibolink::connectToDevice() {
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
    pChrNotify->subscribe(true, amiibolinkNotifyCB);

    return true;
}


bool Amiibolink::serviceDiscovery() {
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


bool Amiibolink::cmdSetUIDMode(UIDMode mode) {
    if (mode == UIDMode_Auto) return cmdSetUIDModeAuto();
    if (mode == UIDMode_Manual) return cmdSetUIDModeManual();
    return false;
}


bool Amiibolink::cmdSetUIDModeAuto() {
    Serial.println("Set UID Mode Auto");

    uint8_t cmd[20] = {
        0x00, 0x00, 0x10, 0x03,
        0xE3, 0x96, 0x51, 0xEC, 0x07, 0xE7, 0xE5, 0x54,
        0x37, 0xB6, 0x13, 0x8E, 0x80, 0xC9, 0xB3, 0x09
    };

    return writeCommand(cmd, sizeof(cmd));
}


bool Amiibolink::cmdSetUIDModeManual() {
    Serial.println("Set UID Mode Manual");

    uint8_t cmd[20] = {
        0x00, 0x00, 0x10, 0x03,
        0x34, 0x1F, 0x98, 0xE8, 0x46, 0x19, 0x85, 0x75,
        0xE3, 0xD3, 0xE0, 0x42, 0x5D, 0x41, 0x89, 0x42
    };

    return writeCommand(cmd, sizeof(cmd));
}


bool Amiibolink::cmdPreUploadDump() {
    Serial.println("Pre upload dump commands");

    uint8_t cmd0[2] = {0xA0, 0xB0};
    if (!writeCommand(cmd0, sizeof(cmd0))) return false;

    uint8_t cmd1[8] = {0xAC, 0xAC, 0x00, 0x04, 0x00, 0x00, 0x02, 0x1C};
    if (!writeCommand(cmd1, sizeof(cmd1))) return false;

    uint8_t cmd2[4] = {0xAB, 0xAB, 0x02, 0x1C};
    if (!writeCommand(cmd2, sizeof(cmd2))) return false;

    return true;
}


bool Amiibolink::cmdPostUploadDump() {
    Serial.println("Post upload dump commands");

    uint8_t cmd0[2] = {0xBC, 0xBC};
    if (!writeCommand(cmd0, sizeof(cmd0))) return false;

    uint8_t cmd1[2] = {0xCC, 0xDD};
    if (!writeCommand(cmd1, sizeof(cmd1))) return false;

    return true;
}


bool Amiibolink::cmdUploadDumpData(String dumpData) {
    Serial.println("Upload dump data");
    uint8_t cmd[26] = {0xDD, 0xAA, 0x00, 0x14};

    int index = 0;
    int block = 0;
    for (size_t i = 0; i < dumpData.length(); i += 2) {
        cmd[4 + index++] = strtoul(dumpData.substring(i, i + 2).c_str(), NULL, 16);

        if (index == MAX_DUMP_SIZE || i+2 == dumpData.length()) {
            block = (i/2) / 20 + 1;

            cmd[4 + index++] = 0x00;
            cmd[4 + index++] = block;

            if (!writeCommand(cmd, sizeof(cmd))) return false;

            index = 0;
        }
    }

    return true;
}


bool Amiibolink::writeCommand(uint8_t *data, size_t length) {
    if (_debug) {
        Serial.print("Cmd:");
        for (int i=0; i<length; i++) {
            Serial.print(data[i] < 0x10 ? " 0" : " ");
            Serial.print(data[i], HEX);
        }
        Serial.println("");
    }

    bool writeRes = writeChr->writeValue(data, length, true);

    delay(CMD_DELAY);

    bool res = checkResponse();

    return writeRes && res;
}


bool Amiibolink::checkResponse() {
    while(amiibolinkResponses.empty()) {delay(10);}

    cmdResponse = amiibolinkResponses[0];

    if (_debug) {
        Serial.print("CMD Response: ");
        for (int i = 0; i < cmdResponse.length; i++) {
            Serial.print(cmdResponse.raw[i] < 0x10 ? " 0" : " ");
            Serial.print(cmdResponse.raw[i], HEX);
        }
        Serial.println();
    }

    amiibolinkResponses.clear();
    return true;
}
