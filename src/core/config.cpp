#include "config.h"
#include "mifare_keys_manager.h"
#include "sd_functions.h"
#include <algorithm>
#include <esp_pm.h>
#include <algorithm>

const String CRYPTO_KEY = "WillyFirmwareCoreRefinement";
const String ENC_PREFIX = "_ENC_";
const String ENC_V2_PREFIX = "_ENC_V2_";

static String getCryptoKey() {
    static String key = "";
    if (key.isEmpty()) {
        uint64_t mac = ESP.getEfuseMac();
        char buf[17];
        snprintf(buf, sizeof(buf), "%04x%08x", (uint32_t)(mac >> 32), (uint32_t)mac);
        key = String(buf);
    }
    return key;
}

JsonDocument BruceConfig::toJson() const {
    JsonDocument jsonDoc;
    JsonObject setting = jsonDoc.to<JsonObject>();

    setting["priColor"] = String(priColor, HEX);
    setting["secColor"] = String(secColor, HEX);
    setting["bgColor"] = String(bgColor, HEX);
    setting["themeFile"] = themePath;
    setting["themeOnSd"] = theme.fs;

    setting["dimmerSet"] = dimmerSet;
    setting["bright"] = bright;
    setting["automaticTimeUpdateViaNTP"] = automaticTimeUpdateViaNTP;
    setting["tmz"] = tmz;
    setting["dst"] = dst;
    setting["clock24hr"] = clock24hr;
    setting["soundEnabled"] = soundEnabled;
    setting["soundVolume"] = soundVolume;
    setting["wifiAtStartup"] = wifiAtStartup;
    setting["instantBoot"] = instantBoot;

#ifdef HAS_RGB_LED
    setting["ledBright"] = ledBright;
    setting["ledColor"] = String(ledColor, HEX);
    setting["ledBlinkEnabled"] = ledBlinkEnabled;
    setting["ledEffect"] = ledEffect;
    setting["ledEffectSpeed"] = ledEffectSpeed;
    setting["ledEffectDirection"] = ledEffectDirection;
#endif

    JsonObject _webUI = setting["webUI"].to<JsonObject>();
    _webUI["user"] = webUI.user;
    _webUI["user"] = webUI.user;
    _webUI["pwd"] = encryptString(webUI.pwd);
    JsonObject _webUISessions = setting["webUISessions"].to<JsonObject>();
    for (size_t i = 0; i < webUISessions.size(); i++) { _webUISessions[String(i + 1)] = encryptString(webUISessions[i]); }

    JsonObject _wifiAp = setting["wifiAp"].to<JsonObject>();
    _wifiAp["ssid"] = wifiAp.ssid;
    _wifiAp["ssid"] = wifiAp.ssid;
    _wifiAp["pwd"] = encryptString(wifiAp.pwd);
    setting["wifiMAC"] = wifiMAC; //@IncursioHack

    JsonArray _evilWifiNames = setting["evilWifiNames"].to<JsonArray>();
    for (auto key : evilWifiNames) _evilWifiNames.add(key);

    JsonObject _evilWifiEndpoints = setting["evilWifiEndpoints"].to<JsonObject>();
    _evilWifiEndpoints["getCredsEndpoint"] = evilPortalEndpoints.getCredsEndpoint;
    _evilWifiEndpoints["setSsidEndpoint"] = evilPortalEndpoints.setSsidEndpoint;
    _evilWifiEndpoints["showEndpoints"] = evilPortalEndpoints.showEndpoints;
    _evilWifiEndpoints["allowSetSsid"] = evilPortalEndpoints.allowSetSsid;
    _evilWifiEndpoints["allowGetCreds"] = evilPortalEndpoints.allowGetCreds;

    setting["evilWifiPasswordMode"] = evilPortalPasswordMode;

    JsonObject _wifi = setting["wifi"].to<JsonObject>();
    for (const auto &pair : wifi) { _wifi[pair.first] = encryptString(pair.second); }

    setting["startupApp"] = startupApp;
    setting["startupAppLuaScript"] = startupAppLuaScript;
    setting["wigleBasicToken"] = encryptString(wigleBasicToken);
    setting["devMode"] = devMode;
    setting["colorInverted"] = colorInverted;

    setting["badUSBBLEKeyboardLayout"] = badUSBBLEKeyboardLayout;
    setting["badUSBBLEKeyDelay"] = badUSBBLEKeyDelay;
    setting["badUSBBLEShowOutput"] = badUSBBLEShowOutput;

    JsonArray dm = setting["disabledMenus"].to<JsonArray>();
    for (int i = 0; i < disabledMenus.size(); i++) { dm.add(disabledMenus[i]); }

    JsonArray qrArray = setting["qrCodes"].to<JsonArray>();
    for (const auto &entry : qrCodes) {
        JsonObject qrEntry = qrArray.add<JsonObject>();
        qrEntry["menuName"] = entry.menuName;
        qrEntry["content"] = entry.content;
    }

    return jsonDoc;
}


