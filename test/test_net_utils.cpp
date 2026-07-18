#include "Arduino.h"
#include "HTTPClient.h"
#include "WiFi.h"
#include "../src/core/net_utils.h"

#include <iostream>
#include <cassert>

bool mock_internet_connected = true;
int mock_http_code = 200;
String mock_http_payload = "";
String mock_http_url = "";

bool WiFiClient::connect(const char* host, uint16_t port) {
    return mock_internet_connected;
}
void WiFiClient::stop() {}

void HTTPClient::begin(String url) {
    mock_http_url = url;
}
int HTTPClient::GET() {
    return mock_http_code;
}
void HTTPClient::end() {}
String HTTPClient::getString() {
    return mock_http_payload;
}

void test_no_internet() {
    mock_internet_connected = false;
    String res = getManufacturer("00:11:22:33:44:55");
    assert(res == "NO_INTERNET_ACCESS");
    std::cout << "test_no_internet passed\n";
}

void test_http_get_failed() {
    mock_internet_connected = true;
    mock_http_code = 404;
    String res = getManufacturer("00:11:22:33:44:55");
    assert(res == "GET failed");
    std::cout << "test_http_get_failed passed\n";
}

void test_get_manufacturer_success() {
    mock_internet_connected = true;
    mock_http_code = 200;
    mock_http_payload = "{\"success\":true,\"found\":true,\"macPrefix\":\"2C3358\",\"company\":\"Intel Corporate\",\"address\":\"Lot 8, Jalan\"}";
    String res = getManufacturer("2C:33:58:00:00:00");
    assert(res == "Intel Corporate");
    assert(mock_http_url == "http://api.maclookup.app/v2/macs/2C:33:58:00:00:00");
    std::cout << "test_get_manufacturer_success passed\n";
}

void test_get_manufacturer_unknown() {
    mock_internet_connected = true;
    mock_http_code = 200;
    mock_http_payload = "{\"success\":true,\"found\":false,\"macPrefix\":\"2C3358\",\"company\":\"\",\"address\":\"\"}";
    String res = getManufacturer("2C:33:58:00:00:00");
    assert(res == "UNKNOWN");
    std::cout << "test_get_manufacturer_unknown passed\n";
}

int main() {
    test_no_internet();
    test_http_get_failed();
    test_get_manufacturer_success();
    test_get_manufacturer_unknown();
    std::cout << "All net_utils tests passed!\n";
    return 0;
}
