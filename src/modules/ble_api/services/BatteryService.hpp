#pragma once
#if !defined(LITE_VERSION)
#include "WillyBLEService.hpp"
#include <NimBLEServer.h>

class BatteryService : public WillyBLEService {
    NimBLECharacteristic *battery_char = nullptr;
    TaskHandle_t battery_task_handle = nullptr;

public:
    BatteryService(/* args */);
    ~BatteryService() override;
    void setup(NimBLEServer *pServer) override;
    void end() override;
};
#endif
