# Wardriving, Wigle e WDGWars (GPS)

## Wardriving (`src/modules/gps/wardriving.cpp`)

Menu **GPS → Wardriving**: varredura WiFi (scan bloqueante com `scanDelete()` liberando heap), BLE (com `pBLEScan->stop()` no fim) ou ambos. CSV hierárquico em `/WillyWardriving`, cache de MACs com limite anti-estouro (`enforceRegisteredMACLimit`), `alert.txt` com MACs de interesse (`checkForAlert`), `scanDelete()` após consumo e yields a cada 32 redes.

Formatos: CSV (planilhas), upload Wigle, WDGWars. Tracker dedicado: **GPS → Rastreador GPS** (`GPSTracker`, suspende o CLI serial durante a captura e retoma ao sair).

## Wigle (`wigle.cpp`)

Upload para Wigle.net com token Basic em `bruceConfig.wigleBasicToken`. Configure em **Config → Avancado → Token Wigle** (teclado, até 64 chars) ou via serial `settings wigleBasicToken <token>`.

## WDGWars (`wdgwars.{cpp,h}`)

Conector de upload CSV para `wdgwars.pl` (`WDGoWars::upload()` / `upload_all()`, TLS inseguro por limitação do ESP).

- Chave: 64 hex de `wdgwars.pl/profile` em `bruceConfig.wdgwarsApiKey` — **Config → Avancado → Chave WDGWars** ou `settings wdgwarsApiKey <chave>`. Sem chave válida, o upload aborta com erro na tela.
- Menu **GPS → Enviar p/ WDGWars**: exige WiFi (oferece conectar), usa `/WillyWardriving` do SD (se montado) ou LittleFS, `auto_delete=false`.
- Config GPS avançada (`gps_config.h`): taxa 1–10 Hz (5 Hz recomendado p/ wardriving), economia de energia, protocolo NMEA/UBX, filtro de satélite.

---

## Enviar e apagar

`GPS → Enviar+Apagar WDGWars` confirma e chama `upload_all(..., auto_delete=true)`.
