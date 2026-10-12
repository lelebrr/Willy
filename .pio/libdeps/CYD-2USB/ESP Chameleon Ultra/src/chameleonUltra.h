/**
 * @file chameleonUltra.h
 * @author Rennan Cockles (https://github.com/rennancockles)
 * @brief ESP Chameleon Ultra
 * @version 0.1
 * @date 2024-10-09
 */


#ifndef __CHAMELEON_ULTRA_H__
#define __CHAMELEON_ULTRA_H__

#include <Arduino.h>
#include <algorithm>
#include <NimBLEDevice.h>

#if __has_include(<NimBLEExtAdvertising.h>)
#define NIMBLE_V2_PLUS 1
#include <NimBLEAdvertising.h>
#include <NimBLEServer.h>
#endif

class ChameleonUltra {
public:
    enum Command {
        GET_APP_VERSION = 1000,
        CHANGE_DEVICE_MODE = 1001,
        GET_DEVICE_MODE = 1002,
        SET_ACTIVE_SLOT = 1003,
        SET_SLOT_TAG_TYPE = 1004,
        SET_SLOT_DATA_DEFAULT = 1005,
        SET_SLOT_ENABLE = 1006,

        SET_SLOT_TAG_NICK = 1007,
        GET_SLOT_TAG_NICK = 1008,

        SLOT_DATA_CONFIG_SAVE = 1009,

        ENTER_BOOTLOADER = 1010,
        GET_DEVICE_CHIP_ID = 1011,
        GET_DEVICE_ADDRESS = 1012,

        SAVE_SETTINGS = 1013,
        RESET_SETTINGS = 1014,
        SET_ANIMATION_MODE = 1015,
        GET_ANIMATION_MODE = 1016,

        GET_GIT_VERSION = 1017,

        GET_ACTIVE_SLOT = 1018,
        GET_SLOT_INFO = 1019,

        WIPE_FDS = 1020,

        DELETE_SLOT_TAG_NICK = 1021,

        GET_ENABLED_SLOTS = 1023,
        DELETE_SLOT_SENSE_TYPE = 1024,

        GET_BATTERY_INFO = 1025,

        GET_BUTTON_PRESS_CONFIG = 1026,
        SET_BUTTON_PRESS_CONFIG = 1027,

        GET_LONG_BUTTON_PRESS_CONFIG = 1028,
        SET_LONG_BUTTON_PRESS_CONFIG = 1029,

        SET_BLE_PAIRING_KEY = 1030,
        GET_BLE_PAIRING_KEY = 1031,
        DELETE_ALL_BLE_BONDS = 1032,

        GET_DEVICE_MODEL = 1033,
        // FIXME: implemented but unused in CLI commands
        GET_DEVICE_SETTINGS = 1034,
        GET_DEVICE_CAPABILITIES = 1035,
        GET_BLE_PAIRING_ENABLE = 1036,
        SET_BLE_PAIRING_ENABLE = 1037,

        HF14A_SCAN = 2000,
        MF1_DETECT_SUPPORT = 2001,
        MF1_DETECT_PRNG = 2002,
        MF1_STATIC_NESTED_ACQUIRE = 2003,
        MF1_DARKSIDE_ACQUIRE = 2004,
        MF1_DETECT_NT_DIST = 2005,
        MF1_NESTED_ACQUIRE = 2006,
        MF1_AUTH_ONE_KEY_BLOCK = 2007,
        MF1_READ_ONE_BLOCK = 2008,
        MF1_WRITE_ONE_BLOCK = 2009,
        HF14A_RAW = 2010,
        MF1_MANIPULATE_VALUE_BLOCK = 2011,
        MF1_CHECK_KEYS_OF_SECTORS = 2012,

        EM410X_SCAN = 3000,
        EM410X_WRITE_TO_T55XX = 3001,

