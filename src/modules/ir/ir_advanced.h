/**
 * @file ir_advanced.h
 * @brief Willy IR Advanced Suite - 50+ Infrared Functions
 * @author Willy Team
 * @date 2026
 *
 * Comprehensive IR toolkit for security research:
 * - Signal Analysis & Decoding (10 functions)
 * - Signal Generation & Cloning (10 functions)
 * - Device Control & Automation (10 functions)
 * - Signal Manipulation (10 functions)
 * - Testing & Diagnostics (10 functions)
 */

#ifndef __IR_ADVANCED_H__
#define __IR_ADVANCED_H__

#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRrecv.h>
#include <vector>
#include <globals.h>

//================================================================================
// STRUCTURES & TYPES
//================================================================================

struct IRSignalInfo {
    decode_type_t protocol;
    uint64_t data;
    uint32_t bits;
    uint16_t address;
    uint16_t command;
    uint32_t frequency;
    float dutyCycle;
    String protocolName;
    String description;
};

struct IRTimingStats {
    uint32_t headerMark;
    uint32_t headerSpace;
    uint32_t bitMark;
    uint32_t oneSpace;
    uint32_t zeroSpace;
    uint32_t gap;
    uint32_t totalDuration;
    uint16_t pulseCount;
};

struct IRDeviceProfile {
    String brand;
    String model;
    String category;
    std::vector<uint32_t> commonCodes;
};

struct IRMacroStep {
    uint32_t delay;
    decode_type_t protocol;
    uint64_t data;
    uint32_t bits;
    String label;
};

struct IRBatchEntry {
    String name;
    decode_type_t protocol;
    uint64_t data;
    uint32_t bits;
    uint32_t repeatCount;
    uint32_t delayAfter;
};

enum IRAnalysisMode {
    IR_ANALYZE_PROTOCOL = 0,
    IR_ANALYZE_TIMING,
    IR_ANALYZE_QUALITY,
    IR_ANALYZE_FREQ,
    IR_ANALYZE_NOISE
};

enum IRGenerationMode {
    IR_GEN_PROTOCOL = 0,
    IR_GEN_RAW,
    IR_GEN_PRONTO,
    IR_GEN_UNIVERSAL,
    IR_GEN_CLONE
};

enum IRTestMode {
    IR_TEST_RANGE = 0,
    IR_TEST_STABILITY,
    IR_TEST_REPEAT,
    IR_TEST_PROTOCOL,
    IR_TEST_HARDWARE
};

//================================================================================
// 1. SIGNAL ANALYSIS & DECODING (10 functions)
//================================================================================

void irSignalAnalyzer();
void irProtocolIdentifier();
void irSignalComparison();
void irFrequencyScanner();
void irSignalStrengthMeter();
void irTimingAnalyzer();
void irSignalQualityChecker();
void irBatchDecoder();
void irProtocolDictionary();
void irSignalExporter();

//================================================================================
// 2. SIGNAL GENERATION & CLONING (10 functions)
//================================================================================

void irSignalCloner();
void irProtocolGenerator();
void irCustomCodeBuilder();
void irBulkTransmitter();
void irScheduledTransmitter();
void irSignalRepeat();
void irMultiProtocolSender();
void irSignalModulator();
void irCodeConverter();
void irSignalEditor();

//================================================================================
// 3. DEVICE CONTROL & AUTOMATION (10 functions)
//================================================================================

void irDeviceMapper();
void irMacroRecorder();
void irMacroPlayer();
void irSceneController();
void irTimerScheduler();
void irRemoteCloner();
void irDeviceEmulator();
void irCodeSearch();
void irBrandExplorer();
void irUniversalRemote();

//================================================================================
// 4. SIGNAL MANIPULATION (10 functions)
//================================================================================

void irSignalRepeater();
void irProtocolConverter();
void irTimingModifier();
void irCodeObfuscator();
void irSignalAmplifier();
void irSignalFilter();
void irNoiseGenerator();
void irSignalSplitter();
void irSignalMerger();
void irProtocolBridge();

//================================================================================
// 5. TESTING & DIAGNOSTICS (10 functions)
//================================================================================

void irSignalValidator();
void irProtocolTester();
void irReceiverDiagnostics();
void irTransmitterDiagnostics();
void irRangeTest();
void irInterferenceDetector();
void irSignalBenchmark();
void irProtocolStressTest();
void irHardwareDiagnostics();
void irSignalPlayback();

//================================================================================
// MENU FUNCTIONS
//================================================================================

void irAnalysisMenu();
void irGenerationMenu();
void irDeviceMenu();
void irManipulationMenu();
void irTestingMenu();
void irAdvancedSuiteMenu();

//================================================================================
// HELPER FUNCTIONS
//================================================================================

IRSignalInfo irCaptureSignal(uint32_t timeout = 15000);
void irDisplaySignalInfo(const IRSignalInfo& info, int y = 50);
void irDisplayProgress(const char* msg, int percent);
bool irConfirmAction(const char* msg);
void irShowStats(uint32_t count, uint32_t duration, const char* status);
void irSendDecoded(decode_type_t protocol, uint64_t data, uint32_t bits, uint16_t repeats = 1);
void irSendRaw(uint32_t frequency, const uint16_t* data, uint32_t len, uint16_t repeats = 1);
void irShowWelcomeScreen();

#endif // __IR_ADVANCED_H__
