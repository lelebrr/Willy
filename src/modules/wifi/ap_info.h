#ifndef __AP_INFO_H__
#define __AP_INFO_H__

#include <WiFi.h>
#include "esp_wifi.h"
#include "globals.h"

void displayAPInfo(const wifi_ap_record_t &record);

#endif