        MF1_WRITE_EMU_BLOCK_DATA = 4000,
        HF14A_SET_ANTI_COLL_DATA = 4001,
        MF1_SET_DETECTION_ENABLE = 4004,
        MF1_GET_DETECTION_COUNT = 4005,
        MF1_GET_DETECTION_LOG = 4006,
        // FIXME: not implemented
        MF1_GET_DETECTION_ENABLE = 4007,
        MF1_READ_EMU_BLOCK_DATA = 4008,
        MF1_GET_EMULATOR_CONFIG = 4009,
        // FIXME: not implemented
        MF1_GET_GEN1A_MODE = 4010,
        MF1_SET_GEN1A_MODE = 4011,
        // FIXME: not implemented
        MF1_GET_GEN2_MODE = 4012,
        MF1_SET_GEN2_MODE = 4013,
        // FIXME: not implemented
        MF1_GET_BLOCK_ANTI_COLL_MODE = 4014,
        MF1_SET_BLOCK_ANTI_COLL_MODE = 4015,
        // FIXME: not implemented
        MF1_GET_WRITE_MODE = 4016,
        MF1_SET_WRITE_MODE = 4017,
        HF14A_GET_ANTI_COLL_DATA = 4018,
        MF0_NTAG_GET_UID_MAGIC_MODE = 4019,
        MF0_NTAG_SET_UID_MAGIC_MODE = 4020,
        MF0_NTAG_READ_EMU_PAGE_DATA = 4021,
        MF0_NTAG_WRITE_EMU_PAGE_DATA = 4022,
        MF0_NTAG_GET_VERSION_DATA = 4023,
        MF0_NTAG_SET_VERSION_DATA = 4024,
        MF0_NTAG_GET_SIGNATURE_DATA = 4025,
        MF0_NTAG_SET_SIGNATURE_DATA = 4026,
        MF0_NTAG_GET_COUNTER_DATA = 4027,
        MF0_NTAG_SET_COUNTER_DATA = 4028,
        MF0_NTAG_RESET_AUTH_CNT = 4029,
        MF0_NTAG_GET_PAGE_COUNT = 4030,

        EM410X_SET_EMU_ID = 5000,
        EM410X_GET_EMU_ID = 5001,
    };

    enum TagSenseType {
        RFID_UNDEFINED = 0x00,
        RFID_LF = 0x01,
        RFID_HF = 0x02,
    };

    enum HwMode {
        HW_MODE_EMULATOR = 0x00,
        HW_MODE_READER = 0x01,
    };

    enum TagType {
        UNDEFINED = 0,

        // LF
        EM410X = 100,
        TAG_TYPES_LF_END = 999,

        // HF
        // MIFARE Classic series  1000
        MIFARE_Mini = 1000,
        MIFARE_1024 = 1001,
        MIFARE_2048 = 1002,
        MIFARE_4096 = 1003,
        // MFUL / NTAG series     1100
        NTAG_213 = 1100,
        NTAG_215 = 1101,
        NTAG_216 = 1102,
        MF0ICU1 = 1103,
        MF0ICU2 = 1104,
        MF0UL11 = 1105,
        MF0UL21 = 1106,
        NTAG_210 = 1107,
        NTAG_212 = 1108,

        ISO_14443 = 1200,
    };

    enum RspStatus {
        HF_TAG_OK = 0x00,     // IC card operation is successful
        HF_TAG_NO = 0x01,     // IC card not found
        HF_ERR_STAT = 0x02,   // Abnormal IC card communication
        HF_ERR_CRC = 0x03,    // IC card communication verification abnormal
        HF_COLLISION = 0x04,  // IC card conflict
        HF_ERR_BCC = 0x05,    // IC card BCC error
        MF_ERR_AUTH = 0x06,   // MF card verification failed
        HF_ERR_PARITY = 0x07, // IC card parity error
        HF_ERR_ATS = 0x08,    // ATS should be present but card NAKed, or ATS too large

        // Some operations with low frequency cards succeeded!
        LF_TAG_OK = 0x40,
        // Unable to search for a valid EM410X label
        EM410X_TAG_NO_FOUND = 0x41,

        // The parameters passed by the BLE instruction are wrong,
        // or the parameters passed by calling some functions are wrong
        PAR_ERR = 0x60,
        // The mode of the current device is wrong, and the corresponding
        // API cannot be called
        DEVICE_MODE_ERROR = 0x66,
        INVALID_CMD = 0x67,
        SUCCESS = 0x68,
        NOT_IMPLEMENTED = 0x69,
        FLASH_WRITE_FAIL = 0x70,
        FLASH_READ_FAIL = 0x71,
        INVALID_SLOT_TYPE = 0x72,
    };

    typedef struct {
        bool activateRfField = false;
        bool waitResponse = false;
        bool appendCrc = false;
        bool autoSelect = false;
        bool keepRfField = false;
        bool checkResponseCrc = false;
    } RawOptions;

    typedef struct {
        byte size;
        byte uidByte[10];
    } LfTag;

    typedef struct {
        byte size;
        byte data[10];
    } TagVersion;

    typedef struct {
        byte size;
        byte uidByte[10];
        byte sak;
        byte atqaByte[2];
    } HfTag;

    typedef struct {
        uint8_t raw[250];
        size_t length;
        uint16_t command;
        uint8_t status;
        uint8_t dataSize;
        uint8_t data[200];

    } CmdResponse;

