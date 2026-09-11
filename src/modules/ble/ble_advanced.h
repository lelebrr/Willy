/**
 * @file ble_advanced.h
 * @brief Willy BLE Advanced Suite - 50+ Bluetooth Functions
 * @author Willy Team
 * @date 2026
 *
 * Comprehensive BLE toolkit for security research:
 * - Reconnaissance & Scanning (10 functions)
 * - Attacks & Exploits (15 functions)
 * - Utilities & Tools (10 functions)
 * - Device-Specific Exploits (10 functions)
 * - Monitoring & Detection (8 functions)
 */

#ifndef __BLE_ADVANCED_H__
#define __BLE_ADVANCED_H__

#include <Arduino.h>
#include <NimBLEDevice.h>
#include <NimBLEScan.h>
#include <vector>
#include <globals.h>

//================================================================================
// STRUCTURES & TYPES
//================================================================================

struct BLEScanResult {
    String name;
    String address;
    int rssi;
    uint16_t appearance;
    bool connectable;
    uint32_t timestamp;
    std::vector<NimBLEUUID> services;
    std::vector<uint8_t> manufacturerData;
    String localName;
};

struct BLETrackEntry {
    String address;
    String name;
    int lastRSSI;
    uint32_t firstSeen;
    uint32_t lastSeen;
    uint32_t hitCount;
    float avgRSSI;
};

struct BLEPacketLog {
    uint32_t timestamp;
    int rssi;
    uint8_t type;
    std::vector<uint8_t> data;
    String sourceAddr;
};

struct BLESignalSample {
    uint32_t timestamp;
    int rssi;
};

struct BLEVulnCheck {
    String name;
    String description;
    bool vulnerable;
    String details;
};

enum BLESimpleFilterType {
    FILTER_NAME = 0,
    FILTER_RSSI,
    FILTER_SERVICE,
    FILTER_MANUFACTURER,
    FILTER_APPEARANCE,
    FILTER_CONNECTABLE
};

enum BLEAttackMode {
    ATTACK_RELAY = 0,
    ATTACK_FLOOD_EATT,
    ATTACK_CONN_FLOOD,
    ATTACK_ADV_FLOOD,
    ATTACK_PARAM_DRAIN,
    ATTACK_FIRMWARE_CRASH,
    ATTACK_CHAR_OVERFLOW,
    ATTACK_PAIRING_BRUTE,
    ATTACK_PAIRING_SCREAM,
    ATTACK_REPLAY,
    ATTACK_MITM_PAIR,
    ATTACK_JAMMER
};

//================================================================================
// 1. RECONNAISSANCE & SCANNING (10 functions)
//================================================================================

// 1.1 - Scan with filter by name pattern
void bleScanFilterByName();

// 1.2 - Scan with filter by RSSI range
void bleScanFilterByRSSI();

// 1.3 - Scan with filter by service UUID
void bleScanFilterByService();

// 1.4 - Scan with filter by manufacturer data
void bleScanFilterByManufacturer();

// 1.5 - Export scan results to CSV on SD card
void bleScanExportCSV();

// 1.6 - Continuous scanning with auto-save
void bleScanContinuous();

// 1.7 - Compare RSSI between multiple devices
void bleScanCompareRSSI();

// 1.8 - Track device movement over time
void bleDeviceTracker();

// 1.9 - Scan for hidden/non-broadcasting devices
void bleScanHiddenDevices();

// 1.10 - Watchdog: detect new devices in range
void bleScanWatchdog();

//================================================================================
// 2. ATTACKS & EXPLOITS (15 functions)
//================================================================================

// 2.1 - BLE Relay Attack (forward data between two devices)
void bleRelayAttack();

// 2.2 - EATT Channel Flood (exhaust transport channels)
void bleEATTFlood();

// 2.3 - Connection Flood DoS
void bleConnectionFloodDoS();

// 2.4 - Advertising Flood DoS
void bleAdvertisingFloodDoS();

// 2.5 - Connection Parameter Abuse (battery drain)
void bleParamAbuseDrain();

// 2.6 - Firmware Crash via malformed packets
void bleFirmwareCrash();

