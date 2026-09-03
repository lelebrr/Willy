# Changelog da unificação Willy

Consolidação de `Willy_ESP_s3`, `Willy_ESP32_S3`, `Cyd_Lele`, `Esp32-S3` e Bruce `firmware-main` em `F:\Willy` (as duas do meio eram esqueletos vazios — só estrutura de pastas).

## Arquivos incorporados

- **+205**: `bjs_interpreter` modular, `web_server`/`web_files`, gerenciadores de sistema, `lib/mquickjs`, `lib/HAL`, `lib/stubs`, boards N8R2/N16R8, `custom_8Mb.csv`, `Willy_ili9341_custom.bin`, `WillyWebUI`, `system_config.json`, `example_plugin.json`, docs e mídia de hardware (+45 atualizados p/ versão maior).
- **+203**: BLE Suite/sniffer/HFP/FastPair, stack RF (decoder/encoder/Keeloq/registry/presets), NetCut/SOCKS4/channel-analyzer/jam-detect, ST25R3916, U2F, wdgwars, `bus_HAL`, `ram_profile`, bindings JS (ble/buffer/led/menu/nrf24/rfid), 20 famílias de placas, `custom_16Mb.csv`, `ssid_list.zip`, `tools/gen_mfcodes.py`, `rfid_commands`.
- **+36**: PCBs/gerbers, fotos, CI (`.github`), `nrf_spectrum.cpp.bak`.
- `platformio.ini`: envs `willy_release`/`willy_debug`/`test` anexados sem quebrar o multi-placa.

## Correções de compilação/integração

`sys_i2c` + `ST25R_bus` + `ST25R3916_I2C_MODULE` no enum, `validateRfidModuleValue()` preservando ST25R, `wdgwarsApiKey` + `ensureMifareKeysLoaded()`, `SYS_I2C_SDA/SCL` fallback, `rfid`/`hw` registrados no CLI, menus (NetCut/SOCKS4/Channel/Jam/BLE Suite/U2F/ST25R pins), `gpt_tracker.cpp` vazio removido.

## Otimizações

- **Visual**: destaque de linha selecionada, cartões neon nos diálogos, títulos com barra, status 2 tons, footers com divisórias, progresso arredondado, splash com versão dinâmica + cor do tema, Cyber Menu por nome, LVGL sem 13 widgets ociosos, `lv_conf.h` único, `web_server.cpp` fora do build.
- **Desempenho**: SSID com cache-first no sniffer, `cleanupStaleBeacons` O(n), sem `String` a 100 Hz no menu, watchdog I2C a cada 1 s.
- **Comunicação**: I2C usuário a 400 kHz, hostname DHCP `Willy-<MAC>`.
- **`WiFi.scanDelete()` em 17 pontos** (vazamento de heap do driver).

## 50 itens

Menus (ST25R ×2, tokens Wigle/WDGWars, upload WDGWars, autotestes RF, info/remount SD, status WiFi, ver chaves, Sobre, Jam BLE/NRF, layout de teclado, título ST25R); CLI (`subghz selftest/keeloqtest/keeloqfiletest`, `version`, `tasks`, `echo`, `sleep_ms`); web (`/api/version`, versão/PSRAM no painel, CSS, pausa com aba oculta); robustez (scroll, force-clamp, deauth reset, wifiIP, timeout Ethernet, dedup QR/menus); 17× scanDelete.

---

## Rodada 80+ melhorias gerais

- **Comunicação/identificação**: campo `hostname` (Config → Avançado → Nome do Host, serial `settings hostname`, DHCP + mDNS unificados); `wifi scan` no serial (lista SSID/RSSI/canal com `scanDelete`); gateway personalizado no Evil Portal; porta do SOCKS4 configurável (padrão 1080); WDGWars com opção enviar-e-apagar; credenciais WebUI pelo menu.
- **Robustez**: timezone do WireGuard via `tmz` (era JST fixo); segredos WG fora do Serial; BadUSB USB com saída ESC + timeout 30 s.
- **PT-BR**: ~100 strings de UI traduzidas (connect, WiFi, BLE, LoRa, RF, RFID, NFC, NRF24, GPS/WDGWars, IR, áudio, iButton, netcut, sniffer, recover, WPS, QR, Evil Portal, FM, QR demos).
- **Conteúdo**: `sd_files/interpreter/info.js` (dados do aparelho) e `dialogo.js` (demo das caixas de diálogo).

---

## Temas: +16 novos e motor otimizado