String BruceConfig::getDefaultWifiApSsid() const {
    uint64_t chipid = ESP.getEfuseMac();
    char buf[32];
    snprintf(buf, sizeof(buf), "WillyNet_%04X", (uint16_t)(chipid >> 32));
    return String(buf);
}

String BruceConfig::getDefaultWifiApPwd() const {
    uint64_t chipid = ESP.getEfuseMac();
    char buf[32];
    snprintf(buf, sizeof(buf), "Willy_%08X", (uint32_t)chipid);
    return String(buf);
}

void BruceConfig::initDefaults() {
    if (wifiAp.ssid.isEmpty() || wifiAp.ssid == "WillyNet") wifiAp.ssid = getDefaultWifiApSsid();
    if (wifiAp.pwd.isEmpty() || wifiAp.pwd == "WillyNet") wifiAp.pwd = getDefaultWifiApPwd();

    // Set default QR codes if none exist initially (this only runs on first init before loading from file)
    if (qrCodes.empty() && wifiAp.ssid == getDefaultWifiApSsid()) {
        qrCodes = {
            {"Willy AP",   "WIFI:T:WPA;S:" + wifiAp.ssid + ";P:" + wifiAp.pwd + ";;"},
            {"Willy Wiki", "https://github.com/lelebrr/Willy/wiki"},
            {"Willy Site", "https://willy.computer"},
            {"Rickroll",   "https://youtu.be/dQw4w9WgXcQ"}
        };
    }
}

