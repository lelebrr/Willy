#include "config.h"
#include "mifare_keys_manager.h"
#include "sd_functions.h"
#include <esp_pm.h>

const String CRYPTO_KEY = "WillyFirmwareCoreRefinement";
const String ENC_PREFIX = "_ENC_";

JsonDocument WillyConfig::toJson() const {
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
  for (size_t i = 0; i < webUISessions.size(); i++) {
    _webUISessions[String(i + 1)] = encryptString(webUISessions[i]);
  }

  JsonObject _wifiAp = setting["wifiAp"].to<JsonObject>();
  _wifiAp["ssid"] = wifiAp.ssid;
  _wifiAp["ssid"] = wifiAp.ssid;
  _wifiAp["pwd"] = encryptString(wifiAp.pwd);
  setting["wifiMAC"] = wifiMAC; //@IncursioHack

  JsonArray _evilWifiNames = setting["evilWifiNames"].to<JsonArray>();
  for (auto key : evilWifiNames)
    _evilWifiNames.add(key);

  JsonObject _evilWifiEndpoints = setting["evilWifiEndpoints"].to<JsonObject>();
  _evilWifiEndpoints["getCredsEndpoint"] = evilPortalEndpoints.getCredsEndpoint;
  _evilWifiEndpoints["setSsidEndpoint"] = evilPortalEndpoints.setSsidEndpoint;
  _evilWifiEndpoints["showEndpoints"] = evilPortalEndpoints.showEndpoints;
  _evilWifiEndpoints["allowSetSsid"] = evilPortalEndpoints.allowSetSsid;
  _evilWifiEndpoints["allowGetCreds"] = evilPortalEndpoints.allowGetCreds;

  setting["evilWifiPasswordMode"] = evilPortalPasswordMode;

  JsonObject _wifi = setting["wifi"].to<JsonObject>();
  for (const auto &pair : wifi) {
    _wifi[pair.first] = encryptString(pair.second);
  }

  setting["startupApp"] = startupApp;
  setting["startupAppLuaScript"] = startupAppLuaScript;
  setting["wigleBasicToken"] = encryptString(wigleBasicToken);
  setting["devMode"] = devMode;
  setting["colorInverted"] = colorInverted;

  setting["badUSBBLEKeyboardLayout"] = badUSBBLEKeyboardLayout;
  setting["badUSBBLEKeyDelay"] = badUSBBLEKeyDelay;
  setting["badUSBBLEShowOutput"] = badUSBBLEShowOutput;

  JsonArray dm = setting["disabledMenus"].to<JsonArray>();
  for (size_t i = 0; i < disabledMenus.size(); i++) {
    dm.add(disabledMenus[i]);
  }

  JsonArray qrArray = setting["qrCodes"].to<JsonArray>();
  for (const auto &entry : qrCodes) {
    JsonObject qrEntry = qrArray.add<JsonObject>();
    qrEntry["menuName"] = entry.menuName;
    qrEntry["content"] = entry.content;
  }

  return jsonDoc;
}