- **8 presets embutidos** (Cor UI): Cyberpunk, Matrix, Ocean, Sunset, Blood, Gold, Ice, Dracula.
- **8 packs SD** (`sd_files/themes/<nome>/theme.json`) no schema do firmware + mesmos nomes no menu.
- **Web**: 3 alternativos novos no `theme.css` (Dracula, Nord, Ouro).
- **Motor**: `validateImgFile()` real (extensão + tamanho) e ligada no carregamento (rejeita imagem inválida); erro `displayError("5")` virou `"Tema invalido"`; `removeTheme()` limpa `themePath`; `neon_cyber/theme.json` ganhou chaves de firmware.

---

## Rodada 120+ (código + PT-BR em massa)

- **Código**: `hostname` configurável (DHCP + mDNS), credenciais WebUI pelo menu, porta SOCKS4 configurável, WDGWars enviar-e-apagar, gateway personalizado no Evil Portal, `wifi scan` no serial, atalho Layout Teclado no BadUSB, `isConnectedWireguard` zerado no disconnect, `lvgl_rendering_active` zerado ao sair do Cyber Menu, **Reverse Shell no menu WiFi** (+ desconexão limpa ao sair), **diagnóstico de Joystick** (Outros, guarded `JOY_*_PIN`), removidas declarações mortas (`loraconf`, `loadIoTExploits`).
- **PT-BR**: ~110 strings de UI traduzidas (connect/ESP-NOW, WiFi, BadUSB, BLE, LoRa, áudio, iButton, RF, Chameleon, Tag-O-Matic, PN532, SRIX, NetCut, sniffer, recover, WPS, WDGWars, IR custom, massStorage, i2c_finder, teclado, QR, Evil Portal, FM, settings).
- Docs sincronizados: `evil_portal.md` (gateway), `serial_cli.md` (`wifi scan`, hostname), `wifi_netcut_socks.md` (porta), `wardriving_wdgwars.md` (enviar-apagar), `settings_guide.md` (hostname, WebUI).

---

## Limpeza geral (mortos e duplicados removidos)

- **Deletados**: `src/web_server.*` + `src/web_files/` (demo órfã), `src/core/globals.h` (sombra conflitante), `src/core/headless_mode.h`, `User_Setup.h` (raiz e `include/`, obsoletos), `src/core/USBSerial/BruceUSBSerial.h` (classe com nome errado), `lib/mquickjs/` (~700 KB, gerador usa o baixado), `lib/stubs/{lvgl,FastLED_fixes}.h`, `lib/utility/Keyboard.*`, `test/test_joystick_ir.cpp` (superado pelo diagnóstico no firmware), `src/idf_component.yml.orig`, `include/README`, `lib/README`, `__pycache__`, `sd_files/ssid_list.zip`, `sd_files/wifi/exploits/*.json` (9 arquivos órfãos, nada lia), `firmware_dump.txt` (4 MB), ~50 MB de mídia não referenciada (arte IA, branding antigo, duplicatas).
- **ODR unificado**: `include/willy_logger.h` removido; todos incluem `core/willy_logger.h` (duas definições diferentes da mesma classe = corrupção silenciosa).
- **Build destravado**: criado `lib/TFT_eSPI/User_Setups/Setup_ESP32_S3_N8R2_ILI9341.h` (3 envs quebravam por arquivo ausente); stubs `SimpleCLI` com fallback `__has_include` (path hardcoded quebrava fresh builds).
- **Testes**: `test/Makefile` apontado para `include/willy_config.h` + `src/ui/willy_config.cpp` (caminhos obsoletos pós-refactor).
- `platformio.ini` sem filtros obsoletos.

---

## Bateria real + pinos auditados

- **Bateria funcional**: `elecrow_advance_s3` devolvia 100%% fixo (agora 0=oculto); `drawBatteryStatus()` recriado (ícone 22x12, verde/amarelo/vermelho ≤15%%, cor do tema carregando) na status bar com cache de 10 s; Pwnagotchi `UPS %%` real; BLE GATT com nível real (init + notify/min); Cyber Menu com `%%` ao vivo; `0` = sem monitor (CYD/N8R2 sem circuito). Ver `battery.md`.
- **Pinos**: N8R2 auditado sem conflitos reais; `SPIPins::checkConflict()` cobre `io0`/`io2`; guia `pinout_conflitos.md` (mapa N8R2, zonas proibidas 33–37/19–20 no S3, regras multiboard).