void BruceConfig::fromFile(bool checkFS) {
    FS *fs;
    if (checkFS) {
        if (!getFsStorage(fs)) {
            log_i("Fail getting filesystem for config");
            return;
        }
    } else {
        if (checkLittleFsSize()) fs = &LittleFS;
        else return;
    }

    if (!fs->exists(filepath)) {
        log_i("Config file not found. Creating default config");
        return saveFile();
    }

    File file;
    file = fs->open(filepath, FILE_READ);
    if (!file) {
        log_i("Config file not found. Using default values");
        return;
    }

    // Deserialize the JSON document
    JsonDocument jsonDoc;
    Serial.println("Starting deserializeJson in BruceConfig...");
    if (deserializeJson(jsonDoc, file)) {
        Serial.println("Failed to read config file, using default configuration");
        return;
    }
    Serial.println("deserializeJson completed successfully.");
    file.close();

    JsonObject setting = jsonDoc.as<JsonObject>();
    int count = 0;

    if (!setting["priColor"].isNull()) {
        priColor = strtoul(setting["priColor"], nullptr, 16);
    } else {
        count++;
        log_e("Fail");
    }
    if (!setting["secColor"].isNull()) {
        secColor = strtoul(setting["secColor"], nullptr, 16);
    } else {
        count++;
        log_e("Fail");
    }
    if (!setting["bgColor"].isNull()) {
        bgColor = strtoul(setting["bgColor"], nullptr, 16);
    } else {
        count++;
        log_e("Fail");
    }

    if (!setting["themeFile"].isNull()) {
        themePath = setting["themeFile"].as<String>();
    } else {
        count++;
        log_e("Fail");
    }
    if (!setting["themeOnSd"].isNull()) {
        theme.fs = setting["themeOnSd"].as<int>();
    } else {
        count++;
        log_e("Fail");
    }

    if (!setting["dimmerSet"].isNull()) {
        dimmerSet = setting["dimmerSet"].as<int>();
    } else {
        count++;
        log_e("Fail");
    }
    if (!setting["bright"].isNull()) {
        bright = setting["bright"].as<int>();
    } else {
        count++;
        log_e("Fail");
    }
    if (!setting["automaticTimeUpdateViaNTP"].isNull()) {
        automaticTimeUpdateViaNTP = setting["automaticTimeUpdateViaNTP"].as<bool>();
    } else {
        count++;
        log_e("Fail");
    }
    if (!setting["tmz"].isNull()) {
        tmz = setting["tmz"].as<float>();
    } else {
        count++;
        log_e("Fail");
    }
    if (!setting["dst"].isNull()) {
        dst = setting["dst"].as<bool>();
    } else {
        count++;
        log_e("Fail");
    }
    if (!setting["clock24hr"].isNull()) {
        clock24hr = setting["clock24hr"].as<bool>();
    } else {
        count++;
        log_e("Fail");
    }
    if (!setting["soundEnabled"].isNull()) {
        soundEnabled = setting["soundEnabled"].as<int>();
    } else {
        count++;
        log_e("Fail");
    }
    if (!setting["soundVolume"].isNull()) {
        soundVolume = setting["soundVolume"].as<int>();
    } else {
        count++;
        log_e("Fail");
    }
    if (!setting["wifiAtStartup"].isNull()) {
        wifiAtStartup = setting["wifiAtStartup"].as<int>();
    } else {
        count++;
        log_e("Fail");
    }
    if (!setting["instantBoot"].isNull()) {
        instantBoot = setting["instantBoot"].as<int>();
    } else {
        count++;
        log_e("Fail");
    }

#ifdef HAS_RGB_LED
    if (!setting["ledBright"].isNull()) {
        ledBright = setting["ledBright"].as<int>();
    } else {
        count++;
        log_e("Fail");
    }
    if (!setting["ledColor"].isNull()) {
        ledColor = strtoul(setting["ledColor"], nullptr, 16);
    } else {
        count++;
        log_e("Fail");
    }
    if (!setting["ledBlinkEnabled"].isNull()) {
        ledBlinkEnabled = setting["ledBlinkEnabled"].as<int>();
    } else {
        count++;
        log_e("Fail");
    }
    if (!setting["ledEffect"].isNull()) {
        ledEffect = setting["ledEffect"].as<int>();
    } else {
        count++;
        log_e("Fail");
    }
    if (!setting["ledEffectSpeed"].isNull()) {
        ledEffectSpeed = setting["ledEffectSpeed"].as<int>();
    } else {
        count++;
        log_e("Fail");
    }
    if (!setting["ledEffectDirection"].isNull()) {
        ledEffectDirection = setting["ledEffectDirection"].as<int>();
    } else {
        count++;
        log_e("Fail");
    }
#endif

    if (!setting["webUI"].isNull()) {
        JsonObject webUIObj = setting["webUI"].as<JsonObject>();
        webUI.user = webUIObj["user"].as<String>();
        webUI.pwd = decryptString(webUIObj["pwd"].as<String>());
    } else {
        count++;
        log_e("Fail");
    }

    if (!setting["webUISessions"].isNull()) {
        webUISessions.clear();
        JsonObject webUISessionsObj = setting["webUISessions"].as<JsonObject>();
        for (JsonPair kv : webUISessionsObj) { webUISessions.push_back(decryptString(kv.value().as<String>())); }
    } else {
        count++;
        log_e("Fail");
    }

    if (!setting["wifiAp"].isNull()) {
        JsonObject wifiApObj = setting["wifiAp"].as<JsonObject>();
        wifiAp.ssid = wifiApObj["ssid"].as<String>();
        wifiAp.pwd = decryptString(wifiApObj["pwd"].as<String>());
    } else {
        count++;
        log_e("Fail");
    }

    //@IncursioHack
    if (!setting["wifiMAC"].isNull()) {
        wifiMAC = setting["wifiMAC"].as<String>();
    } else {
        wifiMAC = "";
        count++;
        log_e("wifiMAC not found, using default");
    }

    // Wifi List
    if (!setting["wifi"].isNull()) {
        wifi.clear();
        JsonObject wifiObj = setting["wifi"].as<JsonObject>();
        for (JsonPair kv : wifiObj) wifi[kv.key().c_str()] = decryptString(kv.value().as<String>());
    } else {
        count++;
        log_e("Fail");
    }

    if (!setting["evilWifiNames"].isNull()) {
        evilWifiNames.clear();
        JsonArray _evilWifiNames = setting["evilWifiNames"].as<JsonArray>();
        for (JsonVariant key : _evilWifiNames) evilWifiNames.insert(key.as<String>());
    } else {
        count++;
        log_e("Fail");
    }

    if (!setting["evilWifiEndpoints"].isNull()) {
        JsonObject evilPortalEndpointsObj = setting["evilWifiEndpoints"].as<JsonObject>();
        evilPortalEndpoints.getCredsEndpoint = evilPortalEndpointsObj["getCredsEndpoint"].as<String>();
        evilPortalEndpoints.setSsidEndpoint = evilPortalEndpointsObj["setSsidEndpoint"].as<String>();
        evilPortalEndpoints.showEndpoints = evilPortalEndpointsObj["showEndpoints"].as<bool>();
        evilPortalEndpoints.allowSetSsid = evilPortalEndpointsObj["allowSetSsid"].as<bool>();
        evilPortalEndpoints.allowGetCreds = evilPortalEndpointsObj["allowGetCreds"].as<bool>();
    } else {
        count++;
        log_e("Fail");
    }

    if (!setting["evilWifiPasswordMode"].isNull()) {
        int mode = setting["evilWifiPasswordMode"].as<int>();
        if (mode >= 0 && mode <= 2) {
            evilPortalPasswordMode = static_cast<EvilPortalPasswordMode>(mode);
        } else {
            evilPortalPasswordMode = FULL_PASSWORD;
            log_w("Invalid evilWifiPasswordMode, using FULL_PASSWORD");
        }
    } else {
        count++;
        log_e("Fail");
    }

    if (!setting["startupApp"].isNull()) {
        startupApp = setting["startupApp"].as<String>();
    } else {
        count++;
        log_e("Fail");
    }
    if (!setting["startupAppLuaScript"].isNull()) {
        startupAppLuaScript = setting["startupAppLuaScript"].as<String>();
    } else if (!setting["startupAppJSInterpreterFile"].isNull()) {
        // Migration from old name
        startupAppLuaScript = setting["startupAppJSInterpreterFile"].as<String>();
    } else {
        count++;
        log_e("Fail");
    }

    if (!setting["wigleBasicToken"].isNull()) {
        wigleBasicToken = decryptString(setting["wigleBasicToken"].as<String>());
    } else {
        count++;
        log_e("Fail");
    }
    if (!setting["devMode"].isNull()) {
        devMode = setting["devMode"].as<int>();
    } else {
        count++;
        log_e("Fail");
    }
    if (!setting["colorInverted"].isNull()) {
        colorInverted = setting["colorInverted"].as<int>();
    } else {
        count++;
        log_e("Fail");
    }

    if (!setting["badUSBBLEKeyboardLayout"].isNull()) {
        badUSBBLEKeyboardLayout = setting["badUSBBLEKeyboardLayout"].as<int>();
    } else {
        count++;
        log_e("Fail");
    }

    if (!setting["badUSBBLEKeyDelay"].isNull()) {
        badUSBBLEKeyDelay = setting["badUSBBLEKeyDelay"].as<int>();
    } else {
        count++;
        log_e("Fail");
    }

    if (!setting["badUSBBLEShowOutput"].isNull()) {
        badUSBBLEShowOutput = setting["badUSBBLEShowOutput"].as<bool>();
    } else {
        count++;
        log_e("Fail");
    }

    if (!setting["disabledMenus"].isNull()) {
        disabledMenus.clear();
        JsonArray dm = setting["disabledMenus"].as<JsonArray>();
        for (JsonVariant e : dm) { disabledMenus.push_back(e.as<String>()); }
    } else {
        count++;
        log_e("Fail");
    }

    if (!setting["qrCodes"].isNull()) {
        qrCodes.clear();
        JsonArray qrArray = setting["qrCodes"].as<JsonArray>();
        for (JsonObject qrEntry : qrArray) {
            String menuName = qrEntry["menuName"].as<String>();
            String content = qrEntry["content"].as<String>();
            qrCodes.push_back({menuName, content});
        }
    } else {
        count++;
        log_e("Fail to load qrCodes");
    }

    validateConfig();
    if (count > 0) saveFile();

    // Load MIFARE keys (loading via manager)
    MifareKeysManager::ensureLoaded(mifareKeys);

    log_i("Using config from file");
}

