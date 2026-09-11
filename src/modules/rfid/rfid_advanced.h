#ifndef __RFID_ADVANCED_H__
#define __RFID_ADVANCED_H__

#include <Arduino.h>
#include <vector>
#include "RFIDInterface.h"

// ============================================================================
// RFID Advanced Suite — 55+ Real Functions for RFID Security Research
// ============================================================================

// --- Color/Layout Constants ---
#define RFID_COLOR_BG       wilyConfig.bgColor
#define RFID_COLOR_PRIMARY  wilyConfig.priColor
#define RFID_COLOR_TITLE    TFT_CYAN
#define RFID_COLOR_ACCENT   TFT_GREEN
#define RFID_COLOR_WARN     TFT_YELLOW
#define RFID_COLOR_DANGER   TFT_RED
#define RFID_COLOR_TEXT     TFT_WHITE
#define RFID_COLOR_DIM      TFT_DARKGREY
#define RFID_BODY_Y         50
#define RFID_FOOTER_Y       (tftHeight - 28)

// --- Config Structure ---
struct RFIDAdvConfig {
    int defaultBaudRate;      // 0=MIFARE, 1=FeliCa
    int keyAttackMaxTries;    // Max key tries per sector
    int readDelayMs;          // Delay between reads
    bool autoSave;            // Auto-save dumps to SD
    bool showProgress;        // Show progress bars
    int emulTimeoutMs;        // Emulation timeout
    int scanDurationSec;      // Scan duration

    RFIDAdvConfig() :
        defaultBaudRate(0),
        keyAttackMaxTries(50),
        readDelayMs(100),
        autoSave(false),
        showProgress(true),
        emulTimeoutMs(30000),
        scanDurationSec(15) {}
};

extern RFIDAdvConfig rfidAdvConfig;

// ============================================================================
// CATEGORY 1: MIFARE Classic Deep Analysis (12 functions)
// ============================================================================

// Full dump of all sectors with key recovery — populates mfcDump
void rfidMifareClassicFullDump();
// Test all known keys against a specific sector
void rfidMifareClassicSectorAuth();
// Recover keys from successful authentications across all sectors
void rfidMifareClassicKeyRecovery();
// Map all sectors showing auth status, key A/B found status
void rfidMifareClassicSectorMap();
// Read a specific block by number with authentication
void rfidMifareClassicBlockRead();
// Write data to a specific block with authentication
void rfidMifareClassicBlockWrite();
// Read/write MIFARE Classic value blocks (increment/decrement)
void rfidMifareClassicValueBlock();
// Analyze access bits/conditions for all sectors
void rfidMifareClassicAccessBits();
// Brute-force with comprehensive key dictionary
void rfidMifareClassicKeyDictionary();
// Clone MIFARE Classic dump to magic Gen1/Gen2 tag
void rfidMifareClassicClone();
// Emulate MIFARE Classic tag (ST25R3916 only)
void rfidMifareClassicEmulate();
// Detect static nonce vulnerability
void rfidMifareStaticNonceDetect();

// ============================================================================
// CATEGORY 2: MIFARE Ultralight & NTAG (10 functions)
// ============================================================================

// Dump all pages of Ultralight/NTAG tag
void rfidUltralightFullDump();
// Read a specific page by number
void rfidUltralightPageRead();
// Write data to a specific page
void rfidUltralightPageWrite();
// Test password authentication (3-step auth)
void rfidUltralightPasswordAuth();
// Read and display OTP bytes with explanation
void rfidUltralightOTPRead();
// Read monotonic counters (NTAG213/215/216)
void rfidUltralightCounterRead();
// Display NTAG version information
void rfidNTAGVersionInfo();
// Read and display ECC-P256 signature
void rfidNTAGSignatureRead();
// Set or verify NTAG password protection
void rfidNTAGPasswordProtect();
// Verify originality signature (NTAG21x)
void rfidNTAGOriginalityCheck();

// ============================================================================
// CATEGORY 3: DESFire & ISO-DEP (8 functions)
// ============================================================================

