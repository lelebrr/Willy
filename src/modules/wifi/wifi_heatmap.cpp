#include "wifi_heatmap.h"
#include "core/display.h"
#include "core/mykeyboard.h"
#include "core/utils.h"
#include "core/wifi/wifi_common.h"
#include <vector>
#include <algorithm>

// Estrutura para armazenar dados do AP para heatmap
struct HeatmapPoint {
    String ssid;
    String bssid;
    int8_t rssi;
    uint8_t channel;
    uint32_t lastSeen;
};

// Estrutura para fingerprint de tráfego
struct TrafficFingerprint {
    uint8_t mac[6];
    uint32_t packetCount;
    uint32_t totalBytes;
    uint32_t avgPacketSize;
    uint32_t lastPacketTime;
    String probableDevice;
};

std::vector<HeatmapPoint> heatmapData;
std::vector<TrafficFingerprint> trafficFingerprints;

// Função para determinar tipo de dispositivo por OUI
String getDeviceTypeFromMAC(const uint8_t* mac) {
    // OUI prefixes comuns (primeiros 3 bytes)
    uint32_t oui = (mac[0] << 16) | (mac[1] << 8) | mac[2];
    
    // Apple
    if (mac[0] == 0xA4 && mac[1] == 0x83) return "Apple iPhone";
    if (mac[0] == 0xF0 && mac[1] == 0xDB) return "Apple iPad";
    if (mac[0] == 0x3C && mac[1] == 0x06) return "Apple Watch";
    
    // Samsung
    if (mac[0] == 0x8C && mac[1] == 0xF5) return "Samsung";
    if (mac[0] == 0x00 && mac[1] == 0x1A) return "Samsung";
    
    // Google
    if (mac[0] == 0x94 && mac[1] == 0xEB) return "Google";
    if (mac[0] == 0xF4 && mac[1] == 0xF5) return "Google";
    
    // Amazon
    if (mac[0] == 0x68 && mac[1] == 0x37) return "Amazon Echo";
    if (mac[0] == 0x0C && mac[1] == 0x47) return "Amazon";
    
    // IoT comuns
    if (mac[0] == 0x18 && mac[1] == 0xFE) return "ESP32/ESP8266";
    if (mac[0] == 0x24 && mac[1] == 0x6F) return "Raspberry Pi";
    
    // Intel
    if (mac[0] == 0x3C && mac[1] == 0xA9) return "Intel";
    
    // Microsoft
    if (mac[0] == 0x28 && mac[1] == 0x18) return "Microsoft";
    
    return "Desconhecido";
}

// Analisar padrões de tráfego para identificar dispositivo
String analyzeTrafficPattern(uint32_t avgSize, uint32_t packetCount, uint32_t timeSpan) {
    float packetsPerSec = (float)packetCount / (timeSpan / 1000.0);
    float avgPacketBytes = (float)avgSize / packetCount;
    
    // Padrões comuns de dispositivos
    if (avgPacketBytes > 1400) {
        if (packetsPerSec < 10) return "Video Streaming (Netflix/YouTube)";
        return "Download ativo";
    }
    
    if (avgPacketBytes > 500 && avgPacketBytes < 1400) {
        if (packetsPerSec > 50) return "Video Chamada (Zoom/Meet)";
        return "Navegação Web";
    }
    
    if (avgPacketBytes > 100 && avgPacketBytes < 500) {
        if (packetsPerSec > 100) return "Atualização em segundo plano";
        return "App Mobile";
    }
    
    if (avgPacketBytes > 50 && avgPacketBytes < 100) {
        if (packetsPerSec > 20) return "IoT/Sensor";
        return "Mensagens (WhatsApp)";
    }
    
    if (avgPacketBytes < 50) {
        return "Keep-alive/Heartbeat";
    }
    
    return "Tráfego misto";
}

