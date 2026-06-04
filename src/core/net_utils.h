#include <HTTPClient.h>
#include <WiFi.h>

bool internetConnection();

String getManufacturer(const String &mac);


void stringToMAC(const std::string &macStr, uint8_t MAC[6]);

// Função para converter IP para string
String ipToString(const uint8_t *ip);

// Função para converter MAC para string
String macToString(const uint8_t *mac);

// Helper function to convert IPAddress to byte array
void toBytes(IPAddress ip, uint8_t *bytes);
