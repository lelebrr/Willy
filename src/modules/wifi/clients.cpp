#ifndef LITE_VERSION
// SSH borrowed from https://github.com/m5stack/M5Cardputer :)

// TODO: Display is kinda glitchy :P figure out some way to show better outputs also

// SSH libs
#include "libssh_esp32.h"
#include <libssh/libssh.h>

// Telnet libs
#include "clients.h"
#include "core/display.h"
#include "core/mykeyboard.h"
#include "core/scrollableTextArea.h"
#include "core/wifi/wifi_common.h"
#include <Arduino.h>
#include <esp_event.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <globals.h>
#include <lwip/sockets.h>
#include <string.h>

// SSH server configuration (initialize as mpty strings)
String ssh_host = "";
String ssh_user = "";
String ssh_port = "";
String ssh_password = "";
char *ssh_port_char;

String commandBuffer = "> ";
int cursorY = 0;
const int lineHeight = 32; // 32
unsigned long lastKeyPressMillis = 0;
const unsigned long debounceDelay = 200; // Adjust debounce delay as needed

// ssh_bind sshbind = (ssh_bind)state->input;

// ssh_init sshbind;
ssh_session my_ssh_session;
ssh_channel channel_ssh;

char *stringTochar(String s) {
    if (s.length() == 0) {
        return nullptr;
    }

    static char arr[64]; // Enlarged to hold hostnames or IPv6 if needed
    memset(arr, 0, sizeof(arr));
    s.toCharArray(arr, sizeof(arr));
    return arr;
}

bool filterAnsiSequences = true; // Set to false to disable ANSI sequence filtering

String filterAnsi(const String &input, bool &inEscape) {
    if (!filterAnsiSequences) return input;
    String output = "";
    output.reserve(input.length());
    for (int i = 0; i < input.length(); i++) {
        char c = input[i];
        if (c == '\x1B') {
            inEscape = true;
            continue;
        }
        if (inEscape) {
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
                inEscape = false;
            }
            continue;
        }
        output += c;
    }
    return output;
}

void ssh_setup(String host) {
    if (!WifiState::wifiConnected) wifiConnectMenu();

    tft.fillScreen(bruceConfig.bgColor);
    tft.setCursor(0, 0);
    if (host != "") ssh_host = host;
    else {
        String my_net =
            WiFi.gatewayIP().toString().substring(0, WiFi.gatewayIP().toString().lastIndexOf(".") + 1);
        ssh_host = keyboard(my_net, 100, "HOST SSH (IP ou Hostname)");
        // ssh_host=keyboard("192.168.3.60",15,"SSH HOST (IP)");
    }
    ssh_port = num_keyboard("22", 5, "PORTA SSH");

    ssh_user = keyboard("", 76, "USUARIO SSH");
    // ssh_user=keyboard("ubuntu",76,"SSH USER");

    ssh_password = keyboard("", 76, "SENHA SSH", true);
    // ssh_password=keyboard("ubuntu",76,"SSH PASSWORD", true);

    IPAddress resolvedIp;
    if (WiFi.hostByName(ssh_host.c_str(), resolvedIp)) {
        ssh_host = resolvedIp.toString();
    } else {
        tft.setTextColor(TFT_RED, bruceConfig.bgColor);
        displayRedStripe("Falha resolver hostname.", true);
        log_e("Failed to resolve hostname: %s", ssh_host.c_str());
        returnToMenu = true;
        return;
    }

    // Connect to SSH server
    TaskHandle_t sshTaskHandle = NULL;

#if SOC_CPU_CORES_NUM > 1
    xTaskCreatePinnedToCore(ssh_loop, "SSH Task", SSH_TASK_STACK_SIZE, NULL, 1, &sshTaskHandle, 1);
#else
    xTaskCreate(ssh_loop, "SSH Task", SSH_TASK_STACK_SIZE, NULL, 1, &sshTaskHandle); // runs on core0
#endif
    if (sshTaskHandle == NULL) {
        displayError("Falha criar tarefa SSH.", true);
        Serial.println("Failed to create SSH Task");
        return;
    }

    while (!returnToMenu) { vTaskDelay(pdMS_TO_TICKS(200)); }
}

