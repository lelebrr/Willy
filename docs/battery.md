# Bateria (monitoramento real)

## Arquitetura

`int getBattery()` (1–100, `core/utils.h`, `__attribute__((weak)`) é sobrescrito por cada placa (`boards/*/interface.cpp`); `bool isCharging()` (`display.h`, weak) idem. **Convenção: `0` = sem monitor → UI oculta.** Semáforo: leituras PMIC via I2C, com cache de 10 s na status bar.

| Placa | Fonte | `isCharging()` |
|---|---|---|
| T-Watch S3, M5Stack (Core/Core2/CoreS3/CPlus/nega/StickS3/DinMeter/Nesso) | fuel-gauge (`axp192.getBatteryPercent()`, `M5.Power`, `bq` com suavização) | PMIC real |
| Smoochiee, xk404, Reaper, T-Embed, Lora-Pager | `PPM.getBattVoltage()` → % linear 3300–4150 mV, `isChargeDone()` → 100 | `PPM.isCharging()` |
| CYD-2432S028, N8R2, N16R8, C5, ESP-General, Marauder, nm-cyd-c5, elecrow | sem circuito de bateria → `0` (oculto) | `false` |

> `elecrow_advance_s3` retornava `100` fixo (falso cheio) — corrigido para `0`.

## Onde aparece

- **Status bar**: ícone 22×12 com nível; verde >50, amarelo >15, **vermelho ≤15**; cor do tema carregando. Só com monitor real.
- **Cyber Menu**: `%` ao vivo (`--%` sem monitor).
- **Pwnagotchi**: `UPS %` real na tela de relógio.
- **BLE GATT** (`BatteryService` 0x180F/0x2A19): valor inicial + notify a cada 60 s.
- **Web** `/api/status`: campo `battery`. **JS**: `device.battery()`.
