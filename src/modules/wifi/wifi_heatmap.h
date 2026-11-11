#ifndef __WIFI_HEATMAP_H__
#define __WIFI_HEATMAP_H__

#include <WiFi.h>
#include "globals.h"

/**
 * @brief Visual WiFi Heatmap - Mapa de calor em tempo real de APs por RSSI
 *
 * Funcionalidade de reconhecimento físico para pentest defensivo.
 * Permite visualizar a intensidade do sinal de APs em diferentes posições.
 */
void wifiHeatmap();

/**
 * @brief Encrypted Traffic Fingerprint - Identificação passiva de dispositivos
 *
 * Analisa padrões de pacotes criptografados para identificar SO/dispositivo.
 * Baseado em tamanho de pacotes e intervalos de transmissão.
 */
void encryptedTrafficFingerprint();

#endif
