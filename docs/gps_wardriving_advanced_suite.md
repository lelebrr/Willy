# GPS & Wardriving Advanced Suite - Willy Firmware

## Visão Geral
Suite completa de 55+ funções GPS e Wardriving para o Willy Firmware. Implementações REAIS usando TinyGPS++, WiFi, NimBLE e IO Expander. Acesse via menu **GPS → GPS Suite Avancada**.

## Arquitetura
- **Header**: `src/modules/gps/gps_wardriving_advanced.h` (218 linhas)
- **Implementação**: `src/modules/gps/gps_wardriving_advanced.cpp` (~2000 linhas)
- **Integração**: `src/core/menu_items/GpsMenu.cpp`
- **Depende de**: TinyGPS++, WiFi, NimBLE, IO Expander, SD/LittleFS

## Categorias (7 menus)

### 1. Navigation & Tracking (10 funções)
| Função | Descrição |
|--------|-----------|
| `gpsSpeedTracker` | Rastreador de velocidade com display em tempo real |
| `gpsCompass` | Bússola digital com heading e rosa dos ventos |
| `gpsWaypointManager` | Gerenciador de waypoints com salvar/carregar |
| `gpsTripComputer` | Computador de viagem com distance, tempo, velocidade |
| `gpsGeofence` | Alerta de geofence com raio configurável |
| `gpsAltitudeProfile` | Perfil de altitude com gráfico |
| `gpsRouteRecorder` | Gravador de rota com track points |
| `gpsRouteReplay` | Reprodução de rota gravada |
| `gpsProximityAlert` | Alerta de proximidade para waypoints |
| `gpsMultiMarker` | Marcação de múltiplos pontos |

### 2. GPS Data & Analysis (10 funções)
| Função | Descrição |
|--------|-----------|
| `gpsSignalMonitor` | Monitor de sinal GPS com C/N0 |
| `gpsNMEARawViewer` | Visualizador NMEA bruto |
| `gpsDataLogger` | Logger de dados GPS |
| `gpsAccuracyTest` | Teste de precisão com HDOP |
| `gpsSatelliteView` | Visualização de satélites visíveis |
| `gpsTimeServer` | Servidor de tempo GPS |
| `gpsCoordinateConvert` | Conversão de coordenadas (DMS/UTM) |
| `gpsDistanceCalculator` | Calculadora de distância |
| `gpsBearingCalculator` | Calculadora de bearing |
| `gpsAreaCalculator` | Calculadora de área |

### 3. Export & Visualization (8 funções)
| Função | Descrição |
|--------|-----------|
| `gpsExportKML` | Exporta dados em formato KML |
| `gpsExportGeoJSON` | Exporta dados em formato GeoJSON |
| `gpsExportCSV` | Exporta dados em formato CSV |
| `gpsExportGPXEnhanced` | Exporta dados em formato GPX |
| `gpsImportWaypoints` | Importa waypoints de arquivo |
| `gpsMapDisplay` | Display de mapa ASCII |
| `gpsTrackOverlay` | Overlay de track no mapa |
| `gpsDataSummary` | Resumo dos dados coletados |

### 4. WiFi Wardriving Enhanced (12 funções)
| Função | Descrição |
|--------|-----------|
| `wifiWardriveEnhanced` | Wardriving WiFi com GPS |
| `wifiChannelHopScan` | Scan com channel hopping |
| `wifiSignalMonitor` | Monitor de sinal WiFi |
| `wifiNetworkFilter` | Filtro de redes por critérios |
| `wifiDuplicateTracker` | Rastreador de redes duplicadas |
| `wifiHiddenNetworkDetect` | Detector de redes ocultas |
| `wifiBandwidthEstimate` | Estimativa de largura de banda |
| `wifiEncryptionAnalyzer` | Analisador de criptografia |
| `wifiCoverageMap` | Mapa de cobertura WiFi |
| `wifiSessionStats` | Estatísticas da sessão |
| `wifiDataMerge` | Merge de dados de múltiplas sessões |
| `wifiNetworkAlert` | Alerta de rede específica |

### 5. BLE Wardriving (8 funções)
| Função | Descrição |
|--------|-----------|
| `bleWardriveScan` | Scan BLE com GPS |
| `bleDeviceClassifier` | Classificador de dispositivos BLE |
| `bleSignalTracker` | Rastreador de sinal BLE |
| `bleProximityDetector` | Detector de proximidade BLE |
| `bleBeaconScanner` | Scanner de beacons |
| `bleMACManufacturer` | Identificação de fabricante por MAC |
| `bleDeviceHistory` | Histórico de dispositivos |
| `bleExportCSV` | Exporta dados BLE em CSV |

### 6. Combined GPS+WiFi+BLE (7 funções)
| Função | Descrição |
|--------|-----------|
| `gpsWifiHeatmap` | Heatmap WiFi+GPS |
| `gpsWifiAutoConnect` | Auto-connect baseado em localização |
| `gpsTriggerCapture` | Captura triggered por geofence |
| `gpsLocationFingerprint` | Fingerprint de localização |
| `gpsDeadDrop` | Dead drop digital baseado em GPS |
| `gpsNetworkMapper` | Mapeamento de redes por local |
| `gpsSessionReplay` | Replay de sessão completa |

### 7. Upload & Cloud (4 funções)
| Função | Descrição |
|--------|-----------|
| `wigleQuery` | Consulta WiGLE API |
| `wigleStats` | Estatísticas WiGLE |
| `wdgwarsStats` | Estatísticas WDGWars |
| `cloudSync` | Sincronização na nuvem |

## Configurações
| Parâmetro | Default | Descrição |
|-----------|---------|-----------|
| `alertRadius` | 100m | Raio para alertas de proximidade |
| `geofenceRadius` | 500m | Raio da geofence |
| `scanIntervalMs` | 2000ms | Intervalo de scan |
| `autoExport` | false | Exportação automática ao final |
| `showSignalBars` | true | Mostrar barras de sinal |
| `maxWaypoints` | 50 | Máximo de waypoints em memória |
| `enableBLE` | true | Habilitar scan BLE |
| `enableWiFi` | true | Habilitar scan WiFi |
| `channelHopDelayMs` | 300ms | Delay por channel hop |
| `logNMEA` | false | Logar frases NMEA brutas |
| `logSpeed` | true | Logar dados de velocidade |
| `exportFormat` | "KML" | Formato de exportação |

## Uso
1. Acesse **GPS → GPS Suite Avancada**
2. Bem-vindo com contagem de funções
3. Navegue pelas 7 categorias usando UP/DOWN
4. Selecione com SEL para entrar na subcategoria
5. ESC para voltar

## Dependências
- `TinyGPSPlus` — parse NMEA
- `WiFi` — scan e wardriving
- `NimBLE-Arduino` — scan BLE
- `IO Expander` — controle de energia GPS
- `SD/LittleFS` — persistência de dados
