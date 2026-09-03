# NetCut, SOCKS4, Channel Analyzer, Jam Detect (WiFi)

Ferramentas de rede no menu **WiFi** (`src/modules/wifi/`).

## NetCut (`netcut.h`)

Menu **WiFi → NetCut** (`netcutMenu()`). Poisoning/restore ARP via `linkoutput()` da LwIP (sem `esp_wifi_internal_tx`), descoberta reaproveitando o `ARPScanner`, UI em `loopOptions()`, VIPs persistidos no LittleFS. **Exige STA conectado.** Modo Troll é bloqueante (sai com ESC).

## SOCKS4 Proxy (`socks4_proxy.h`)

Menu **WiFi → Proxy SOCKS4** → `socks4Proxy(1080)` (builds completos). Servidor SOCKS4 sobre `WiFiServer`/`WiFiClient`. Uso no PC:

```bash
ssh -o ProxyCommand='nc -X 4 -x <IP_DO_ESP>:1080 %h %p' user@alvo
```

ou aponte o proxy do sistema/navegador para `<IP_DO_ESP>:1080`.

## Channel Analyzer (`channel_analyzer.h`)

Menu **WiFi → Analisador de Canais** (`channel_analyzer_setup()`, builds completos). Salta canais 1–11 em modo promíscuo, estima ocupação (airtime), desenha barras com %, pico guardado e medidor de sinal.

## Jam Detect (`jam_detect.h`)

Menu **WiFi → Detectar Jam** (`jam_detect_setup()`, builds completos). Observa taxa de quadros deauth/disassoc no canal e alerta visual acima do limiar ajustável.

---

## Porta do SOCKS4

O menu pergunta a porta (`num_keyboard`, padrão 1080, 1–65535) em vez de fixa.