// Read DESFire card information (version, memory, key settings)
void rfidDESFireCardInfo();
// List all applications on DESFire card
void rfidDESFireAppList();
// Read data from DESFire standard/backup data file
void rfidDESFireReadFile();
// Write data to DESFire standard/backup data file
void rfidDESFireWriteFile();
// Authenticate to DESFire application (DES/3DES/AES)
void rfidDESFireAuthenticate();
// Format/reset DESFire card (deletes all apps)
void rfidDESFireFormatCard();
// Probe ISO-DEP device (sends SELECT + GET VERSION)
void rfidISODEPProbe();
// Read NDEF data from ISO-DEP / Type 4 Tag
void rfidISODEPNdefRead();

// ============================================================================
// CATEGORY 4: ISO15693 & FeliCa (6 functions)
// ============================================================================

// ISO15693 (NFC-V) inventory — detect all tags in field
void rfidISO15693Inventory();
// Read a block from ISO15693 tag
void rfidISO15693ReadBlock();
// Write a block to ISO15693 tag
void rfidISO15693WriteBlock();
// Get ISO15693 system info (block count, memory size, etc.)
void rfidISO15693SystemInfo();
// Read FeliCa block without encryption
void rfidFeliCaRead();
// Write FeliCa block without encryption
void rfidFeliCaWrite();

// ============================================================================
// CATEGORY 5: 125kHz Operations (8 functions)
// ============================================================================

// Read EM4100 125kHz tag via UART
void rfidEM4100Read();
// Clone EM4100 tag data to T5577 writable tag
void rfidEM4100Clone();
// Write configuration to T5577 tag
void rfidT5577WriteConfig();
// Read T5577 configuration block
void rfidT5577ReadConfig();
// Read HID Prox format tag (26-bit/34-bit)
void rfidHIDProxRead();
// Read Indala format tag
void rfidIndalaRead();
// Write EM4x02 tag data
void rfidEM4x02Write();
// Display raw 125kHz tag data in multiple formats
void rfid125kHzRawDisplay();

// ============================================================================
// CATEGORY 6: Advanced Tag Operations (8 functions)
// ============================================================================

// Detect multiple tags simultaneously in field
void rfidMultiTagDetect();
// Display memory map of detected tag
void rfidTagMemoryMap();
// Auto-detect tag type from UID/SAK/ATQA
void rfidTagFormatDetect();
// Compare two tag dumps side by side
void rfidTagDumpCompare();
// Merge two tag dumps (fill missing blocks)
void rfidTagDumpMerge();
// Analyze dump structure and report stats
void rfidTagDumpAnalyze();
// Export dump in Flipper Zero / Proxmark format
void rfidTagDumpExport();
// Import dump from file
void rfidTagDumpImport();

// ============================================================================
// CATEGORY 7: Security Testing (6 functions)
// ============================================================================

// Measure tag read success rate over time
void rfidReadRateTest();
// Test authentication success rate per sector
void rfidAuthSuccessRate();
// Measure tag response timing
void rfidTimingAnalysis();
// Test tag under various conditions
void rfidStressTest();
// Test reader field strength / range
void rfidFieldStrengthTest();
// Comprehensive tag compatibility check
void rfidCompatibilityCheck();

// ============================================================================
// MENU SYSTEM (7 menus)
// ============================================================================

void rfidMifareClassicMenu();
void rfidUltralightNTAGMenu();
void rfidDESFireMenu();
void rfidISO15693FeliCaMenu();
void rfid125kHzMenu();
void rfidAdvancedOpsMenu();
void rfidSecurityTestMenu();
void rfidAdvancedSuiteMenu();

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

RFIDInterface* rfidAdvCreateModule();
void rfidAdvSetup();
void rfidAdvCleanup();

NFCTagInfo rfidAdvReadTagInfo();
void rfidAdvDisplayTagInfo(const NFCTagInfo &info, int y = RFID_BODY_Y);
void rfidAdvDisplayProgress(const char *msg, int percent);
bool rfidAdvConfirmAction(const char *msg);
void rfidAdvShowWelcomeScreen();

void rfidAdvDrawHeader(const char *title, uint16_t color);
void rfidAdvDrawFooter(const char *left, const char *right);
void rfidAdvDrawCard(int y, int h, bool sel);
void rfidAdvDrawProgressBar(int y, int pct, const char *label);
void rfidAdvDrawSpinner(int x, int y, int frame);
void rfidAdvShowWelcomeScreen();
void rfidAdvConfigMenu();

#endif // __RFID_ADVANCED_H__