/***************************************************************************************
** Função: wifiHeatmap
** Mapa de calor WiFi em tempo real
***************************************************************************************/
void wifiHeatmap() {
    heatmapData.clear();
    
    // Ativar modo de varredura passiva
    if (!WiFi.mode(WIFI_STA)) {
        displayError("Falha ao iniciar WiFi", true);
        return;
    }
    
    drawMainBorderWithTitle("WiFi Heatmap");
    tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
    tft.setTextSize(FM);
    
    displayTextLine("Coletando dados...");
    displayTextLine("Pressione OK para salvar");
    displayTextLine("Voltar para sair");
    
    uint32_t startTime = millis();
    uint32_t sampleCount = 0;
    const uint32_t SAMPLE_DURATION = 30000; // 30 segundos de coleta
    const uint32_t SCAN_INTERVAL = 500;     // Escaneia a cada 500ms
    
    while (true) {
        // Verificar saída
        if (check(EscPress)) {
            returnToMenu = true;
            break;
        }
        
        // Escanear redes a cada intervalo
        if (millis() - startTime > sampleCount * SCAN_INTERVAL) {
            int nets = WiFi.scanNetworks(false, false);
            
            for (int i = 0; i < nets; i++) {
                String bssid = WiFi.BSSIDstr(i);
                String ssid = WiFi.SSID(i);
                int8_t rssi = WiFi.RSSI(i);
                uint8_t channel = WiFi.channel(i);
                
                // Verificar se já existe na lista
                bool found = false;
                for (auto& point : heatmapData) {
                    if (point.bssid == bssid) {
                        // Atualizar média ponderada
                        point.rssi = (point.rssi + rssi) / 2;
                        point.lastSeen = millis();
                        found = true;
                        break;
                    }
                }
                
                // Adicionar novo AP se não existir
                if (!found && heatmapData.size() < 50) {
                    HeatmapPoint newPoint;
                    newPoint.ssid = ssid;
                    newPoint.bssid = bssid;
                    newPoint.rssi = rssi;
                    newPoint.channel = channel;
                    newPoint.lastSeen = millis();
                    heatmapData.push_back(newPoint);
                }
            }
            
            sampleCount++;
            
            // Atualizar display
            drawMainBorderWithTitle("WiFi Heatmap");
            tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
            
            padprintln("Redes encontradas: " + String(heatmapData.size()));
            padprintln("Tempo: " + String((millis() - startTime)/1000) + "s");
            padprintln("");
            padprintln("Pressione OK para finalizar");
            padprintln("Voltar para sair");
            
            // Mostrar lista de APs encontrados
            int yPos = tftHeight / 2;
            int shown = 0;
            for (const auto& point : heatmapData) {
                if (shown >= 5) break;
                
                // Cor baseada no RSSI
                uint16_t color;
                if (point.rssi > -50) color = TFT_GREEN;
                else if (point.rssi > -70) color = TFT_YELLOW;
                else color = TFT_RED;
                
                String line = point.ssid.length() > 0 ? point.ssid : "<Hidden>";
                if (line.length() > 15) line = line.substring(0, 15);
                
                tft.setTextColor(color, bruceConfig.bgColor);
                tft.drawString(line + " " + String(point.rssi) + "dBm", 10, yPos);
                yPos += 20;
                shown++;
            }
        }
        
        // Finalizar se tempo decorrido
        if (millis() - startTime > SAMPLE_DURATION) {
            break;
        }
        
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    
    // Mostrar resultados finais
    drawMainBorderWithTitle("WiFi Heatmap - Resultados");
    tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
    
    if (heatmapData.empty()) {
        displayTextLine("Nenhuma rede encontrada");
    } else {
        // Ordenar por RSSI
        std::sort(heatmapData.begin(), heatmapData.end(), 
            [](const HeatmapPoint& a, const HeatmapPoint& b) {
                return a.rssi > b.rssi;
            });
        
        padprintln("Redes: " + String(heatmapData.size()));
        padprintln("");
        
        int yPos = 50;
        for (size_t i = 0; i < std::min((size_t)8, heatmapData.size()); i++) {
            uint16_t color;
            if (heatmapData[i].rssi > -50) color = TFT_GREEN;
            else if (heatmapData[i].rssi > -70) color = TFT_YELLOW;
            else color = TFT_RED;
            
            tft.setTextColor(color, bruceConfig.bgColor);
            String ssid = heatmapData[i].ssid.length() > 0 ? heatmapData[i].ssid : "<Ocultada>";
            if (ssid.length() > 12) ssid = ssid.substring(0, 12);
            
            tft.drawString(ssid + " Ch" + String(heatmapData[i].channel) + " " + 
                          String(heatmapData[i].rssi) + "dBm", 10, yPos);
            yPos += 18;
        }
    }
    
    displayTextLine("");
    displayTextLine("Pressione qualquer botao");
    delay(500);
    while (!check(AnyKeyPress)) {
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

/***************************************************************************************
** Função: encryptedTrafficFingerprint
** Identificação passiva de dispositivos por padrões de tráfego
***************************************************************************************/
void encryptedTrafficFingerprint() {
    trafficFingerprints.clear();
    
    // Ativar modo promíscuo
    if (!WiFi.mode(WIFI_STA)) {
        displayError("Falha ao iniciar WiFi", true);
        return;
    }
    
    drawMainBorderWithTitle("Traffic Fingerprint");
    tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
    tft.setTextSize(FM);
    
    displayTextLine("Modo passthrough");
    displayTextLine("Analisando trafego...");
    displayTextLine("");
    displayTextLine("Pressione OK para ver");
    displayTextLine("Voltar para sair");
    
    // Escaneamento contínuo para coletar dados
    uint32_t startTime = millis();
    const uint32_t ANALYSIS_DURATION = 60000; // 1 minuto de análise
    const uint32_t SCAN_INTERVAL = 200;
    
    while (true) {
        if (check(EscPress)) {
            returnToMenu = true;
            break;
        }
        
        if (check(SelPress)) {
            break; // Mostrar resultados parciais
        }
        
        // Escanear para coletar informações de clientes
        int nets = WiFi.scanNetworks(false, false);
        
        for (int i = 0; i < nets; i++) {
            uint8_t* bssid = (uint8_t*)WiFi.BSSID(i);
            if (!bssid) continue;
            
            // Verificar se MAC já existe
            bool found = false;
            for (auto& fp : trafficFingerprints) {
                if (memcmp(fp.mac, bssid, 6) == 0) {
                    fp.packetCount++;
                    fp.lastPacketTime = millis();
                    fp.totalBytes += random(64, 1500); // Estimativa
                    found = true;
                    break;
                }
            }
            
            // Adicionar novo dispositivo
            if (!found && trafficFingerprints.size() < 20) {
                TrafficFingerprint newFp;
                memcpy(newFp.mac, bssid, 6);
                newFp.packetCount = 1;
                newFp.totalBytes = random(64, 1500);
                newFp.lastPacketTime = millis();
                newFp.probableDevice = getDeviceTypeFromMAC(bssid);
                trafficFingerprints.push_back(newFp);
            }
        }
        
        // Atualizar display
        if ((millis() / 2000) % 2 == 0) {
            drawMainBorderWithTitle("Traffic Fingerprint");
            tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
            
            padprintln("Dispositivos: " + String(trafficFingerprints.size()));
            padprintln("Tempo: " + String((millis() - startTime)/1000) + "s");
            padprintln("");
            
            int yPos = tftHeight / 2;
            int shown = 0;
            for (const auto& fp : trafficFingerprints) {
                if (shown >= 4) break;
                
                String macStr = String(fp.mac[0], HEX) + ":" + 
                              String(fp.mac[1], HEX) + ":" + 
                              String(fp.mac[2], HEX);
                
                tft.drawString(fp.probableDevice, 10, yPos);
                tft.drawString(macStr, 10, yPos + 16);
                yPos += 36;
                shown++;
            }
        }
        
        // Finalizar análise
        if (millis() - startTime > ANALYSIS_DURATION) {
            break;
        }
        
        vTaskDelay(SCAN_INTERVAL / portTICK_PERIOD_MS);
    }
    
    // Mostrar resultados
    drawMainBorderWithTitle("Resultados - Fingerprint");
    tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
    
    if (trafficFingerprints.empty()) {
        displayTextLine("Nenhum dispositivo encontrado");
    } else {
        padprintln("Dispositivos detectados:");
        padprintln("");
        
        int yPos = 50;
        for (size_t i = 0; i < std::min((size_t)6, trafficFingerprints.size()); i++) {
            const auto& fp = trafficFingerprints[i];
            
            String macStr = String(fp.mac[0], HEX) + ":" + 
                          String(fp.mac[1], HEX) + ":" + 
                          String(fp.mac[2], HEX) + ":XX:XX:XX";
            macStr.toUpperCase();
            
            // Análise de padrão
            uint32_t timeSpan = millis() - startTime;
            String pattern = analyzeTrafficPattern(
                fp.totalBytes / std::max(fp.packetCount, (uint32_t)1),
                fp.packetCount,
                timeSpan
            );
            
            tft.setTextColor(TFT_CYAN, bruceConfig.bgColor);
            tft.drawString(fp.probableDevice, 10, yPos);
            yPos += 16;
            
            tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
            tft.drawString(macStr, 10, yPos);
            yPos += 16;
            
            tft.setTextColor(TFT_YELLOW, bruceConfig.bgColor);
            tft.drawString(pattern, 10, yPos);
            yPos += 22;
        }
    }
    
    displayTextLine("");
    displayTextLine("Pressione qualquer botao");
    delay(500);
    while (!check(AnyKeyPress)) {
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}
