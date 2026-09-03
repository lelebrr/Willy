# Barramentos de hardware (`bus_HAL`, SPI, SD)

## I2C (`src/core/bus_HAL.{cpp,h}`)

Dois papéis: `sys_i2c` (periféricos do sistema: touch, PMIC, RTC — configurado uma vez em `boards/*/interface.cpp`, nunca desligado) e `i2c_bus` (porta do usuário, p.ex. Grove, configurável em runtime).

- `acquireI2CBus([sda, scl])`: se os pinos coincidem com `sys_i2c`, devolve o barramento do sistema (sem reiniciar); senão inicia/reconfigura o controlador livre e aplica **fast-mode 400 kHz** (PN532, MFRC522, ST25R, SRIX, gesto e RTC suportam). Placas M5 usam o adaptador `M5SysWireAdapter`.
- `releaseI2CBus()`: encerra o bus de usuário (nunca o do sistema).
- `checkAndRecoverSysI2CBus()`: watchdog chamado a cada ~1 s pela task de input — detecta SDA/SCL preso em LOW só com `digitalRead()` e recupera com 9 clocks + re-init. Em placas sem `sys_i2c` (`SYS_I2C_SDA=-1`) retorna `false` imediatamente.

## SPI

Instâncias: `SPI` (padrão), `sdcardSPI` (SD em barramento próprio), `CC_NRF_SPI` (rádios, VSPI/HSPI conforme `USE_HSPI_PORT`), `AUX_SPI` interno. `acquireSPIBus(sck, miso, mosi)` escolhe a instância pelos pinos. Módulos chamam `sdcardSPI.end()` / `CC_NRF_SPI.end()` ao detectar conflito de pinos (ex.: RF TX no MOSI do SD).

## SD (`setupSdCard`)

Cascata por barramento: padrão → 4 MHz → 1 MHz; barramento compartilhado com o TFT usa a instância do display. `SD.end()` + `setupSdCard()` no menu **Arquivos → Recarregar SD** permite trocar o cartão sem reboot.

## Pinos persistidos (`brucePins.conf`)

`CC1101_bus`, `NRF24_bus`, `PN532_bus`, `SDCARD_bus`, `ST25R_bus` (+ `W5500_bus`, `LoRa_bus` fora do LITE), `sys_i2c`, `i2c_bus`, `uart_bus`, `gps_bus`, rotação, BLE name, IR/RF/RFID/GPS. Editáveis em **Config → Modo Dev** (pinos) e via serial `settings`. Fallbacks de compilação em `include/precompiler_flags.h` (`GROVE_*`, `SYS_I2C_*` = -1 quando a placa não define).