    LfTag lfTagData;
    HfTag hfTagData;
    TagVersion tagVersion;
    CmdResponse cmdResponse;
    uint8_t mifareDefaultKey[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    uint8_t mifareKey[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};


    /////////////////////////////////////////////////////////////////////////////////////
    // Constructor
    /////////////////////////////////////////////////////////////////////////////////////
    ChameleonUltra(bool debug = false);
    ~ChameleonUltra();

    /////////////////////////////////////////////////////////////////////////////////////
    // Connection
    /////////////////////////////////////////////////////////////////////////////////////
    bool searchChameleonDevice();
    bool connectToChamelon();
    bool chamelonServiceDiscovery();

    /////////////////////////////////////////////////////////////////////////////////////
    // Commands
    /////////////////////////////////////////////////////////////////////////////////////
    TagType getTagType(byte sak);
    String getTagTypeStr(byte sak);

    // HW Commands
    //   > hw slot enable -s <1-8> (--hf | --lf)
    bool cmdEnableSlot(uint8_t slot, TagSenseType freq);
    //   > hw slot change -s <1-8>
    bool cmdChangeActiveSlot(uint8_t slot);
    //   > hw slot type -s <1-8> -t TAG_TYPE
    bool cmdChangeSlotType(uint8_t slot, TagType tagType);
    //   > hw slot nick -s <1-8> (--hf | --lf) -n NAME
    bool cmdChangeSlotNickName(uint8_t slot, TagSenseType freq, String name);
    //   > hw mode [-r | -e]
    bool cmdChangeMode(HwMode mode);
    //   > hw battery
    bool cmdBatteryInfo();
    //   > hw factory_reset --force
    bool cmdFactoryReset();

    // LF Commands
    //   > lf em 410x read
    bool cmdLFRead();
    //   > lf em 410x write --id <hex>
    bool cmdLFWrite(byte *uid, size_t length);
    //   > lf em 410x econfig -s <1-8> --id <hex>
    bool cmdLFEconfig(byte *uid, size_t length);

    // HF Commands
    //   > hf 14a scan
    bool cmd14aScan();
    //   > hf 14a raw [-a] [-s] [-d <hex>] [-b <dec>] [-c] [-r] [-cc] [-k] [-t <dec>]
    bool cmd14aRaw(RawOptions options, uint8_t timeout = 100, uint8_t *data = nullptr, size_t length = 0, uint8_t bitlen = 0);

    //   > hf mfu version
    bool cmdMfuVersion();
    //   > hf mfu rdpg -p <dec>
    bool cmdMfuReadPage(uint8_t page);
    //   > hf mfu wrpg -p <dec> -d <hex>
    bool cmdMfuWritePage(uint8_t page, uint8_t *data, size_t length);

    //   > hf mf rdbl --blk <dec> -k <hex>
    bool cmdMfReadBlock(uint8_t block, uint8_t *key);
    //   > hf mf wrbl --blk <dec> -k <hex> -d <hex>
    bool cmdMfWriteBlock(uint8_t block, uint8_t *key, uint8_t *data, size_t length);
    //   > hf mf eload -s <1-8> -f FILE [-t {bin,hex}]
    bool cmdMfEload(String dumpData);
    //   > hf mf econfig -s <1-8> [--uid <hex>] [--atqa <hex>] [--sak <hex>]
    bool cmdMfEconfig(byte *uid, size_t length, byte *atqa, byte sak);

    //   > hf 14a raw -c -d 5000
    bool cmdMfHalt();
    //   > hf 14a raw -k -d 40 -b 7
    //   > hf 14a raw -k -d 43
    bool cmdMfGen1aAuth();
    //   > hf 14a raw -k -c -cc -d 30<hex>
    bool cmdMfGen1aReadBlock(uint8_t block);
    //   > hf 14a raw -k -c -d <hex>
    bool cmdMfGen1aWriteBlock(uint8_t block, uint8_t *data, size_t length);
    bool cmdMfSetUid(byte *uid, size_t length);



private:
    NimBLEUUID serviceUUID = NimBLEUUID("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
    NimBLEUUID chrTxUUID = NimBLEUUID("6E400002-B5A3-F393-E0A9-E50E24DCCA9E");
    NimBLEUUID chrRxUUID = NimBLEUUID("6E400003-B5A3-F393-E0A9-E50E24DCCA9E");

    NimBLERemoteCharacteristic* writeChr;
    #ifdef NIMBLE_V2_PLUS
    NimBLEAdvertisedDevice *_device = nullptr;
    #else
    NimBLEAdvertisedDevice _device;
    #endif

    bool _debug = false;


    /////////////////////////////////////////////////////////////////////////////////////
    // Communication
    /////////////////////////////////////////////////////////////////////////////////////
    bool writeCommand(Command cmd, uint8_t *data = nullptr, size_t length = 0);
    bool checkResponse();

};

#endif
