# ST25R3916 (RFID)

Suporte ao leitor NFC ST25R3916 (SPI ou I2C) em `src/modules/rfid/ST25R3916.{cpp,h}` + `crypto1.{cpp,h}`.

## Seleção do módulo

**RFID → Config → Modulo RFID**: `ST25R3916 em SPI` / `ST25R3916 em I2C` (builds completos). Valores do enum `RFIDModules`: `ST25R3916_SPI_MODULE = 4`, `ST25R3916_I2C_MODULE = 6`. O `validateRfidModuleValue()` os preserva; placas `reaper` e `lilygo-t-lora-pager` já definem SPI como padrão.

## Pinos

`bruceConfigPins.ST25R_bus` (`SPIPins`: sck/miso/mosi/cs + `io0`=IRQ). Padrões via defines da placa (`ST25R_SCLK`, `ST25R_MISO`, `ST25R_MOSI`, `ST25R_CS`, `ST25R_IRQ`); sem defines, barramento vazio. Ajuste em **Config → Modo Dev → Pinos ST25R3916**, persistido em `ST25R_Pins` no `brucePins.conf`.

## Integração

- CLI serial: `rfid ...` (`createRfidCommands`) instancia via `_createRfidModule()` conforme `rfidModule`, incluindo ST25R.
- `bruceConfig.ensureMifareKeysLoaded()` carrega as chaves MIFARE antes das operações.
- I2C usa `acquireI2CBus()` (fast-mode 400 kHz, ver `hardware_buses.md`).

## Troubleshooting

| Sintoma | Causa provável |
|---|---|
| `ERROR: RFID module not found` no `rfid` | `begin()` falhou: confira CS/IRQ e alimentação |
| Módulo volta para RFID2 sozinho | build `LITE_VERSION` não inclui ST25R (esperado) |
| Pins zerados após update | `validateSpiPins()` corrigiu valores inválidos; reconfigure |
