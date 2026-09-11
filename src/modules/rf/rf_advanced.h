/**
 * @file rf_advanced.h
 * @brief Willy RF Advanced Suite - 50+ Sub-GHz Functions
 * @author Willy Team
 * @date 2026
 *
 * Comprehensive RF/Sub-GHz toolkit for security research:
 * - Signal Analysis & Scanning (10 functions)
 * - Signal Generation & Transmission (10 functions)
 * - Device Control & Automation (10 functions)
 * - Signal Manipulation (10 functions)
 * - Testing & Diagnostics (10 functions)
 */

#ifndef __RF_ADVANCED_H__
#define __RF_ADVANCED_H__

#include <Arduino.h>
#include "modules/rf/structs.h"
#include "modules/rf/rf_utils.h"
#include <vector>

//================================================================================
// STRUCTURES & TYPES
//================================================================================

struct RFSignalInfo {
    float frequency;
    int rssi;
    String protocol;
    uint64_t key;
    int bits;
    int te;
    String preset;
    String rawData;
    String description;
};

struct RFTimingStats {
    uint32_t te;
    uint32_t syncHigh;
    uint32_t syncLow;
    uint32_t zeroHigh;
    uint32_t zeroLow;
    uint32_t oneHigh;
    uint32_t oneLow;
    uint32_t totalDuration;
    uint16_t pulseCount;
};

struct RFDeviceProfile {
    String brand;
    String model;
    String category;
    std::vector<uint32_t> commonCodes;
};

struct RFMacroStep {
    uint32_t delay;
    float frequency;
    String protocol;
    uint64_t key;
    int bits;
    int te;
    String label;
};

struct RFBatchEntry {
    String name;
    float frequency;
    String protocol;
    uint64_t key;
    int bits;
    int te;
    uint32_t repeatCount;
    uint32_t delayAfter;
};

enum RFAnalysisMode {
    RF_ANALYZE_PROTOCOL = 0,
    RF_ANALYZE_TIMING,
    RF_ANALYZE_QUALITY,
    RF_ANALYZE_FREQ,
    RF_ANALYZE_NOISE
};

enum RFGenerationMode {
    RF_GEN_PROTOCOL = 0,
    RF_GEN_RAW,
    RF_GEN_PRONTO,
    RF_GEN_UNIVERSAL,
    RF_GEN_CLONE
};

enum RFTestMode {
    RF_TEST_RANGE = 0,
    RF_TEST_STABILITY,
    RF_TEST_REPEAT,
    RF_TEST_PROTOCOL,
    RF_TEST_HARDWARE
};

//================================================================================
// 1. SIGNAL ANALYSIS & SCANNING (10 functions)
//================================================================================

void rfSignalAnalyzer();
void rfProtocolIdentifier();
void rfSignalComparison();
void rfFrequencyScanner();
void rfSignalStrengthMeter();
void rfTimingAnalyzer();
void rfSignalQualityChecker();
void rfBatchDecoder();
void rfProtocolDictionary();
void rfSignalExporter();

//================================================================================
// 2. SIGNAL GENERATION & TRANSMISSION (10 functions)
//================================================================================

void rfSignalCloner();
void rfProtocolGenerator();
void rfCustomCodeBuilder();
void rfBulkTransmitter();
void rfScheduledTransmitter();
void rfSignalRepeat();
void rfMultiFreqSender();
void rfSignalModulator();
void rfCodeConverter();
void rfSignalEditor();

//================================================================================
// 3. DEVICE CONTROL & AUTOMATION (10 functions)
//================================================================================

void rfDeviceMapper();
void rfMacroRecorder();
void rfMacroPlayer();
void rfSceneController();
void rfTimerScheduler();
void rfRemoteCloner();
void rfDeviceEmulator();
void rfCodeSearch();
void rfBrandExplorer();
void rfUniversalRemote();

//================================================================================
// 4. SIGNAL MANIPULATION (10 functions)
//================================================================================

void rfSignalRepeater();
void rfProtocolConverter();
void rfTimingModifier();
void rfCodeObfuscator();
void rfSignalAmplifier();
void rfSignalFilter();
void rfNoiseGenerator();
void rfSignalSplitter();
void rfSignalMerger();
void rfProtocolBridge();

//================================================================================
// 5. TESTING & DIAGNOSTICS (10 functions)
//================================================================================

void rfSignalValidator();
void rfProtocolTester();
void rfReceiverDiagnostics();
void rfTransmitterDiagnostics();
void rfRangeTest();
void rfInterferenceDetector();
void rfSignalBenchmark();
void rfProtocolStressTest();
void rfHardwareDiagnostics();
void rfSignalPlayback();

//================================================================================
// MENU FUNCTIONS
//================================================================================

void rfAnalysisMenu();
void rfGenerationMenu();
void rfDeviceControlMenu();
void rfManipulationMenu();
void rfTestingMenu();
void rfAdvancedSuiteMenu();

//================================================================================
// HELPER FUNCTIONS
//================================================================================

RFSignalInfo rfCaptureSignal(uint32_t timeout = 15000);
void rfDisplaySignalInfo(const RFSignalInfo& info, int y = 50);
void rfDisplayProgress(const char* msg, int percent);
bool rfConfirmAction(const char* msg);
void rfShowStats(uint32_t count, uint32_t duration, const char* status);
void rfSendCode(float freq, uint64_t code, int bits, int te, int repeats = 1);
void rfShowWelcomeScreen();
void rfDrawHeader(const char* title, uint16_t color);
void rfDrawFooter(const char* left, const char* right);
void rfDrawCard(int y, int h, bool sel);
void rfDrawProgressBar(int y, int pct, const char* label);
void rfDrawSpinner(int x, int y, int frame);

#endif // __RF_ADVANCED_H__
