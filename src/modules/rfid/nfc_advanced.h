/**
 * @file nfc_advanced.h
 * @brief Willy NFC Advanced Suite - 50+ NFC/RFID Functions
 * @author Willy Team
 * @date 2026
 *
 * Comprehensive NFC/RFID toolkit for security research:
 * - Tag Analysis & Reading (10 functions)
 * - Tag Generation & Cloning (10 functions)
 * - Device Control & Automation (10 functions)
 * - Tag Manipulation (10 functions)
 * - Testing & Diagnostics (10 functions)
 */

#ifndef __NFC_ADVANCED_H__
#define __NFC_ADVANCED_H__

#include <Arduino.h>
#include "modules/rfid/RFIDInterface.h"
#include <vector>

//================================================================================
// STRUCTURES & TYPES
//================================================================================

struct NFCTagInfo {
    String uid;
    String sak;
    String atqa;
    String typeName;
    String manufacturer;
    String technology;
    int totalBytes;
    int totalPages;
    bool authSuccess;
    String dump;
};

struct NFCAuthResult {
    bool success;
    String keyUsed;
    int sector;
    int block;
    String keyType;
};

struct NFCKeyEntry {
    uint8_t key[6];
    String name;
    bool found;
};

enum NFCAnalysisMode {
    NFC_ANALYZE_FULL = 0,
    NFC_ANALYZE_UID,
    NFC_ANALYZE_PROTOCOL,
    NFC_ANALYZE_NDEF,
    NFC_ANALYZE_MEMORY
};

enum NFCCloneMode {
    NFC_CLONE_UID = 0,
    NFC_CLONE_FULL,
    NFC_CLONE_NDEF,
    NFC_CLONE_CUSTOM
};

//================================================================================
// 1. TAG ANALYSIS & READING (10 functions)
//================================================================================

void nfcTagInfoReader();
void nfcUidAnalyzer();
void nfcProtocolDetector();
void nfcNdefParser();
void nfcMemoryDumper();
void nfcAuthAnalyzer();
void nfcAntiCollisionAnalyzer();
void nfcTagFingerprint();
void nfcSignalQualityChecker();
void nfcBatchDecoder();

//================================================================================
// 2. TAG GENERATION & CLONING (10 functions)
//================================================================================

void nfcUidCloner();
void nfcNdefBuilder();
void nfcCustomDataWriter();
void nfcMifareKeyManager();
void nfcMagicTagWriter();
void nfcTagEmulator();
void nfcMultiTagCloner();
void nfcDataConverter();
void nfcTagEditor();
void nfcFlipperConverter();

//================================================================================
// 3. DEVICE CONTROL & AUTOMATION (10 functions)
//================================================================================

void nfcTagMapper();
void nfcMacroRecorder();
void nfcMacroPlayer();
void nfcSceneController();
void nfcTimerScheduler();
void nfcMultiDeviceCloner();
void nfcTagEmulatorAuto();
void nfcUidSearcher();
void nfcProtocolExplorer();
void nfcUniversalReader();

//================================================================================
// 4. TAG MANIPULATION (10 functions)
//================================================================================

void nfcTagRepeater();
void nfcProtocolConverter();
void nfcTimingModifier();
void nfcDataObfuscator();
void nfcTagAmplifier();
void nfcTagFilter();
void nfcNoiseGenerator();
void nfcDataSplitter();
void nfcDataMerger();
void nfcProtocolBridge();

//================================================================================
// 5. TESTING & DIAGNOSTICS (10 functions)
//================================================================================

void nfcTagValidator();
void nfcProtocolTester();
void nfcReaderDiagnostics();
void nfcWriterDiagnostics();
void nfcRangeTest();
void nfcInterferenceDetector();
void nfcTagBenchmark();
void nfcProtocolStressTest();
void nfcHardwareDiagnostics();
void nfcTagPlayback();

//================================================================================
// MENU FUNCTIONS
================================================================================

void nfcAnalysisMenu();
void nfcGenerationMenu();
void nfcDeviceControlMenu();
void nfcManipulationMenu();
void nfcTestingMenu();
void nfcAdvancedSuiteMenu();

//================================================================================
// HELPER FUNCTIONS
//================================================================================

NFCTagInfo nfcReadTagInfo();
void nfcDisplayTagInfo(const NFCTagInfo& info, int y = 50);
void nfcDisplayProgress(const char* msg, int percent);
bool nfcConfirmAction(const char* msg);
void nfcShowStats(uint32_t count, uint32_t duration, const char* status);
void nfcShowWelcomeScreen();
void nfcDrawHeader(const char* title, uint16_t color);
void nfcDrawFooter(const char* left, const char* right);
void nfcDrawCard(int y, int h, bool sel);
void nfcDrawProgressBar(int y, int pct, const char* label);
void nfcDrawSpinner(int x, int y, int frame);
RFIDInterface* nfcCreateModule();

#endif // __NFC_ADVANCED_H__