void BruceConfig::saveFile() {
    FS *fs = &LittleFS;
    JsonDocument jsonDoc = toJson();

    // Open file for writing
    File file = fs->open(filepath, FILE_WRITE);
    if (!file) {
        log_e("Failed to open config file");
        file.close();
        return;
    };

    // Serialize JSON to file
    if (serializeJsonPretty(jsonDoc, file) < 5) log_e("Failed to write config file");
    else log_i("config file written successfully");

    file.close();

    if (setupSdCard()) copyToFs(LittleFS, SD, filepath, false);
}

void BruceConfig::factoryReset() {
    FS *fs = &LittleFS;
    fs->rename(String(filepath), "/bak." + String(filepath).substring(1));
    if (setupSdCard()) SD.rename(String(filepath), "/bak." + String(filepath).substring(1));
    ESP.restart();
}

void BruceConfig::validateConfig() {
    validateDimmerValue();
    validateBrightValue();
    validateTmzValue();
    validateSoundEnabledValue();
    validateSoundVolumeValue();
    validateWifiAtStartupValue();
#ifdef HAS_RGB_LED
    validateLedBrightValue();
    validateLedColorValue();
    validateLedBlinkEnabledValue();
    validateLedEffectValue();
    validateLedEffectSpeedValue();
    validateLedEffectDirectionValue();
#endif
    validateMifareKeysItems();
    validateDevModeValue();
    validateColorInverted();
    validateBadUSBBLEKeyboardLayout();
    validateBadUSBBLEKeyDelay();
    validateEvilEndpointCreds();
    validateEvilEndpointSsid();
    validateEvilPasswordMode();
}

