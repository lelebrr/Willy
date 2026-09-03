# Build e envs (`platformio.ini`, `boards/`)

## Envs

| Env | Uso |
|---|---|
| `[env]` (+`[env_light]`) | base Bruce multi-placa (Arduino 3.3.6 via pioarduino), `default_envs = CYD-2USB` |
| `[env:willy_release]` / `[env:willy_debug]` | ESP32-S3 DevKitC-1 N8R2, 8 MB (`custom_8Mb.csv`), ILI9341 dedicado, `-O2 -flto` no release |
| `[env:test]` | Unity no device (`test/`) |

`extra_configs = boards/*.ini boards/*/*.ini`: cada placa adiciona seus envs/flags automaticamente.

## Placas (`boards/`)

Cada família tem `*.ini` (env + `-D` de pinos), `interface.cpp` (`_setup_gpio`/`_post_setup_gpio`: display, touch, PMIC, `setSysI2CBus` quando aplicável), `pins_arduino.h` e JSON em `_boards_json/` (`boards_dir`). Famílias: CYD (várias), ESP32-S3 N8R2/N16R8, ESP32-C5, ES3C28P, Elecrow, LilyGo (T-Deck, T-Display, T-Embed, HMI, LoRa Pager, T-Watch, TTGO), M5Stack (Cardputer, Core/2/S3, CPlus, StickS3, DinMeter), Marauder Mini/Touch, Phantom, Reaper, Smoochiee, xk404, Nesso, nm-cyd-c5 + template `_New-Device-Model`.

## Filtros e flags importantes

`build_src_filter` exclui o gerado `mqjs_stdlib.c`. Flags-chave: `GEN_MQJS_HEADERS`, `board_build.partitions` (`custom_4Mb.csv`, `custom_8Mb.csv`, `custom_16Mb.csv`), `ARDUINO_USB_CDC_ON_BOOT`, `USER_SETUP_LOADED`, `LV_TICK_CUSTOM`.

## Scripts (`scripts/`, `pre:`/`post:`)

`patch.py` (inclui gerar `include/webFiles.h` gzip a partir de `embedded_resources/web_interface`), `gen_mqjs_headers.py`, `pre_build_current_year.py`, `build.py`, `fix_compiler_flags.py`, `patch_i2s.py`, `flto_prep.py`, `patch_library_conflicts.py`, `tools/gen_mfcodes.py`.
