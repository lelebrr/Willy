#include "modules/wifi/tcp_utils.h"
#include "core/display.h"
#include "core/wifi/wifi_common.h"
#include "globals.h"
#include <WiFi.h>

bool inputMode;

void listenTcpPort(int port) {
  if (!wifiConnected)
    wifiConnectMenu();

  WiFiClient tcpClient;
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  int portNumberInt = port;
  if (portNumberInt == 0) {
    String portNumber = num_keyboard("", 5, "Porta TCP p/ ouvir");
    if (portNumber.length() == 0) {
      displayError("Sem numero porta, saindo", true);
      return;
    }
    portNumberInt = atoi(portNumber.c_str());
    if (portNumberInt == 0) {
      displayError("Porta invalida, saindo", true);
      return;
    }
  }

  WiFiServer server(portNumberInt);
  server.begin();

  tft.println("Ouvindo...");
  tft.print(WiFi.localIP().toString().c_str());
  tft.println(":" + String(portNumberInt));

  for (;;) {
    WiFiClient client = server.accept(); // Wait for a client to connect

    if (client) {
      Serial.println("Client connected");
      tft.println("Cliente conectado");

      while (client.connected()) {
        if (inputMode) {
          String keyString = keyboard("", 16, "enviar dados, q=sair");
          if (keyString == "q") {
            displayError("Saindo Listener", true);
            client.stop();
            server.stop();
            return;
          }
          inputMode = false;
          tft.fillScreen(TFT_BLACK);
          tft.setCursor(0, 0);
          if (keyString.length() > 0) {
            client.print(keyString); // Send the entire string to the client
            Serial.print(keyString);
          }
        } else {
          if (client.available()) {
            char incomingChar =
                client.read(); // Read one byte at time from the client
            tft.print(incomingChar);
            Serial.print(incomingChar);
          }
          if (check(SelPress)) {
            inputMode = true;
          }
        }
      }
      client.stop();
      Serial.println("Client disconnected");
      displayError("Cliente desconectado", true);
    }
    if (check(EscPress)) {
      displayError("Saindo Listener", true);
      server.stop();
      break;
    }
  }
}

void clientTCP() {
  if (!wifiConnected)
    wifiConnectMenu();

  String serverIP = keyboard("", 15, "Digite IP servidor");
  String portString = num_keyboard("", 5, "Digite Porta servidor");
  int portNumber = atoi(portString.c_str());

  if (serverIP.length() == 0 || portNumber == 0) {
    displayError("IP ou Porta invalido", true);
    return;
  }

  WiFiClient client;
  if (!client.connect(serverIP.c_str(), portNumber)) {
    displayError("Conexao falhou", true);
    return;
  }

  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.println("Conectado a:");
  tft.println(serverIP + ":" + portString);
  Serial.println("Connected to server");

  while (client.connected()) {
    if (inputMode) {
      String keyString = keyboard("", 16, "enviar dados");
      inputMode = false;
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0, 0);
      if (keyString.length() > 0) {
        client.print(keyString);
        Serial.print(keyString);
      }
    } else {
      if (client.available()) {
        char incomingChar = client.read();
        tft.print(incomingChar);
        Serial.print(incomingChar);
      }
      if (check(SelPress)) {
        inputMode = true;
      }
    }
    if (check(EscPress)) {
      displayError("Saindo Cliente", true);
      client.stop();
      break;
    }
  }

  displayError("Conexao fechada.", true);
  Serial.println("Connection closed.");
  client.stop();
}

void listenUdpPort() {
  if (!wifiConnected)
    wifiConnectMenu();

  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  String portNumber = num_keyboard("", 5, "Porta UDP p/ ouvir");
  if (portNumber.length() == 0) {
    displayError("Sem numero porta, saindo", true);
    return;
  }
  int portNumberInt = atoi(portNumber.c_str());
  if (portNumberInt == 0) {
    displayError("Porta invalida, saindo", true);
    return;
  }

  WiFiUDP udp;
  udp.begin(portNumberInt);

  tft.println("Ouvindo UDP...");
  tft.print(WiFi.localIP().toString().c_str());
  tft.println(":" + portNumber);

  char packetBuffer[255];
  while (!check(EscPress)) {
    int packetSize = udp.parsePacket();
    if (packetSize) {
      int len = udp.read(packetBuffer, 255);
      if (len > 0) {
        packetBuffer[len] = 0;
        tft.println("Recebido: " + String(packetBuffer));
        Serial.println("Received: " + String(packetBuffer));
      }
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }

  udp.stop();
  displayError("Saindo Listener UDP", true);
}

void clientUDP() {
  if (!wifiConnected)
    wifiConnectMenu();

  String serverIP = keyboard("", 15, "Digite IP servidor UDP");
  String portString = num_keyboard("", 5, "Digite Porta servidor UDP");
  int portNumber = atoi(portString.c_str());

  if (serverIP.length() == 0 || portNumber == 0) {
    displayError("IP ou Porta invalido", true);
    return;
  }

  WiFiUDP udp;
  udp.begin(portNumber);

  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.println("Conectado a UDP:");
  tft.println(serverIP + ":" + portString);
  Serial.println("Connected to UDP server");

  while (!check(EscPress)) {
    if (inputMode) {
      String keyString = keyboard("", 16, "enviar dados UDP");
      inputMode = false;
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0, 0);
      if (keyString.length() > 0) {
        udp.beginPacket(serverIP.c_str(), portNumber);
        udp.print(keyString);
        udp.endPacket();
        Serial.print("Sent: " + keyString);
      }
    } else {
      if (check(SelPress)) {
        inputMode = true;
      }
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }

  udp.stop();
  displayError("Saindo Cliente UDP", true);
}