void BruceConfig::setUiColor(uint16_t primary, uint16_t *secondary, uint16_t *background) {
    BruceTheme::_setUiColor(primary, secondary, background);
    saveFile();
}

void BruceConfig::setDimmer(int value) {
    dimmerSet = value;
    validateDimmerValue();
    saveFile();
}

void BruceConfig::validateDimmerValue() {
    if (dimmerSet < 0) dimmerSet = 10;
    if (dimmerSet > 60) dimmerSet = 0;
}

void BruceConfig::setBright(uint8_t value) {
    bright = value;
    validateBrightValue();
    saveFile();
}

void BruceConfig::validateBrightValue() {
    if (bright > 100) bright = 100;
}

void BruceConfig::setAutomaticTimeUpdateViaNTP(bool value) {
    automaticTimeUpdateViaNTP = value;
    saveFile();
}

void BruceConfig::setTmz(float value) {
    tmz = value;
    validateTmzValue();
    saveFile();
}

void BruceConfig::validateTmzValue() {
    if (tmz < -12 || tmz > 14) tmz = 0;
}

void BruceConfig::setDST(bool value) {
    dst = value;
    saveFile();
}

void BruceConfig::setClock24Hr(bool value) {
    clock24hr = value;
    saveFile();
}

void BruceConfig::setSoundEnabled(int value) {
    soundEnabled = value;
    validateSoundEnabledValue();
    saveFile();
}

void BruceConfig::setSoundVolume(int value) {
    soundVolume = value;
    validateSoundVolumeValue();
    saveFile();
}

void BruceConfig::validateSoundEnabledValue() {
    if (soundEnabled > 1) soundEnabled = 1;
}

void BruceConfig::validateSoundVolumeValue() {
    if (soundVolume > 100) soundVolume = 100;
}

void BruceConfig::setWifiAtStartup(int value) {
    wifiAtStartup = value;
    validateWifiAtStartupValue();
    saveFile();
}

void BruceConfig::validateWifiAtStartupValue() {
    if (wifiAtStartup > 1) wifiAtStartup = 1;
}

#ifdef HAS_RGB_LED
void BruceConfig::setLedBright(int value) {
    ledBright = value;
    validateLedBrightValue();
    saveFile();
}

void BruceConfig::validateLedBrightValue() { ledBright = max(0, min(100, ledBright)); }