void WillyConfig::fromFile(bool checkFS) {
  FS *fs;
  if (checkFS) {
    if (!getFsStorage(fs)) {
      log_i("Fail getting filesystem for config");
      return;
    }
  } else {
    if (checkLittleFsSize())
      fs = &LittleFS;
    else
      return;
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
  Serial.println("Starting deserializeJson in WillyConfig...");
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
    for (JsonPair kv : webUISessionsObj) {
      webUISessions.push_back(decryptString(kv.value().as<String>()));
    }
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
    for (JsonPair kv : wifiObj)
      wifi[kv.key().c_str()] = decryptString(kv.value().as<String>());
  } else {
    count++;
    log_e("Fail");
  }

  if (!setting["evilWifiNames"].isNull()) {
    evilWifiNames.clear();
    JsonArray _evilWifiNames = setting["evilWifiNames"].as<JsonArray>();
    for (JsonVariant key : _evilWifiNames)
      evilWifiNames.insert(key.as<String>());
  } else {
    count++;
    log_e("Fail");
  }

  if (!setting["evilWifiEndpoints"].isNull()) {
    JsonObject evilPortalEndpointsObj =
        setting["evilWifiEndpoints"].as<JsonObject>();
    evilPortalEndpoints.getCredsEndpoint =
        evilPortalEndpointsObj["getCredsEndpoint"].as<String>();
    evilPortalEndpoints.setSsidEndpoint =
        evilPortalEndpointsObj["setSsidEndpoint"].as<String>();
    evilPortalEndpoints.showEndpoints =
        evilPortalEndpointsObj["showEndpoints"].as<bool>();
    evilPortalEndpoints.allowSetSsid =
        evilPortalEndpointsObj["allowSetSsid"].as<bool>();
    evilPortalEndpoints.allowGetCreds =
        evilPortalEndpointsObj["allowGetCreds"].as<bool>();
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
    for (JsonVariant e : dm) {
      disabledMenus.push_back(e.as<String>());
    }
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
  if (count > 0)
    saveFile();

  // Load MIFARE keys (loading via manager)
  MifareKeysManager::ensureLoaded(mifareKeys);

  log_i("Using config from file");
}

void WillyConfig::saveFile() {
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
  if (serializeJsonPretty(jsonDoc, file) < 5)
    log_e("Failed to write config file");
  else
    log_i("config file written successfully");

  file.close();

  if (setupSdCard())
    copyToFs(LittleFS, SD, filepath, false);
}

void WillyConfig::factoryReset() {
  FS *fs = &LittleFS;
  fs->rename(String(filepath), "/bak." + String(filepath).substring(1));
  if (setupSdCard())
    SD.rename(String(filepath), "/bak." + String(filepath).substring(1));
  ESP.restart();
}

void WillyConfig::validateConfig() {
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

void WillyConfig::setUiColor(uint16_t primary, uint16_t *secondary,
                             uint16_t *background) {
  WillyTheme::_setUiColor(primary, secondary, background);
  saveFile();
}

void WillyConfig::setDimmer(int value) {
  dimmerSet = value;
  validateDimmerValue();
  saveFile();
}

void WillyConfig::validateDimmerValue() {
  if (dimmerSet < 0)
    dimmerSet = 10;
  if (dimmerSet > 60)
    dimmerSet = 0;
}

void WillyConfig::setBright(uint8_t value) {
  bright = value;
  validateBrightValue();
  saveFile();
}

void WillyConfig::validateBrightValue() {
  if (bright > 100)
    bright = 100;
}

void WillyConfig::setAutomaticTimeUpdateViaNTP(bool value) {
  automaticTimeUpdateViaNTP = value;
  saveFile();
}

void WillyConfig::setTmz(float value) {
  tmz = value;
  validateTmzValue();
  saveFile();
}

void WillyConfig::validateTmzValue() {
  if (tmz < -12 || tmz > 14)
    tmz = 0;
}

void WillyConfig::setDST(bool value) {
  dst = value;
  saveFile();
}

void WillyConfig::setClock24Hr(bool value) {
  clock24hr = value;
  saveFile();
}

void WillyConfig::setSoundEnabled(int value) {
  soundEnabled = value;
  validateSoundEnabledValue();
  saveFile();
}

void WillyConfig::setSoundVolume(int value) {
  soundVolume = value;
  validateSoundVolumeValue();
  saveFile();
}

void WillyConfig::validateSoundEnabledValue() {
  if (soundEnabled > 1)
    soundEnabled = 1;
}

void WillyConfig::validateSoundVolumeValue() {
  if (soundVolume > 100)
    soundVolume = 100;
}

void WillyConfig::setWifiAtStartup(int value) {
  wifiAtStartup = value;
  validateWifiAtStartupValue();
  saveFile();
}

void WillyConfig::validateWifiAtStartupValue() {
  if (wifiAtStartup > 1)
    wifiAtStartup = 1;
}

#ifdef HAS_RGB_LED
void WillyConfig::setLedBright(int value) {
  ledBright = value;
  validateLedBrightValue();
  saveFile();
}

void WillyConfig::validateLedBrightValue() {
  ledBright = max(0, min(100, ledBright));
}

void WillyConfig::setLedColor(uint32_t value) {
  ledColor = value;
  validateLedColorValue();
  saveFile();
}

void WillyConfig::validateLedColorValue() {
  ledColor = max<uint32_t>(0, min<uint32_t>(0xFFFFFFFF, ledColor));
}

void WillyConfig::setLedBlinkEnabled(int value) {
  ledBlinkEnabled = value;
  validateLedBlinkEnabledValue();
  saveFile();
}

void WillyConfig::validateLedBlinkEnabledValue() {
  if (ledBlinkEnabled > 1)
    ledBlinkEnabled = 1;
}

void WillyConfig::setLedEffect(int value) {
  ledEffect = value;
  validateLedEffectValue();
  saveFile();
}

void WillyConfig::validateLedEffectValue() {
  if (ledEffect < 0 || ledEffect > 5)
    ledEffect = 0;
}

void WillyConfig::setLedEffectSpeed(int value) {
  ledEffectSpeed = value;
  validateLedEffectSpeedValue();
  saveFile();
}

void WillyConfig::validateLedEffectSpeedValue() {
#ifdef HAS_ENCODER_LED
  if (ledEffectSpeed > 11)
    ledEffectSpeed = 11;
#else
  if (ledEffectSpeed > 10)
    ledEffectSpeed = 10;
#endif
  if (ledEffectSpeed < 0)
    ledEffectSpeed = 1;
}

void WillyConfig::setLedEffectDirection(int value) {
  ledEffectDirection = value;
  validateLedEffectDirectionValue();
  saveFile();
}

void WillyConfig::validateLedEffectDirectionValue() {
  if (ledEffectDirection > 1 || ledEffectDirection == 0)
    ledEffectDirection = 1;
  if (ledEffectDirection < -1)
    ledEffectDirection = -1;
}
#endif

void WillyConfig::setWebUICreds(const String &usr, const String &pwd) {
  webUI.user = usr;
  webUI.pwd = pwd;
  saveFile();
}

void WillyConfig::setWifiApCreds(const String &ssid, const String &pwd) {
  wifiAp.ssid = ssid;
  wifiAp.pwd = pwd;
  saveFile();
}

void WillyConfig::addWifiCredential(const String &ssid, const String &pwd) {
  wifi[ssid] = pwd;
  saveFile();
}

String WillyConfig::getWifiPassword(const String &ssid) const {
  auto it = wifi.find(ssid);
  if (it != wifi.end())
    return it->second;
  return "";
}

void WillyConfig::addEvilWifiName(String value) {
  evilWifiNames.insert(value);
  saveFile();
}

void WillyConfig::removeEvilWifiName(String value) {
  evilWifiNames.erase(value);
  saveFile();
}

void WillyConfig::setEvilEndpointCreds(String value) {
  evilPortalEndpoints.getCredsEndpoint = value;
  validateEvilEndpointCreds();
  saveFile();
}

void WillyConfig::validateEvilEndpointCreds() {
  if (evilPortalEndpoints.getCredsEndpoint ==
      evilPortalEndpoints.setSsidEndpoint) {
    // on collision reset to defaults
    evilPortalEndpoints.getCredsEndpoint = "/creds";
  }
  if (evilPortalEndpoints.getCredsEndpoint[0] != '/') {
    evilPortalEndpoints.getCredsEndpoint =
        '/' + evilPortalEndpoints.getCredsEndpoint;
  }
}

void WillyConfig::setEvilEndpointSsid(String value) {
  evilPortalEndpoints.setSsidEndpoint = value;
  validateEvilEndpointCreds();
  saveFile();
}

void WillyConfig::validateEvilEndpointSsid() {
  if (evilPortalEndpoints.getCredsEndpoint ==
      evilPortalEndpoints.setSsidEndpoint) {
    // on collision reset to defaults
    evilPortalEndpoints.setSsidEndpoint = "/ssid";
  }
  if (evilPortalEndpoints.setSsidEndpoint[0] != '/') {
    evilPortalEndpoints.setSsidEndpoint =
        '/' + evilPortalEndpoints.setSsidEndpoint;
  }
}

void WillyConfig::setEvilAllowEndpointDisplay(bool value) {
  evilPortalEndpoints.showEndpoints = value;
  saveFile();
}

void WillyConfig::setEvilAllowGetCreds(bool value) {
  evilPortalEndpoints.allowGetCreds = value;
  saveFile();
}

void WillyConfig::setEvilAllowSetSsid(bool value) {
  evilPortalEndpoints.allowSetSsid = value;
  saveFile();
}

void WillyConfig::setEvilPasswordMode(EvilPortalPasswordMode value) {
  evilPortalPasswordMode = value;
  saveFile();
}

void WillyConfig::validateEvilPasswordMode() {
  if (evilPortalPasswordMode < 0 || evilPortalPasswordMode > 2)
    evilPortalPasswordMode = FULL_PASSWORD;
}

void WillyConfig::setStartupApp(String value) {
  startupApp = value;
  saveFile();
}

void WillyConfig::setStartupAppLuaScript(String value) {
  startupAppLuaScript = value;
  saveFile();
}

void WillyConfig::setWigleBasicToken(String value) {
  wigleBasicToken = value;
  saveFile();
}

void WillyConfig::setDevMode(int value) {
  devMode = value;
  validateDevModeValue();
  saveFile();
}

void WillyConfig::validateDevModeValue() {
  if (devMode > 1)
    devMode = 1;
}

void WillyConfig::setColorInverted(int value) {
  colorInverted = value;
  validateColorInverted();
  saveFile();
}

void WillyConfig::validateColorInverted() {
  if (colorInverted > 1)
    colorInverted = 1;
}

void WillyConfig::setBadUSBBLEKeyboardLayout(int value) {
  badUSBBLEKeyboardLayout = value;
  validateBadUSBBLEKeyboardLayout();
  saveFile();
}

void WillyConfig::validateBadUSBBLEKeyboardLayout() {
  if (badUSBBLEKeyboardLayout < 0 || badUSBBLEKeyboardLayout > 13)
    badUSBBLEKeyboardLayout = 0;
}

void WillyConfig::setBadUSBBLEKeyDelay(uint16_t value) {
  badUSBBLEKeyDelay = value;
  validateBadUSBBLEKeyDelay();
  saveFile();
}

void WillyConfig::validateBadUSBBLEKeyDelay() {
  if (badUSBBLEKeyDelay > 500)
    badUSBBLEKeyDelay = 500;
}

void WillyConfig::setBadUSBBLEShowOutput(bool value) {
  badUSBBLEShowOutput = value;
  saveFile();
}
void WillyConfig::addMifareKey(String value) {
  MifareKeysManager::addKey(mifareKeys, value);
}

void WillyConfig::validateMifareKeysItems() {
  MifareKeysManager::validateKeys(mifareKeys);
}

void WillyConfig::addDisabledMenu(String value) {
  // Check if duplicate
  for (const String &menu : disabledMenus) {
    if (menu == value) {
      return; // Already exists, do not add
    }
  }
  disabledMenus.push_back(value);
  saveFile();
}

void WillyConfig::addQrCodeEntry(const String &menuName,
                                 const String &content) {
  // Check if duplicate menuName
  for (const auto &qr : qrCodes) {
    if (qr.menuName == menuName) {
      return; // Already exists, do not add
    }
  }
  qrCodes.push_back({menuName, content});
  saveFile();
}

void WillyConfig::removeQrCodeEntry(const String &menuName) {
  size_t writeIndex = 0;

  for (size_t readIndex = 0; readIndex < qrCodes.size(); ++readIndex) {
    const QrCodeEntry &entry = qrCodes[readIndex];

    if (entry.menuName != menuName) {
      if (writeIndex != readIndex) {
        qrCodes[writeIndex] = std::move(qrCodes[readIndex]);
      }
      ++writeIndex;
    }
  }

  if (writeIndex < qrCodes.size()) {
    qrCodes.erase(qrCodes.begin() + writeIndex, qrCodes.end());
  }

  saveFile();
}

void WillyConfig::addWebUISession(const String &token) {
  webUISessions.push_back(token);
  // Limit to maximum 5 sessions - remove oldest (first element) if exceeded
  if (webUISessions.size() > 5) {
    webUISessions.erase(webUISessions.begin());
  }
  saveFile();
}

void WillyConfig::removeWebUISession(const String &token) {
  for (auto it = webUISessions.begin(); it != webUISessions.end(); ++it) {
    if (*it == token) {
      webUISessions.erase(it);
      break;
    }
  }
  saveFile();
}

bool WillyConfig::isValidWebUISession(const String &token) {
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
  if (webUISessions.size() > 10) {
    webUISessions.erase(webUISessions.begin());
  }

  saveFile();
  return true;
}

String WillyConfig::encryptString(const String &input) const {
  if (input.isEmpty())
    return "";
  String output = ENC_PREFIX;
  for (size_t i = 0; i < input.length(); i++) {
    char c = input[i] ^ CRYPTO_KEY[i % CRYPTO_KEY.length()];
    char hex[3];
    sprintf(hex, "%02x", (unsigned char)c);
    output += hex;
  }
  return output;
}

String WillyConfig::decryptString(const String &input) const {
  if (!input.startsWith(ENC_PREFIX))
    return input;
  String hexData = input.substring(ENC_PREFIX.length());
  String output = "";
  for (size_t i = 0; i < hexData.length(); i += 2) {
    String byteString = hexData.substring(i, i + 2);
    char c = (char)strtol(byteString.c_str(), nullptr, 16);
    output += (char)(c ^ CRYPTO_KEY[(i / 2) % CRYPTO_KEY.length()]);
  }
  return output;
}

bool WillyConfig::setSetting(const String &setting_name,
                             const String &setting_value) {
  // Configurações de brightness e dimmer
  if (setting_name == "dimmerSet") {
    setDimmer(setting_value.toInt());
    return true;
  }
  if (setting_name == "bright") {
    setBright(setting_value.toInt());
    return true;
  }

  // Configurações de tempo
  if (setting_name == "automaticTimeUpdateViaNTP") {
    setAutomaticTimeUpdateViaNTP(setting_value.toInt() != 0);
    return true;
  }
  if (setting_name == "tmz") {
    setTmz(setting_value.toFloat());
    return true;
  }
  if (setting_name == "dst") {
    setDST(setting_value.toInt() != 0);
    return true;
  }
  if (setting_name == "clock24hr") {
    setClock24Hr(setting_value.toInt() != 0);
    return true;
  }

  // Configurações de som
  if (setting_name == "soundEnabled") {
    setSoundEnabled(setting_value.toInt());
    return true;
  }
  if (setting_name == "soundVolume") {
    setSoundVolume(setting_value.toInt());
    return true;
  }

  // Configurações de WiFi
  if (setting_name == "wifiAtStartup") {
    setWifiAtStartup(setting_value.toInt());
    return true;
  }
  if (setting_name == "instantBoot") {
    setDevMode(setting_value.toInt());
    return true;
  }

#ifdef HAS_RGB_LED
  // Configurações de LED
  if (setting_name == "ledBright") {
    setLedBright(setting_value.toInt());
    return true;
  }
  if (setting_name == "ledColor") {
    setLedColor(strtoul(setting_value.c_str(), nullptr, 16));
    return true;
  }
  if (setting_name == "ledBlinkEnabled") {
    setLedBlinkEnabled(setting_value.toInt());
    return true;
  }
  if (setting_name == "ledEffect") {
    setLedEffect(setting_value.toInt());
    return true;
  }
  if (setting_name == "ledEffectSpeed") {
    setLedEffectSpeed(setting_value.toInt());
    return true;
  }
  if (setting_name == "ledEffectDirection") {
    setLedEffectDirection(setting_value.toInt());
    return true;
  }
#endif

  // Configurações de UI
  if (setting_name == "priColor" || setting_name == "secColor" ||
      setting_name == "bgColor") {
    uint32_t color32 = strtoul(setting_value.c_str(), nullptr, 16);
    uint16_t color16 = static_cast<uint16_t>(color32 & 0xFFFF);
    if (setting_name == "priColor") {
      setUiColor(color16, nullptr, nullptr);
    } else if (setting_name == "secColor") {
      uint16_t currentPri = priColor;
      setUiColor(currentPri, &color16, nullptr);
    } else if (setting_name == "bgColor") {
      uint16_t currentPri = priColor;
      uint16_t currentSec = secColor;
      setUiColor(currentPri, &currentSec, &color16);
    }
    return true;
  }

  // Configurações de WebUI
  if (setting_name == "webUI_user") {
    webUI.user = setting_value;
    saveFile();
    return true;
  }
  if (setting_name == "webUI_pwd") {
    webUI.pwd = setting_value;
    saveFile();
    return true;
  }

  // Configurações de WiFi AP
  if (setting_name == "wifiAp_ssid") {
    wifiAp.ssid = setting_value;
    saveFile();
    return true;
  }
  if (setting_name == "wifiAp_pwd") {
    wifiAp.pwd = setting_value;
    saveFile();
    return true;
  }

  // Configurações de aplicativo inicial
  if (setting_name == "startupApp") {
    setStartupApp(setting_value);
    return true;
  }
  if (setting_name == "startupAppLuaScript") {
    setStartupAppLuaScript(setting_value);
    return true;
  }

  // Configurações diversas
  if (setting_name == "wigleBasicToken") {
    setWigleBasicToken(setting_value);
    return true;
  }
  if (setting_name == "devMode") {
    setDevMode(setting_value.toInt());
    return true;
  }
  if (setting_name == "colorInverted") {
    setColorInverted(setting_value.toInt());
    return true;
  }

  // Configurações de BadUSB/BLE
  if (setting_name == "badUSBBLEKeyboardLayout") {
    setBadUSBBLEKeyboardLayout(setting_value.toInt());
    return true;
  }
  if (setting_name == "badUSBBLEKeyDelay") {
    setBadUSBBLEKeyDelay(setting_value.toInt());
    return true;
  }
  if (setting_name == "badUSBBLEShowOutput") {
    setBadUSBBLEShowOutput(setting_value.toInt() != 0);
    return true;
  }

  // Configurações de EvilPortal
  if (setting_name == "evilPortalEndpoints_getCredsEndpoint") {
    setEvilEndpointCreds(setting_value);
    return true;
  }
  if (setting_name == "evilPortalEndpoints_setSsidEndpoint") {
    setEvilEndpointSsid(setting_value);
    return true;
  }
  if (setting_name == "evilPortalEndpoints_showEndpoints") {
    setEvilAllowEndpointDisplay(setting_value.toInt() != 0);
    return true;
  }
  if (setting_name == "evilPortalEndpoints_allowSetSsid") {
    setEvilAllowSetSsid(setting_value.toInt() != 0);
    return true;
  }
  if (setting_name == "evilPortalEndpoints_allowGetCreds") {
    setEvilAllowGetCreds(setting_value.toInt() != 0);
    return true;
  }
  if (setting_name == "evilPortalPasswordMode") {
    int mode = setting_value.toInt();
    if (mode >= 0 && mode <= 2) {
      setEvilPasswordMode(static_cast<EvilPortalPasswordMode>(mode));
    } else {
      setEvilPasswordMode(FULL_PASSWORD);
    }
    return true;
  }

  // Configurações de RFID/MIFARE
  if (setting_name == "mifareKeys") {
    // Espera uma lista de chaves separadas por vírgula
    mifareKeys.clear();
    int start = 0;
    int end = setting_value.indexOf(',');
    while (end != -1) {
      String key = setting_value.substring(start, end);
      key.trim();
      if (key.length() > 0) {
        addMifareKey(key);
      }
      start = end + 1;
      end = setting_value.indexOf(',', start);
    }
    String lastKey = setting_value.substring(start);
    lastKey.trim();
    if (lastKey.length() > 0) {
      addMifareKey(lastKey);
    }
    return true;
  }

  // Configurações de tema
  if (setting_name == "themeFile") {
    themePath = setting_value;
    saveFile();
    return true;
  }
  if (setting_name == "themeOnSd") {
    theme.fs = setting_value.toInt();
    saveFile();
    return true;
  }

  // Configurações de WiFi MAC
  if (setting_name == "wifiMAC") {
    wifiMAC = setting_value;
    saveFile();
    return true;
  }

  // Configurações de sessões WebUI (lista separada por vírgulas)
  if (setting_name == "webUISessions") {
    webUISessions.clear();
    int start = 0;
    int end = setting_value.indexOf(',');
    while (end != -1) {
      String token = setting_value.substring(start, end);
      token.trim();
      if (token.length() > 0) {
        webUISessions.push_back(token);
      }
      start = end + 1;
      end = setting_value.indexOf(',', start);
    }
    String lastToken = setting_value.substring(start);
    lastToken.trim();
    if (lastToken.length() > 0) {
      webUISessions.push_back(lastToken);
    }
    saveFile();
    return true;
  }

  // Configurações de Evil WiFi Names (lista separada por vírgulas)
  if (setting_name == "evilWifiNames") {
    evilWifiNames.clear();
    int start = 0;
    int end = setting_value.indexOf(',');
    while (end != -1) {
      String name = setting_value.substring(start, end);
      name.trim();
      if (name.length() > 0) {
        evilWifiNames.insert(name);
      }
      start = end + 1;
      end = setting_value.indexOf(',', start);
    }
    String lastName = setting_value.substring(start);
    lastName.trim();
    if (lastName.length() > 0) {
      evilWifiNames.insert(lastName);
    }
    saveFile();
    return true;
  }

  // Configurações de WiFi credentials (espera formato: "ssid1:pwd1,ssid2:pwd2")
  if (setting_name == "wifi") {
    wifi.clear();
    int start = 0;
    int end = setting_value.indexOf(',');
    while (end != -1) {
      String pair = setting_value.substring(start, end);
      pair.trim();
      int colon = pair.indexOf(':');
      if (colon != -1) {
        String ssid = pair.substring(0, colon);
        String pwd = pair.substring(colon + 1);
        ssid.trim();
        pwd.trim();
        if (ssid.length() > 0) {
          wifi[ssid] = pwd;
        }
      }
      start = end + 1;
      end = setting_value.indexOf(',', start);
    }
    String lastPair = setting_value.substring(start);
    lastPair.trim();
    int colon = lastPair.indexOf(':');
    if (colon != -1) {
      String ssid = lastPair.substring(0, colon);
      String pwd = lastPair.substring(colon + 1);
      ssid.trim();
      pwd.trim();
      if (ssid.length() > 0) {
        wifi[ssid] = pwd;
      }
    }
    saveFile();
    return true;
  }

  // Configurações de menus desabilitados (lista separada por vírgulas)
  if (setting_name == "disabledMenus") {
    disabledMenus.clear();
    int start = 0;
    int end = setting_value.indexOf(',');
    while (end != -1) {
      String menu = setting_value.substring(start, end);
      menu.trim();
      if (menu.length() > 0) {
        disabledMenus.push_back(menu);
      }
      start = end + 1;
      end = setting_value.indexOf(',', start);
    }
    String lastMenu = setting_value.substring(start);
    lastMenu.trim();
    if (lastMenu.length() > 0) {
      disabledMenus.push_back(lastMenu);
    }
    saveFile();
    return true;
  }

  // Configurações de QR Codes (espera formato:
  // "menuName1:content1,menuName2:content2")
  if (setting_name == "qrCodes") {
    qrCodes.clear();
    int start = 0;
    int end = setting_value.indexOf(',');
    while (end != -1) {
      String entry = setting_value.substring(start, end);
      entry.trim();
      int colon = entry.indexOf(':');
      if (colon != -1) {
        String menuName = entry.substring(0, colon);
        String content = entry.substring(colon + 1);
        menuName.trim();
        content.trim();
        if (menuName.length() > 0 && content.length() > 0) {
          qrCodes.push_back({menuName, content});
        }
      }
      start = end + 1;
      end = setting_value.indexOf(',', start);
    }
    String lastEntry = setting_value.substring(start);
    lastEntry.trim();
    int colon = lastEntry.indexOf(':');
    if (colon != -1) {
      String menuName = lastEntry.substring(0, colon);
      String content = lastEntry.substring(colon + 1);
      menuName.trim();
      content.trim();
      if (menuName.length() > 0 && content.length() > 0) {
        qrCodes.push_back({menuName, content});
      }
    }
    saveFile();
    return true;
  }

  // Se não reconheceu a configuração, retorna false
  return false;
}
