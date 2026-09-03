# Evil Portal (`src/modules/wifi/evil_portal.{cpp,h}`)

Portal captivo com captura de credenciais em `default_creds.csv`.

## Fluxo

`EvilPortal()` → `setup()` (escolhe HTML, SSID e gateway) → `beginAP()` → `setupRoutes()` → `loop()` (task `EvilPortalLoop` 4 KB) com `CaptiveRequestHandler` (DNS + HTTP). Rotas: `/` (portal), `/post` (creds), endpoints configuráveis de `/creds` e `/ssid` (`evilPortalEndpoints`: renomear, permitir GET, exibir endpoints).

## Opções no setup

- **HTML**: `Padrao` (embutido, `loadDefaultHtml[_one]`) ou `HTML Personalizado` (`loadCustomHtml`, arquivo do SD; ver `sd_files/portals/` com 14 templates en/pt-br).
- **SSID**: digitado ou da lista `evilWifiNames` (Config WiFi).
- **Gateway**: `172.0.0.1`, `192.168.4.1` ou **Personalizado** (validado com `IPAddress::fromString`).
- **Verificação de senha** (`_verifyPwd`, modo APSTA) e **deauth associado**: parâmetros do construtor (`EvilPortal(tssid, channel, deauth, verifyPwd)`); o menu usa os padrões.

## Credenciais

`credsController()` salva em CSV e conta (`totalCapturedCredentials`); modo verify compara `passwordValue` antes de aceitar.