void BruceConfig::setLedColor(uint32_t value) {
    ledColor = value;
    validateLedColorValue();
    saveFile();
}

void BruceConfig::validateLedColorValue() {
    ledColor = max<uint32_t>(0, min<uint32_t>(0xFFFFFFFF, ledColor));
}

void BruceConfig::setLedBlinkEnabled(int value) {
    ledBlinkEnabled = value;
    validateLedBlinkEnabledValue();
    saveFile();
}

void BruceConfig::validateLedBlinkEnabledValue() {
    if (ledBlinkEnabled > 1) ledBlinkEnabled = 1;
}

void BruceConfig::setLedEffect(int value) {
    ledEffect = value;
    validateLedEffectValue();
    saveFile();
}

void BruceConfig::validateLedEffectValue() {
    if (ledEffect < 0 || ledEffect > 5) ledEffect = 0;
}

void BruceConfig::setLedEffectSpeed(int value) {
    ledEffectSpeed = value;
    validateLedEffectSpeedValue();
    saveFile();
}

void BruceConfig::validateLedEffectSpeedValue() {
#ifdef HAS_ENCODER_LED
    if (ledEffectSpeed > 11) ledEffectSpeed = 11;
#else
    if (ledEffectSpeed > 10) ledEffectSpeed = 10;
#endif
    if (ledEffectSpeed < 0) ledEffectSpeed = 1;
}

void BruceConfig::setLedEffectDirection(int value) {
    ledEffectDirection = value;
    validateLedEffectDirectionValue();
    saveFile();
}

void BruceConfig::validateLedEffectDirectionValue() {
    if (ledEffectDirection > 1 || ledEffectDirection == 0) ledEffectDirection = 1;
    if (ledEffectDirection < -1) ledEffectDirection = -1;
}
#endif

void BruceConfig::setWebUICreds(const String &usr, const String &pwd) {
    webUI.user = usr;
    webUI.pwd = pwd;
    saveFile();
}

void BruceConfig::setWifiApCreds(const String &ssid, const String &pwd) {
    wifiAp.ssid = ssid;
    wifiAp.pwd = pwd;
    saveFile();
}

void BruceConfig::addWifiCredential(const String &ssid, const String &pwd) {
    wifi[ssid] = pwd;
    saveFile();
}

String BruceConfig::getWifiPassword(const String &ssid) const {
    auto it = wifi.find(ssid);
    if (it != wifi.end()) return it->second;
    return "";
}

void BruceConfig::addEvilWifiName(String value) {
    evilWifiNames.insert(value);
    saveFile();
}

void BruceConfig::removeEvilWifiName(String value) {
    evilWifiNames.erase(value);
    saveFile();
}

void BruceConfig::setEvilEndpointCreds(String value) {
    evilPortalEndpoints.getCredsEndpoint = value;
    validateEvilEndpointCreds();
    saveFile();
}

void BruceConfig::validateEvilEndpointCreds() {
    if (evilPortalEndpoints.getCredsEndpoint == evilPortalEndpoints.setSsidEndpoint) {
        // on collision reset to defaults
        evilPortalEndpoints.getCredsEndpoint = "/creds";
    }
    if (evilPortalEndpoints.getCredsEndpoint[0] != '/') {
        evilPortalEndpoints.getCredsEndpoint = '/' + evilPortalEndpoints.getCredsEndpoint;
    }
}

void BruceConfig::setEvilEndpointSsid(String value) {
    evilPortalEndpoints.setSsidEndpoint = value;
    validateEvilEndpointCreds();
    saveFile();
}

void BruceConfig::validateEvilEndpointSsid() {
    if (evilPortalEndpoints.getCredsEndpoint == evilPortalEndpoints.setSsidEndpoint) {
        // on collision reset to defaults
        evilPortalEndpoints.setSsidEndpoint = "/ssid";
    }
    if (evilPortalEndpoints.setSsidEndpoint[0] != '/') {
        evilPortalEndpoints.setSsidEndpoint = '/' + evilPortalEndpoints.setSsidEndpoint;
    }
}

void BruceConfig::setEvilAllowEndpointDisplay(bool value) {
    evilPortalEndpoints.showEndpoints = value;
    saveFile();
}

