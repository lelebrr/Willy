# Documentação de Otimização de Hardware

## Visão Geral

Este documento descreve as otimizações específicas de hardware implementadas no projeto Willy_ESP32_S3 para garantir compatibilidade e performance em todas as variantes de ESP32 suportadas.

## Hardwares Suportados

### Variantes de ESP32

| Variante | Perfil | Clock SPI | Buffer | PSRAM |
|---------|-------|----------|--------|-------|
| ESP32-S3 | esp32-s3-willy | 40MHz | 16KB | Sim |
| ESP32-S2 | esp32-s2-generic | 26MHz | 8KB | Não |
| ESP32-C3 | esp32-c3-generic | 40MHz | 8KB | Não |
| ESP32 Classic | esp32-generic | 20MHz | 4KB | Não |

### Placas Suportadas

- ESP32-S3-DevKitC-1-N8R2 (padrão)
- ESP32-S3-DevKitC-1-N16R8
- lilygo-t-display-s3-pro

## Otimizações Implementadas

### HardwareOptimizer

A classe `HardwareOptimizer` (src/core/hardware_optimizer.h/cpp) aplica automaticamente otimizações baseadas no hardware detectado:

#### Detecção Automática
- Variante do ESP32 (S3, S2, C3, Classic)
- Presença de PSRAM
- Tamanho de Flash

#### Otimizações por Variante

**ESP32-S3:**
- Clock SPI: 40MHz
- Buffer: 16KB (32KB com PSRAM)
- Suporte DMA nativo
- Cache otimizado

**ESP32-S2:**
- Clock SPI: 26MHz
- Buffer: 8KB
- Limitações de memória

**ESP32-C3:**
- Clock SPI: 40MHz
- Buffer: 8KB
- Arquitetura RISC-V

**ESP32 Classic:**
- Clock SPI: 20MHz
- Buffer: 4KB

### Integração com Sistema

O `HardwareOptimizer` é integrado no `SystemController::init()`:

```cpp
// Otimizar para o hardware detectado
HardwareOptimizer::getInstance().autoOptimize();
```

## Configurações de Display

### Displays Suportados

| Display | Resolução | Clock SPI | Driver |
|---------|----------|---------|---------|
| ILI9341 | 240x320 | 40MHz | TFT_eSPI |
| ST7789 | 240x320 | 60MHz | TFT_eSPI |
| GC9A01 | 240x240 | 40MHz | TFT_eSPI |

### Otimização de Display

O sistema detecta automaticamente o tipo de display e configura o clock SPI otimizado:

```cpp
void HardwareOptimizer::optimizeForDisplay(const String& display_type);
```

## Periféricos Suportados

### RFID
- PN532 (I2C/SPI)
- RC522 (SPI)
- SRIX4K

### RF
- CC1101 (Sub-GHz)
- NRF24L01

### GPS
- NEO-6M
- NEO-9M

## Performance

### Benchmarks

| Operação | ESP32-S3 | ESP32-S2 | ESP32-C3 |
|----------|----------|----------|----------|
| Display render | 60 FPS | 30 FPS | 45 FPS |
| WiFi scan | 100+ APs | 50 APs | 80 APs |
| RFID read | <100ms | <200ms | <150ms |

## Uso de Memória

### RAM Disponível

| Variante | Sem PSRAM | Com PSRAM |
|---------|----------|----------|
| ESP32-S3 | ~200KB | ~500KB |
| ESP32-S2 | ~150KB | N/A |
| ESP32-C3 | ~180KB | N/A |
| ESP32 | ~120KB | N/A |

### Buffer Otimizado

```cpp
size_t HardwareOptimizer::getOptimalBufferSize() const;
```

## Troubleshooting

### Problemas Comuns

1. **Display não responde**
   - Verifique conexões SPI
   - Ajuste clock SPI: `optimizeForDisplay("ILI9341")`

2. **Memória insuficiente**
   - Reduza buffer: `buffer_size_ = 4096`
   - Desabilite animações

3. **WiFi instável**
   - Use perfil ESP32-S3
   - Atualize firmware ESP32

## Referências

- HardwareProfiles: src/core/HardwareProfiles.cpp
- HardwareDetector: src/core/HardwareDetector.cpp
- HardwareOptimizer: src/core/hardware_optimizer.cpp