# Willy Firmware — Relatório de Atualização de Dependências

> Data: 11/09/2026 | Branch: `main`
> Escopo: auditoria completa do `platformio.ini` + patches, validação real via `pio run -e CYD-2USB`, e aplicação incremental (1 item = 1 commit = 1 build).

---

## 1. Resumo dos commits aplicados

| # | Commit | Tipo | O que mudou | Risco |
|---|--------|------|-------------|-------|
| 1 | `ad5a548` | fix(build) | `patch_library_conflicts.py`: adiciona `#include <algorithm>` aos 3 patches NimBLE (Amiibolink, Chameleon Ultra, PN532BLE) | ZERO |
| 2 | `bc35658` | chore(deps) | `[env]`: pin `ArduinoJson@^7.4.3` (antes flutuante) | ZERO |
| 3 | `242bd88` | chore(deps) | `[env_light]`: pin `ArduinoJson@^7.4.3` + `ESPAsyncWebServer@^3.11.2` + `RF24@^1.6.2` | BAIXO |
| 4 | `8d8a927` | chore(deps) | `[env]`: `RF24 1.4.11 -> ^1.6.2` (unifica) + `RadioLib 7.4.0 -> ^7.7.1` + `ESPAsyncWebServer@^3.11.2` + dedup `AnimatedGIF` | BAIXO |
| 5 | `1d8fb92` | chore(deps) | `[env:willy_release]`, `[env:willy_debug]`, `[env:test]`: mesmo lote do `[env]` | BAIXO |
| 6 | `04d98d1` + `3d0159f` | chore(deps) | NimBLE `2.3.7 -> ^2.5.1` (todos os 5 envs) | BAIXO |
| 7 | `c940781` | chore(deps) | PNGdec `1.0.3 -> ^1.1.2` + LVGL `8.3.11 -> ^8.4.0` | BAIXO |
| 8 | `5d9ada9` | fix | `display.h`: corrige typo `willyConfig -> wilyConfig` (7 ocorrencias) | ZERO |
| 9 | `91d62b8` | chore(ci) | `docker/Dockerfile.ci`: pin `platformio>=6.2,<7` | ZERO |
| 10 | `b146e37` | revert | Desfaz tentativa de `CONFIG_ETH_USE_ESP32_EMAC=1` (nao resolveu) | ZERO |

---

## 2. Validacao real (build no CYD-2USB)

### 2.1 `<algorithm>` nos patches NimBLE -- CORRIGE UM ERRO REAL

**Erro antes do fix** (reprodutivel):
```
pn532_ble.cpp:293: error: 'find_if' is not a member of 'std'
pn532_ble.cpp:671: error: 'reverse' is not a member of 'std'
chameleonUltra.cpp:63: error: 'Serial' was not declared
```
As libs NimBLE de terceiros usam `std::find_if`/`std::reverse` mas o patch so injetava `Arduino.h`. Faltava `<algorithm>`. **Corrigido.**

### 2.2 RF24 `1.4.11`/`1.5.0 -> ^1.6.2` -- COMPILA
- `platformio.ini` tinha **3 versoes diferentes** de RF24 entre os envs. Unificado em `^1.6.2` (latest).
- Sem erro de compilacao. API estavel.

### 2.3 RadioLib `7.4.0 -> ^7.7.1` -- COMPILA
- Todos os modulos RadioLib (SX127x, SX126x, LR11x0, LR2021, LoRaWAN) compilaram sem erro.
- O projeto usa `SX1276`/`SX1262` com API antiga (`begin(bandMHz)`, `setSpreadingFactor`, etc.) -- **mantida como deprecated no 7.7.x**, sem quebra.
- Nota: 7.7.0 depreciou `begin(lista)` -> `begin(ConfigStruct)`; o projeto nao usa essa forma.

### 2.4 ESPAsyncWebServer pin `@^3.11.2` -- COMPILA (escolha conservadora)
- O ini tinha `ESP32Async/ESPAsyncWebServer` **sem pin** (floating).
- Latest = `v3.12.0` (26/Jul/2026) mas tem **breaking change** no `abort()` e WebSocket. Para nao quebrar `willy_web.cpp`, **pinhei em `^3.11.2`**.
- Futuro: testar 3.12.0 com `willy_web.cpp` + WebSocket + upload/OTA.

### 2.5 ArduinoJson pin `@^7.4.3` -- OK
- `[env]` flutuante -> travado em `7.4.3` (= latest). Sem mudanca de API.

### 2.6 AnimatedGIF dedup -- OK
- Removida entrada duplicada do registry, mantido o zip.

### 2.7 NimBLE `2.3.7 -> 2.5.1` -- COMPILA
- Bump em todos os 5 pontos do `platformio.ini`.
- 2.5.1 e o latest (Jul/2026). API 2.x estavel.
- Fixes relevantes: scan crash, Arduino 3.3.11 build, C5/C6 variants, NimBLEAddress byte-order, use-after-free whiteListRemove.
- Sem breaking changes na API 2.x.

### 2.8 PNGdec `1.0.3 -> 1.1.2` + LVGL `8.3.11 -> 8.4.0` -- COMPILAM
- PNGdec: minor bump (latest 1.1.6), API estavel.
- LVGL: 8.3 -> 8.4 (mesma linha 8.x). `lv_conf.h` v8.3 continua valido. Sem breaking do 9.x.
- Validados em build CYD-2USB: ambos compilam antes do stop pre-existente.

