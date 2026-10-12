/**
 * @file amiibolink.h
 * @author Rennan Cockles (https://github.com/rennancockles)
 * @brief ESP Amiibolink
 * @version 0.1
 * @date 2024-10-11
 */

#ifndef __AMIIBOLINK_H__
#define __AMIIBOLINK_H__

#include <Arduino.h>
#include <algorithm>
#include <NimBLEDevice.h>

#if __has_include(<NimBLEExtAdvertising.h>)
#define NIMBLE_V2_PLUS 1
#include <NimBLEAdvertising.h>
#include <NimBLEServer.h>
#endif


class Amiibolink {
public:
    enum UIDMode {
        UIDMode_Auto,
        UIDMode_Manual,
    };

    typedef struct {
        uint8_t raw[250];
        size_t length;
        uint16_t command;
        uint8_t status;
        uint8_t dataSize;
        uint8_t data[200];

    } CmdResponse;
    CmdResponse cmdResponse;

    /////////////////////////////////////////////////////////////////////////////////////
    // Constructor
    /////////////////////////////////////////////////////////////////////////////////////
    Amiibolink(bool debug = false);
    ~Amiibolink();

    /////////////////////////////////////////////////////////////////////////////////////
    // Connection
    /////////////////////////////////////////////////////////////////////////////////////
    bool searchDevice();
    bool connectToDevice();
    bool serviceDiscovery();

    /////////////////////////////////////////////////////////////////////////////////////
    // Commands
    /////////////////////////////////////////////////////////////////////////////////////
    bool cmdSetUIDMode(UIDMode mode);
    bool cmdSetUIDModeAuto();
    bool cmdSetUIDModeManual();
    bool cmdPreUploadDump();
    bool cmdUploadDumpData(String dumpData);
    bool cmdPostUploadDump();

private:
    NimBLERemoteCharacteristic* writeChr;
    #ifdef NIMBLE_V2_PLUS
    NimBLEAdvertisedDevice *_device = nullptr;
    #else
    NimBLEAdvertisedDevice _device;
    #endif

    NimBLEUUID serviceUUID = NimBLEUUID("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
    NimBLEUUID chrTxUUID = NimBLEUUID("6E400002-B5A3-F393-E0A9-E50E24DCCA9E");
    NimBLEUUID chrRxUUID = NimBLEUUID("6E400003-B5A3-F393-E0A9-E50E24DCCA9E");

    bool _debug = false;

    bool writeCommand(uint8_t *data, size_t length);
    bool checkResponse();
};

#endif
