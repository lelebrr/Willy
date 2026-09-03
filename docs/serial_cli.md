# CLI Serial — referência completa

Console em 115200 baud (`# ` prompt). Entrada validada: letras/números, espaço, `-_. /"`; sequências `..` e `//` bloqueadas; máx. 1024 chars. `forceMenuOption` permite navegar menus pelo serial (índices fora da faixa são ignorados).

## Comandos

| Comando | Uso |
|---|---|
| `uptime` | `HH:MM:SS` desde o boot |
| `version` | firmware + chip/revisão + flash/PSRAM + heap livre |
| `tasks` | tabela de tasks FreeRTOS (`vTaskList`) |
| `echo <texto>` | ecoa texto (útil em scripts) |
| `sleep_ms <ms>` | pausa 0–5000 ms (útil em scripts) |
| `date` | data/hora (exige relógio sincronizado) |
| `i2c` | scanner I2C no `i2c_bus` |
| `free` | heap total/livre (+PSRAM se houver) |
| `info`, `!`, `device_info` | infos do dispositivo |
| `help`, `?`, `halp` | ajuda |
| `optionsJSON` | opções do menu em JSON |
| `display ...` | controle de tela (padrão `dump`) |
| `nav <cmd> [dur]` | navegação remota |
| `options`, `loader` | opções / carregador de apps |
| `wifi off` | desliga WiFi |
| `wifi on` | conecta em rede conhecida (ou abre AP) |
| `wifi add <SSID> <pwd>` | salva credencial |
| `webui [--noAp]` | inicia WebUI (ESC sai) |
| `arp` | scan de hosts (exige STA) |
| `listen <porta>` | TCP listen (exige WiFi) |
| `sniffer` | inicia sniffer |
| `rf ...`, `subghz ...` | `rx [freq] [--raw]`, `tx <key> ...`, `scan`, `tx_from_file`, `tx_from_buffer`, `selftest`, `keeloqtest`, `keeloqfiletest`, `RfSend` |
| `rfid ...` | `read [timeout]`, `emulate t4t|felica`, `loadfile`, `ndef`, `write` |
| `hw <sub>` | `info`, `pininfo`, `peripheral`, `configpin <pin> <modo>`, `testpin <pin> read\|write`, `i2cscan [sda scl]`, `hwreport`, `resetpins`, `periphstatus`, `loadhwconfig`, `savehwconfig` |
| `ir ...` | `rx`, `tx`, `txraw`, `tx_from_file`, `tx_from_buffer` |
| `gpio ...` | controle de pinos |
| `crypto ...` | `encrypt/decrypt/type` (arquivo) |
| `poweroff`, `reboot`, `sleep`, `power ...` | energia |
| `set <campo> [valor]` / `tings` | lê/define config (`settings`), ex.: `set wdgwarsApiKey <chave>` |
| `factory_reset` | apaga configs |
| `ls,dir`, `cat,type`, `md5`, `crc32`, `rm`, `md`, `rmdir`, `storage ...` | arquivos |
| `badusb ...` | `run_from_file`, `run_from_buffer` (só `USB_as_HID`) |
| `interpreter ...` | JS (fora do `LITE_VERSION`) |
| `screen ...` | tela (com `HAS_SCREEN`) |
| `sound ...` | som (com speaker/buzzer) |
| `benchmark ...`, `plugin ...`, `dynamic_config ...`, `hardware ...` | frameworks Willy |

Mensagens `COMMAND:` + `[CLI] Result:` ecoam no log. Erros sugerem comandos próximos ("Did you mean").

---

## Novidades

- `wifi scan`: lista redes próximas (SSID | RSSI | canal) e libera o heap do scan.
- `settings hostname <nome>`: define o hostname (DHCP + mDNS).