void ssh_loop(void *pvParameters) {
    String message = "";
    tft.setTextSize(FP);
    tft.fillScreen(bruceConfig.bgColor);
    tft.setCursor(0, 0);
    cursorY = tft.getCursorY();
    log_d("BEFORE SSH");
    my_ssh_session = ssh_new();
    log_d("AFTER SSH");
    // Disable watchdog
    disableCore0WDT();
#if SOC_CPU_CORES_NUM > 1
    disableCore1WDT();
#endif
    disableLoopWDT();

    if (my_ssh_session == NULL) {
        tft.setTextColor(TFT_RED, bruceConfig.bgColor);
        displayError("Falha criar sessao SSH.", true);
        log_d("SSH Session creation failed.");
        returnToMenu = true;
        vTaskDelete(NULL);
        return;
    }
    ssh_port_char = stringTochar(ssh_port);
    uint16_t ssh_port_int = atoi(ssh_port_char);

    ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, ssh_host.c_str());
    ssh_options_set(my_ssh_session, SSH_OPTIONS_PORT, &ssh_port_int);
    ssh_options_set(my_ssh_session, SSH_OPTIONS_USER, ssh_user.c_str());
    log_d("AFTER COMPARE AND OPTION SET");

    if (ssh_connect(my_ssh_session) != SSH_OK) {
        tft.setTextColor(TFT_RED, bruceConfig.bgColor);
        displayError("Erro conexao SSH.", true);
        log_d("SSH Connect error.");
        ssh_free(my_ssh_session);
        returnToMenu = true;
        vTaskDelete(NULL);
        return;
    }

    if (ssh_userauth_password(my_ssh_session, NULL, ssh_password.c_str()) != SSH_AUTH_SUCCESS) {
        tft.setTextColor(TFT_RED, bruceConfig.bgColor);
        displayError("Erro autenticacao SSH.", true);
        log_d("SSH Authentication error.");
        ssh_disconnect(my_ssh_session);
        ssh_free(my_ssh_session);
        returnToMenu = true;
        vTaskDelete(NULL);
        return;
    }

    channel_ssh = ssh_channel_new(my_ssh_session);
    if (channel_ssh == NULL || ssh_channel_open_session(channel_ssh) != SSH_OK) {
        tft.setTextColor(TFT_RED, bruceConfig.bgColor);
        displayError("Erro abrir canal SSH.", true);
        log_d("SSH Channel open error.");
        ssh_disconnect(my_ssh_session);
        ssh_free(my_ssh_session);
        returnToMenu = true;
        vTaskDelete(NULL);
        return;
    }

    if (ssh_channel_request_pty(channel_ssh) != SSH_OK) {
        tft.setTextColor(TFT_RED, bruceConfig.bgColor);
        displayError("Erro req PTY SSH.", true);
        log_d("SSH PTY request error.");
        ssh_channel_close(channel_ssh);
        ssh_channel_free(channel_ssh);
        ssh_disconnect(my_ssh_session);
        ssh_free(my_ssh_session);
        returnToMenu = true;
        vTaskDelete(NULL);
        return;
    }

    if (ssh_channel_request_shell(channel_ssh) != SSH_OK) {
        tft.setTextColor(TFT_RED, bruceConfig.bgColor);
        displayError("Erro req Shell SSH.", true);
        log_d("SSH Shell request error.");
        ssh_channel_close(channel_ssh);
        ssh_channel_free(channel_ssh);
        ssh_disconnect(my_ssh_session);
        ssh_free(my_ssh_session);
        returnToMenu = true;
        vTaskDelete(NULL);
        return;
    }

    log_d("SSH setup completed.");
    tft.fillScreen(bruceConfig.bgColor);
    tft.setTextColor(TFT_WHITE, bruceConfig.bgColor);
    tft.setTextSize(FP);

    // Setup scrollable area for SSH output
    ScrollableTextArea sshArea(FP, 0, 0, tftWidth, tftHeight - 20, false, true);
    String lineBuffer = "";
    bool inEscape = false;

    // Draw initial command prompt line at the bottom
    tft.fillRect(0, tftHeight - 20, tftWidth, 20, bruceConfig.bgColor);
    tft.setCursor(0, tftHeight - 16);
    tft.setTextColor(TFT_GREEN, bruceConfig.bgColor);
    tft.print(commandBuffer);

    char buffer[1024];
    int nbytes;
    keyStroke key;
    while (1) {
#ifdef HAS_KEYBOARD
        key = _getKeyPress();
        if (key.pressed) {
            unsigned long currentMillis = millis();
            if (currentMillis - lastKeyPressMillis >= debounceDelay) {
                lastKeyPressMillis = currentMillis;
                bool needsRedraw = false;
                for (auto i : key.word) {
                    commandBuffer += i;
                    needsRedraw = true;
                }
                if (key.del && commandBuffer.length() > 2) {
                    commandBuffer.remove(commandBuffer.length() - 1);
                    needsRedraw = true;
                } else if (key.enter) {
                    commandBuffer.trim();
                    if (commandBuffer.substring(2) == "cls") {
                        sshArea.clear();
                        sshArea.draw(true);
                    } else {
                        String message =
                            commandBuffer.substring(2) + "\r"; // Get the command part, exclude the "> "
                        ssh_channel_write(channel_ssh, message.c_str(), message.length()); // Send the command
                    }
                    commandBuffer = "> ";
                    needsRedraw = true;
                }

                if (needsRedraw) {
                    tft.fillRect(0, tftHeight - 20, tftWidth, 20, bruceConfig.bgColor);
                    tft.setCursor(0, tftHeight - 16);
                    tft.setTextColor(TFT_GREEN, bruceConfig.bgColor);
                    tft.print(commandBuffer);
                }
            }
        }

#else
        if (check(SelPress)) {

            while (check(SelPress)) { yield(); } // timerless debounce
            message = keyboard("cls", 76, "Comando SSH: ");
            while (check(SelPress)) { yield(); } // timerless debounce
            if (message == "cls") {
                sshArea.clear();
                sshArea.draw(true);
            } else {
                message += "\r";
                ssh_channel_write(channel_ssh, message.c_str(), message.length()); // Send the command
                log_d("%s", message);
            }

            commandBuffer = "> " + message;
            tft.fillRect(0, tftHeight - 20, tftWidth, 20, bruceConfig.bgColor);
            tft.setCursor(0, tftHeight - 16);
            tft.setTextColor(TFT_GREEN, bruceConfig.bgColor);
            tft.print(commandBuffer);
            tft.setTextSize(FP);
        }

#endif

        // Read data from SSH server and display it, handling ANSI sequences
        nbytes = ssh_channel_read_nonblocking(channel_ssh, buffer, sizeof(buffer), 0);

        if (nbytes > 0) {
            String msg = "";
            for (int i = 0; i < nbytes; ++i) {
                msg += char(buffer[i]);
            }
            log_d("%s", msg);

            String filteredMsg = filterAnsi(msg, inEscape);

            for (int i = 0; i < filteredMsg.length(); i++) {
                char c = filteredMsg[i];
                if (c == '\r') continue;
                if (c == '\n') {
                    sshArea.addLine(lineBuffer);
                    lineBuffer = "";
                } else {
                    lineBuffer += c;
                }
            }

            // Always add the current buffer to the screen so it's visible,
            // then we'll remove it from area before the next read unless a newline comes.
            // Wait, ScrollableTextArea.addLine adds it permanently.
            // A simple way is to just display it when there's a newline.
            // Let's draw what we have so far, lineBuffer handles incomplete lines.
            // But if the server prompt doesn't have a newline, it won't show.
            // So we add it, draw, and then remove the last line if it didn't have a newline?
            // Actually, for simplicity, we can just add any remaining characters as a line,
            // and if the next batch doesn't start with newline, we can append to the last line.
            // But `ScrollableTextArea` doesn't have `appendLastLine`.
            // Instead, we just keep `lineBuffer` and add it when `\n` is received.
            // If `lineBuffer` is not empty, we can force it to be drawn via `tft.drawString` ? No, just add it.
            if (lineBuffer.length() > 0 && nbytes < sizeof(buffer)) {
                sshArea.addLine(lineBuffer);
                lineBuffer = "";
            }

            sshArea.scrollToLine(sshArea.getMaxLines());
            sshArea.draw(true);

            // Redraw command prompt line
            tft.fillRect(0, tftHeight - 20, tftWidth, 20, bruceConfig.bgColor);
            tft.setCursor(0, tftHeight - 16);
            tft.setTextColor(TFT_GREEN, bruceConfig.bgColor);
            tft.print(commandBuffer);
        }

        // Handle channel closure and other conditions
        if (nbytes < 0 || ssh_channel_is_closed(channel_ssh)) {
            log_d("Encerrando");
            break;
        }
    }
    // Clean Up
    ssh_channel_close(channel_ssh);
    ssh_channel_free(channel_ssh);
    ssh_disconnect(my_ssh_session);
    ssh_free(my_ssh_session);
    check(SelPress); // Reset Button
    displayWarning("Sessao SSH fechada.", true);
    tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
    returnToMenu = true;
    enableCore0WDT();