// 2.7 - Characteristic Overflow Attack
void bleCharOverflow();

// 2.8 - PIN Brute Force (numeric pairing)
void blePINBruteForce();

// 2.9 - Pairing Request Storm
void blePairingStorm();

// 2.10 - Replay Attack (capture & replay)
void bleReplayAttack();

// 2.11 - MITM Pairing Interceptor
void bleMITMPair();

// 2.12 - BLE Jammer (all channels)
void bleJammerAll();

// 2.13 - GATT Service Confusion
void bleGATTConfusion();

// 2.14 - L2CAP Channel Flood
void bleL2CAPFlood();

// 2.15 - Cross-Protocol Interference
void bleCrossProtocol();

//================================================================================
// 3. UTILITIES & TOOLS (10 functions)
//================================================================================

// 3.1 - BLE Packet Crafter (custom advertising)
void blePacketCrafter();

// 3.2 - Raw Advertising Injection
void bleRawAdvertising();

// 3.3 - Manufacturer Data Decoder
void bleManufacturerDecoder();

// 3.4 - Signal Strength Monitor (real-time graph)
void bleSignalMonitor();

// 3.5 - MAC Address Randomizer
void bleMACRandomizer();

// 3.6 - BLE Badge Creator (custom name + icon)
void bleBadgeCreator();

// 3.7 - Device Clone (clone advertising data)
void bleDeviceClone();

// 3.8 - Distance Estimator (RSSI-based)
void bleDistanceEstimator();

// 3.9 - Packet Logger (save to SD)
void blePacketLogger();

// 3.10 - Firmware Version Detector
void bleFirmwareDetector();

//================================================================================
// 4. DEVICE-SPECIFIC EXPLOITS (10 functions)
//================================================================================

// 4.1 - AirPods Battery Drain
void bleAirPodsDrain();

// 4.2 - AirPods Firmware Crash
void bleAirPodsCrash();

// 4.3 - Tile Tracker Track Manipulation
void bleTileManipulate();

// 4.4 - Fitbit Command Injection
void bleFitbitInject();

// 4.5 - Smart Lock PIN Bypass
void bleSmartLockBypass();

// 4.6 - BLE Keyboard Keystroke Injection
void bleKeyboardInject();

// 4.7 - Heart Rate Monitor Data Spoof
void bleHRMSpoof();

// 4.8 - ESP32 Crash Exploit
void bleESP32Crash();

// 4.9 - Smart Home Device Flood
void bleSmartHomeFlood();

// 4.10 - Generic GATT Exploiter
void bleGATTExploiter();

//================================================================================
// 5. MONITORING & DETECTION (8 functions)
//================================================================================

// 5.1 - Tracker Detector (AirTag, Tile, SmartTag)
void bleTrackerDetector();

// 5.2 - Device Classifier (categorize by type)
void bleDeviceClassifier();

// 5.3 - New Device Alert (enter/leave range)
void bleNewDeviceAlert();

// 5.4 - RSSI Heatmap (distance mapping)
void bleRSSIHeatmap();

// 5.5 - BLE Security Analyzer
void bleSecurityAnalyzer();

// 5.6 - Vulnerability Scanner
void bleVulnScanner();

// 5.7 - Service Enumerator (read all GATT services)
void bleServiceEnumerator();

// 5.8 - Connection Quality Monitor
void bleConnectionQuality();

//================================================================================
// MENU FUNCTIONS
//================================================================================

void bleReconMenu();
void bleAttackMenu();
void bleUtilityMenu();
void bleDeviceExploitMenu();
void bleMonitorMenu();
void bleAdvancedSuiteMenu();

//================================================================================
// HELPER FUNCTIONS
//================================================================================

std::vector<BLEScanResult> blePerformScan(int duration, bool active = true);
void bleDisplayScanResult(const BLEScanResult& result);
void bleDisplayProgress(const char* msg, int percent);
bool bleConfirmAction(const char* msg);
String bleSelectTarget();
void bleShowStats(uint32_t packets, uint32_t duration, int rssi);
void bleCleanup();

#endif // __BLE_ADVANCED_H__
