#include "serialcmds.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "utils.h"
#include <globals.h>

QueueHandle_t cmdQueue = nullptr;
QueueHandle_t rspQueue = nullptr;
TaskHandle_t serialcmdsTaskHandle;

struct CmdPacket {
    char text[SAFE_STACK_BUFFER_SIZE];
};
bool parseSerialCommand(const String &command, bool waitForResponse) {
    if (!cmdQueue || !rspQueue) {
        Serial.println("Command or response queue not initialized");
        return false;
    }
    CmdPacket packet;
    memset(&packet, 0, sizeof(packet));
    strncpy(packet.text, command.c_str(), sizeof(packet.text) - 1);

    // Enqueue the command packet for processing
    if (xQueueSend(cmdQueue, &packet, 0) != pdTRUE) {
        Serial.println("Failed to send command to queue");
        return false;
    }
    if (!waitForResponse) { return true; }
    // Wait for the response
    bool result;
    if (xQueueReceive(rspQueue, &result, pdMS_TO_TICKS(20))) { return result; }
    Serial.println("Failed to receive command response");
    return false;
}

void handleSerialCommands(SerialCli &serialCli) {
    CmdPacket packet;
    if (cmdQueue && rspQueue) {
        if (xQueueReceive(cmdQueue, &packet, 0) == pdTRUE) {
            // Trim in-place no buffer do pacote: evita criar uma String
            // (alocacao no heap) por comando recebido.
            char *cmdString = packet.text;
            while (*cmdString && isspace((unsigned char)*cmdString)) cmdString++;
            size_t cmdLen = strlen(cmdString);
            while (cmdLen > 0 && isspace((unsigned char)cmdString[cmdLen - 1])) cmdString[--cmdLen] = '\0';

            bool result = false;
            if (cmdLen > 0) {
                result = serialCli.parse(String(cmdString));
            }
            xQueueSend(rspQueue, &result, 0);
            if (cmdLen > 0) {
                Serial.printf("COMMAND: %s\n", cmdString);
                Serial.printf("[CLI] Result: %s\n", result ? "TRUE" : "FALSE");
            }
        }
    }
    if (!serialDevice->available()) return;

    String cmd_str = serialDevice->readStringUntil('\n');
    cmd_str.trim();
    if (cmd_str.length() > 0) {
        // printf em vez de "COMMAND: " + cmd_str: evita a concatenacao.
        Serial.printf("COMMAND: %s\n", cmd_str.c_str());
        serialCli.parse(cmd_str);
    }
    serialDevice->print("# "); // prompt
    backToMenu();              // forced menu redrawn
}

void _serialCmdsTaskLoop(void *pvParameters) {
    Serial.begin(115200);
    while (1) {
        handleSerialCommands(serialCli);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void startSerialCommandsHandlerTask(bool initQueues) {
    if (initQueues) {
        cmdQueue = xQueueCreate(2, sizeof(CmdPacket));
        rspQueue = xQueueCreate(2, sizeof(bool));
    }

    xTaskCreatePinnedToCore(
        _serialCmdsTaskLoop,         // Function to implement the task
        "serialcmds",                // Name of the task (any string)
        SERIAL_CMDS_TASK_STACK_SIZE, // Stack size in bytes
        NULL, // This is a pointer to the parameter that will be passed to the new task. We are not using it
              // here and therefore it is set to NULL.
        1,    // Priority of the task (Lowered to avoid UI starvation)
        &serialcmdsTaskHandle, // Task handle (optional, can be NULL).
#if SOC_CPU_CORES_NUM > 1
        1 // Core where the task should run. By default, all your Arduino code runs on Core 1 and the Wi-Fi
          // and RF functions
#else
        0 // Core where the task should run. ESP32-C5 has only one core
#endif
    ); // (these are usually hidden from the Arduino environment) use the Core 0.
    if (!serialcmdsTaskHandle) { Serial.println("Failed to create Serial Commands Handler task"); }
}
