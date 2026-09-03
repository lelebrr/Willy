# Cyber Menu UI (Willy)

Menu alternativo em LVGL 8.3 (`src/ui/cyber_menu.{cpp,h}`), com tiles por página (`lv_tileview`), barra de notificação (hora via NTP + ícones WiFi/BLE) e callbacks que roteiam para os menus clássicos.

## Acesso

Menu principal → **Cyber Menu (Exp)**, ou automático no `loop()` (`setup_cyber_menu()` uma vez por retorno ao menu).

## Roteamento por nome

Cada tile resolve o índice em `mainMenu.getItems()` comparando `getName()` (`set_cyber_action()`): imune à ordem dos menus, a defines (`LITE_VERSION`, `NO_RF_MODULE`…) e a menus ocultos. Tiles sem menu dedicado roteiam para o mais próximo (NFC→RFID, Attacks→WiFi, Core→Configuração, Logs→Arquivos). Índice não encontrado = ação ignorada.

Tiles: Wi-Fi, BLE, IR, NFC, Sub-GHz, NRF24, GPS, Attacks, Core, Logs, RFID, SD.

## Visual

`create_cyber_icon()`: botão 90×90 (ampliado p/ 180 na página) com gradiente vertical primária→secundária (`bruceConfig`), borda de destaque ciano, sombra neon pulsante infinita, fade-in + slide-up em cascata, zoom 1.2x no toque. Setas de navegação por toque nas laterais.

## Cores

Usa `bruceConfig.priColor/secColor` (não `WillyConfig`) — acompanha o tema do sistema automaticamente.

## Barra superior

Hora (`HH:MM` quando NTP sincronizado, `--:--` senão), bateria (placeholder `--%`: `getBattery()` é stub fraco e retorna 0 sem implementação da placa), ícones WiFi/BLE e popup "System Ready" com fade.