void BruceConfig::setEvilAllowGetCreds(bool value) {
    evilPortalEndpoints.allowGetCreds = value;
    saveFile();
}

void BruceConfig::setEvilAllowSetSsid(bool value) {
    evilPortalEndpoints.allowSetSsid = value;
    saveFile();
}

void BruceConfig::setEvilPasswordMode(EvilPortalPasswordMode value) {
    evilPortalPasswordMode = value;
    saveFile();
}

void BruceConfig::validateEvilPasswordMode() {
    if (evilPortalPasswordMode < 0 || evilPortalPasswordMode > 2) evilPortalPasswordMode = FULL_PASSWORD;
}

void BruceConfig::setStartupApp(String value) {
    startupApp = value;
    saveFile();
}

void BruceConfig::setStartupAppLuaScript(String value) {
    startupAppLuaScript = value;
    saveFile();
}

void BruceConfig::setWigleBasicToken(String value) {
    wigleBasicToken = value;
    saveFile();
}

void BruceConfig::setDevMode(int value) {
    devMode = value;
    validateDevModeValue();
    saveFile();
}

void BruceConfig::validateDevModeValue() {
    if (devMode > 1) devMode = 1;
}

void BruceConfig::setColorInverted(int value) {
    colorInverted = value;
    validateColorInverted();
    saveFile();
}

void BruceConfig::validateColorInverted() {
    if (colorInverted > 1) colorInverted = 1;
}

void BruceConfig::setBadUSBBLEKeyboardLayout(int value) {
    badUSBBLEKeyboardLayout = value;
    validateBadUSBBLEKeyboardLayout();
    saveFile();
}

void BruceConfig::validateBadUSBBLEKeyboardLayout() {
    if (badUSBBLEKeyboardLayout < 0 || badUSBBLEKeyboardLayout > 13) badUSBBLEKeyboardLayout = 0;
}

void BruceConfig::setBadUSBBLEKeyDelay(uint16_t value) {
    badUSBBLEKeyDelay = value;
    validateBadUSBBLEKeyDelay();
    saveFile();
}

void BruceConfig::validateBadUSBBLEKeyDelay() {
    if (badUSBBLEKeyDelay > 500) badUSBBLEKeyDelay = 500;
}

void BruceConfig::setBadUSBBLEShowOutput(bool value) {
    badUSBBLEShowOutput = value;
    saveFile();
}
void BruceConfig::addMifareKey(String value) { MifareKeysManager::addKey(mifareKeys, value); }

void BruceConfig::validateMifareKeysItems() { MifareKeysManager::validateKeys(mifareKeys); }

void BruceConfig::addDisabledMenu(String value) {
    // TODO: check if duplicate
    disabledMenus.push_back(value);
    saveFile();
}

void BruceConfig::removeDisabledMenu(String value) {
    auto it = std::find(disabledMenus.begin(), disabledMenus.end(), value);
    if (it != disabledMenus.end()) {
        disabledMenus.erase(it);
        saveFile();
    }
}

void BruceConfig::addQrCodeEntry(const String &menuName, const String &content) {
    qrCodes.push_back({menuName, content});
    saveFile();
}

void BruceConfig::removeQrCodeEntry(const String &menuName) {
    size_t writeIndex = 0;

    for (size_t readIndex = 0; readIndex < qrCodes.size(); ++readIndex) {
        const QrCodeEntry &entry = qrCodes[readIndex];

        if (entry.menuName != menuName) {
            if (writeIndex != readIndex) { qrCodes[writeIndex] = std::move(qrCodes[readIndex]); }
            ++writeIndex;
        }
    }

    if (writeIndex < qrCodes.size()) { qrCodes.erase(qrCodes.begin() + writeIndex, qrCodes.end()); }

    saveFile();
}

void BruceConfig::addWebUISession(const String &token) {
    webUISessions.push_back(token);
    // Limit to maximum 5 sessions - remove oldest (first element) if exceeded
    if (webUISessions.size() > 5) { webUISessions.erase(webUISessions.begin()); }
    saveFile();
}

void BruceConfig::removeWebUISession(const String &token) {
    for (auto it = webUISessions.begin(); it != webUISessions.end(); ++it) {
        if (*it == token) {
            webUISessions.erase(it);
            break;
        }
    }
    saveFile();
}

