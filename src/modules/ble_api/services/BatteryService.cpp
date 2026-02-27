#if !defined(LITE_VERSION)
#include "BatteryService.hpp"
#include <NimBLEDevice.h>
#include <NimBLEUtils.h>
#include <WiFi.h>
#include <globals.h>

BatteryService::BatteryService() : BruceBLEService() {}

BatteryService::~BatteryService() {}

void battery_handler_task(void *params) {
    NimBLECharacteristic *battery_char = static_cast<NimBLECharacteristic *>(params);
    while (true) {
        uint8_t val = 0;
        battery_char->setValue(&val, 1);

        vTaskDelay(pdMS_TO_TICKS(60000)); // Update battery every minute
    }
}

void BatteryService::setup(BLEServer *pServer) {

    pService = pServer->createService(BLEUUID((uint16_t)0x180F));

    battery_char = pService->createCharacteristic(
        (uint16_t)0x2A19, // Battery Level
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
    );

    // const uint8_t batLevel = getBattery();
    const uint8_t batLevel = 0;
    battery_char->setValue(&batLevel, 1); // initial value

    pService->start();
    pServer->getAdvertising()->addServiceUUID(pService->getUUID());

    xTaskCreate(
        battery_handler_task,
        "battery_ble_handler",
        2048,
        battery_char,
        tskIDLE_PRIORITY,
        &battery_task_handle
    );
}

void BatteryService::end() {
    if (battery_task_handle != nullptr) {
        vTaskDelete(battery_task_handle);
        battery_task_handle = nullptr;
    }
    if (pService != nullptr) {
        // NimBLE services do not have a stop() method; cleaned up on deinit
    }
}
#endif
