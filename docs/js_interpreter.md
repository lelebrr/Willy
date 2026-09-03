# Interpretador JavaScript (`src/modules/bjs_interpreter/`)

Motor mQuickJS. Menu **Interpretador JS**: lista `.js` de `/scripts`, `/WillyJS` ou `/BruceScripts` (SD ou LittleFS), "Carregar..." para escolher arquivo, app de boot configurável.

## Runtime

256 KB de arena com PSRAM (64 KB sem), `js_bruce_init()` registra a API `bruce`, timers (`setTimeout`/`setInterval` via `globals_js`), `run_timers()` após `JS_Eval`. Cabeçalhos gerados no build (`GEN_MQJS_HEADERS` + `lib/mquickjs_headers/mqjs_stdlib_generator.exe`); `mqjs_stdlib.c` é excluído do build e regenerado.

## Bindings nativos (`*_js.cpp`)

| Módulo JS | Arquivo |
|---|---|
| `audio` | `audio_js` |
| `badusb` | `badusb_js` |
| `ble` | `ble_js` |
| `Buffer` | `buffer_js` |
| `device` (bateria, heap, placa, SDK) | `device_js` |
| `dialog` (error/warning/info/success) | `dialog_js` |
| `tft`/`display` | `display_js` |
| timers, print, Date, assert, require | `globals_js` |
| `gpio` (incl. pinos da placa) | `gpio_js` |
| helpers | `helpers_js` |
| `i2c` | `i2c_js` |
| `ir` | `ir_js` |
| `keyboard` | `keyboard_js` |
| `Math` extra | `math_js` |
| `mic` | `mic_js` |
| `notification` | `notification_js` |
| `nrf24` | `nrf24_js` |
| `rfid` | `rfid_js` |
| runtime | `runtime_js` |
| `serial` | `serial_js` |
| `storage` | `storage_js` |
| `subghz` | `subghz_js` |
| `wifi`, `http` | `wifi_js` |
| `menu`, `led` | `menu_js`, `led_js` |

Exemplos prontos em `sd_files/interpreter/` (jogos, calculadora, `wifi_brute.js`, `ir2keys.js`, `rf_brute.js`, `crypto-prices.js`…).

---

## Scripts demo (`sd_files/interpreter/`)

- `info.js`: placa, CPU, flash, SDK, heap livre e bateria via `compat`/`device`.
- `dialogo.js`: demonstra `dialog.info/success/warning/error`.