#if SOC_CPU_CORES_NUM > 1
    enableCore1WDT();
#endif
    enableLoopWDT();
    feedLoopWDT();
    vTaskDelete(NULL);
}

String telnet_server_string = "";
String telnet_port_string = "";
char *telnet_server_ip;
char *telnet_server_port_char;

int telnet_server_port;

static int sock;

void telnet_loop() {
    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = inet_addr(telnet_server_ip);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(telnet_server_port);

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (sock < 0) {
        Serial.println("Unable to create socket");
        tft.setTextColor(TFT_RED, bruceConfig.bgColor);
        displayRedStripe("Nao criou socket", true);
        tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
        return;
    }

    if (connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) != 0) {
        Serial.println("Socket connection failed");
        tft.setTextColor(TFT_RED, bruceConfig.bgColor);
        displayRedStripe("Conexao socket falhou", true);
        tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
        close(sock);
        return;
    }

    Serial.println("Connected to TELNET server");
    tft.setTextColor(TFT_GREEN, bruceConfig.bgColor);
    displayTextLine("Conectado servidor TELNET");
    tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
    tft.fillScreen(bruceConfig.bgColor);

    ScrollableTextArea telnetArea(FP, 0, 0, tftWidth, tftHeight - 20, false, true);
    String lineBuffer = "";
    bool inEscape = false;

    String commandInput;

    while (1) {
        // waitForInput(commandInput);
        commandInput = keyboard("", 76, "COMANDO");
        if (commandInput.length() > 0) {
            String toSend = commandInput + "\r\n";
            send(sock, toSend.c_str(), toSend.length(), 0);
        }

        // You can also receive data from the server
        char buffer[256];
        int len = recv(sock, buffer, sizeof(buffer) - 1, MSG_DONTWAIT);
        if (len > 0) {
            buffer[len] = '\0';
            // Check for Telnet negotiation commands (IAC)
            /*
if (buffer[0] == 0xFF) {
    // Skip Telnet negotiation command
    continue;
}
*/
            Serial.printf("Received from server %s\n", buffer);
            // tft.printf("Received from server %s\n", buffer);
            for (int i = 0; i < len; i++) { Serial.printf("%02X ", buffer[i]); }

            String msg = filterAnsi(String(buffer), inEscape);

            for (int i = 0; i < msg.length(); i++) {
                char c = msg[i];
                if (c == '\r') continue;
                if (c == '\n') {
                    telnetArea.addLine(lineBuffer);
                    lineBuffer = "";
                } else {
                    lineBuffer += c;
                }
            }
            if (lineBuffer.length() > 0) {
                telnetArea.addLine(lineBuffer);
            }
            telnetArea.scrollToLine(telnetArea.getMaxLines());
            telnetArea.draw(true);
        }

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void telnet_setup() {
    if (!WifiState::wifiConnected) wifiConnectMenu();

    tft.fillScreen(bruceConfig.bgColor);
    tft.setCursor(0, 0);
    Serial.begin(115200); // Initialize serial communication for debugging
    Serial.println("Starting Setup");

    // auto cfg = M5.config();
    // M5Cardputer.begin(cfg, true);
    tft.setRotation(bruceConfigPins.rotation);
    tft.setTextSize(1); // Set text size

    cursorY = tft.getCursorY();

    tft.setCursor(0, 0);
    // tft.print("TELNET Host: \n");

    // Here the telnet_server_ip needs to be a char*, thats why the stringTochar()

    // waitForInput(telnet_server_string);
    telnet_server_string = keyboard("", 76, "SERVIDOR_TELNET");
    telnet_server_ip = stringTochar(telnet_server_string);
    delay(300);
    // Serial.println(telnet_server_ip);

    // tft.print("TELNET Port: \n");
    // waitForInput(telnet_port_string);
    telnet_port_string = num_keyboard("", 76, "PORTA TELNET");
    delay(300);
    char arr2[5];
    // telnet_server_port_char =
    telnet_port_string.toCharArray(arr2, sizeof(arr2));
    // telnet_server_port_char = stringTochar(telnet_port_string);
    telnet_server_port = atoi(arr2);
    Serial.println(telnet_server_ip);
    Serial.println(telnet_server_port);

    telnet_loop();
}
#endif