bool BruceConfig::isValidWebUISession(const String &token) {
    auto it = std::find(webUISessions.begin(), webUISessions.end(), token);

    if (it == webUISessions.end()) {
        return false; // Token not found
    }

    // Check if token is already at the end (most recent position)
    if (it == webUISessions.end() - 1) {
        return true; // Already most recent, no changes needed
    }

    // Move token to end and save
    webUISessions.erase(it);
    webUISessions.push_back(token);

    // Limit to maximum 10 sessions
    if (webUISessions.size() > 10) { webUISessions.erase(webUISessions.begin()); }

    saveFile();
    return true;
}

String BruceConfig::encryptString(const String &input) const {
    if (input.isEmpty()) return "";
    String output = ENC_V2_PREFIX;
    String cryptoKey = getCryptoKey();
    for (size_t i = 0; i < input.length(); i++) {
        char c = input[i] ^ cryptoKey[i % cryptoKey.length()];
        char hex[3];
        snprintf(hex, sizeof(hex), "%02x", (unsigned char)c);
        output += hex;
    }
    return output;
}

String BruceConfig::decryptString(const String &input) const {
    if (input.startsWith(ENC_V2_PREFIX)) {
        String hexData = input.substring(ENC_V2_PREFIX.length());
        String output = "";
        String cryptoKey = getCryptoKey();
        for (size_t i = 0; i < hexData.length(); i += 2) {
            String byteString = hexData.substring(i, i + 2);
            char c = (char)strtol(byteString.c_str(), nullptr, 16);
            output += (char)(c ^ cryptoKey[(i / 2) % cryptoKey.length()]);
        }
        return output;
    } else if (input.startsWith(ENC_PREFIX)) {
        String hexData = input.substring(ENC_PREFIX.length());
        String output = "";
        for (size_t i = 0; i < hexData.length(); i += 2) {
            String byteString = hexData.substring(i, i + 2);
            char c = (char)strtol(byteString.c_str(), nullptr, 16);
            output += (char)(c ^ CRYPTO_KEY[(i / 2) % CRYPTO_KEY.length()]);
        }
        return output;
    }
    return input;
}


bool BruceConfig::setSetting(const String& name, const String& value) {
    static const std::map<String, std::function<void(BruceConfig*, const String&)>> settingsMap = {
        {"priColor", [](BruceConfig* cfg, const String& val) { cfg->setUiColor(val.toInt()); }},
        {"dimmerSet", [](BruceConfig* cfg, const String& val) { cfg->setDimmer(val.toInt()); }},
        {"bright", [](BruceConfig* cfg, const String& val) { cfg->setBright(val.toInt()); }},
        {"tmz", [](BruceConfig* cfg, const String& val) { cfg->setTmz(val.toFloat()); }},
        {"soundEnabled", [](BruceConfig* cfg, const String& val) { cfg->setSoundEnabled(val.toInt()); }},
        {"wifiAtStartup", [](BruceConfig* cfg, const String& val) { cfg->setWifiAtStartup(val.toInt()); }},
        {"webUI", [](BruceConfig* cfg, const String& val) {
            cfg->setWebUICreds(
                val.substring(0, val.indexOf(",")),
                val.substring(val.indexOf(",") + 1)
            );
        }},
        {"wifiAp", [](BruceConfig* cfg, const String& val) {
            cfg->setWifiApCreds(
                val.substring(0, val.indexOf(",")),
                val.substring(val.indexOf(",") + 1)
            );
        }},
        {"wifi", [](BruceConfig* cfg, const String& val) {
            cfg->addWifiCredential(
                val.substring(0, val.indexOf(",")),
                val.substring(val.indexOf(",") + 1)
            );
        }},
        {"wigleBasicToken", [](BruceConfig* cfg, const String& val) { cfg->setWigleBasicToken(val); }},
        {"devMode", [](BruceConfig* cfg, const String& val) { cfg->setDevMode(val.toInt()); }},
        {"disabledMenus", [](BruceConfig* cfg, const String& val) { cfg->addDisabledMenu(val); }}
    };

    auto it = settingsMap.find(name);
    if (it != settingsMap.end()) {
        it->second(this, value);
        return true;
    }
    return false;
}