### 2.9 Typo `willyConfig -> wilyConfig` -- CORRIGIDO
- `src/core/display.h` tinha 7 ocorrencias de `willyConfig` (typo). O correto e `wilyConfig` (declarado em `globals.h`).
- Pre-existente desde `89bb0f6`. O compilador ja sugeria o nome correto.
- **Este typo bloqueava a compilacao** -- foi revelado so agora porque os erros anteriores (ETH.cpp) paravam o build antes de chegar no codigo do projeto.

---

## 3. O que foi testado e NAO entrou (com justificativa)

### 3.1 Platform pioarduino `55.03.36 -> 55.03.311` (Arduino 3.3.11) -- NAO FAZER
- Tag `55.03.311` existe (Arduino 3.3.11 / IDF 5.5.5), mas no Windows local o toolchain `xtensa-esp-elf@14.2.0+20260121` **falhou na instalacao** (`cppbuiltin.h` etc. -> "No such file or directory").
- **Manter `55.03.36`** ate validar em ambiente limpo (Docker).

### 3.2 FastLED `3.10.3 -> 3.10.4` -- INDISPONIVEL
- Erro real: `UnknownPackageError: Could not find the package with 'fastled/FastLED @ ^3.10.4'`.
- Existe no GitHub, mas o PIO Registry nao resolve. **Manter `3.10.3`**.

### 3.3 `CONFIG_ETH_USE_ESP32_EMAC=1` -- REVERTIDO
- Adicionei `-DCONFIG_ETH_USE_ESP32_EMAC=1` no `[env]` para tentar resolver erro em `ETH.cpp`.
- O define **aparece na linha de comando** (confirmado via `-v`), mas o erro persiste -- e problema profundo de sdkconfig do arduino-esp32.
- **Revertido** porque nao resolve. O erro em `ETH.cpp` e **pre-existente** (ja ocorria no commit original `89bb0f6`).

---

## 4. Bloqueadores pre-existentes (NAO causados por mim)

### 4.1 ETH.cpp do framework nao compila no CYD-2USB
```
ETH.cpp:213: error: 'ESP32_BUS_TYPE_ETHERNET_RMII' was not declared in this scope
```
- **Causa**: `ETH.cpp` (puxado por `ARPScanner.cpp` e `EthernetHelper.cpp` via `#include <ETH.h>`) usa enums de `esp32-hal-periman.h` condicionais a `#if CONFIG_ETH_USE_ESP32_EMAC`. Sem sdkconfig correto, o enum nao existe.
- **E pre-existente**: o commit original `89bb0f6` ja falha aqui.
- **Impacto**: o build para no `ETH.cpp` (ou em interface.cpp, ver abaixo -- e intermitente por compilacao paralela).

### 4.2 interface.cpp -- conflito C++20 `std::numbers::e`
```
error: 'e' was not declared in this scope; did you mean 'std::numbers::e'?
```
- **Causa**: codigo nos headers de touch (`TouchDrvGT911.hpp` / `CYD28_TouchscreenC.h` / `CYD28_TouchscreenR.h`) usa `e` como identificador, que conflita com `std::numbers::e` do C++20 (GCC 14.2).
- **E pre-existente**: nao tem relacao com os bumps de dependencia.
- **Intermitencia**: como SCons compila em paralelo, as vezes ETH.cpp falha primeiro, as vezes interface.cpp. Os dois erros coexistem.

**Caminhos para resolver** (fora do escopo atual):
1. ETH.cpp: sdkconfig custom (`board_build.sdkconfig`) com `CONFIG_ETH_USE_ESP32_EMAC=y` -- forma correta no arduino-esp32.
2. interface.cpp: adicionar `#include <numbers>` + qualificar `std::numbers::e` no header de touch, ou renomear o identificador `e` no codigo do touch.
3. Validar em outro env (ex. S3) que nao puxa ETH.cpp.

---

## 5. Descobertas importantes sobre o projeto

1. **Muitas deps em `master` sem versionamento** -- `mQuickJS`, `AnimatedGIF`, `rc-switch`, `IRremoteESP8266`, `SimpleCLI`, etc. usam `refs/heads/master.zip`. Qualquer commit upstream pode quebrar o build. Risco real de supply-chain.
2. **3 versoes de RF24** conviviam -- eliminado.
3. **TFT_eSPI 2.5.43 vendored em `lib/`** -- nao esta abandonado (repo Bodmer esta ativo, so estagnado). Nao mexer por enquanto.
4. **LVGL 8.3.11** -- update para 8.4.0 e seguro (mesma linha); para 9.5.0 e rewrite grande (branch separado).
5. **PlatformIO local = 6.2.0** -- Dockerfile agora pinned compativel.
6. **`pioarduino 55.03.36`** e fork comunitario. Sem alerta formal de seguranca, mas ha risco de supply-chain (mantenedor unico, ZIP externo).

---

## 6. Ordem de execucao seguida (recomendacao)

1. Fix de patch (`<algorithm>`) -> build
2. Pin de flutuantes (ArduinoJson, AsyncWebServer) -> build
3. Bump patch/minor estavel (RF24, RadioLib) -> build
4. Bump com breaking (Async 3.12, LVGL 9, platform 3.3.11) -> branch separado + build
5. Opcional/pesado (LovyanGFX, IDF 6.x, JS engine) -> pesquisa + branch

**Regra de ouro**: 1 item = 1 commit = 1 build. Nunca misturar variaveis.
