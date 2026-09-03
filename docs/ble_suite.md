# BLE Suite (Willy)

Suíte avançada de testes BLE em `src/modules/ble/`. Menu: **Bluetooth → BLE Suite** (builds completos, fora do `LITE_VERSION`).

## Entrada

`void BleSuiteMenu()` — declarada em `BLE_Suite.h`, entrada adicionada no `BleMenu.cpp`. Sniffer dedicado: `BLE_Sniffer()` / `BLE_SnifferMenu()` em `ble_sniffer.h`.

## Ataques com alvo (`showAttackMenuWithTarget`)

| Ataque | Função |
|---|---|
| WhisperPair | `runWhisperPairAttack` |
| Exploit avançado | `runAdvancedExploit` |
| Crash de stack de áudio | `runAudioStackCrash` |
| Hijack de comando de mídia | `runMediaCommandHijack` |
| Injeção HID | `runHIDInjection`, `runForceHIDInjection`, `runHIDConnectionExploit`, `runAdvancedDuckyInjection` |
| DuckyScript | `runDuckyScriptAttack` |
| Força bruta de PIN | `runPINBruteForce` |
| Flood de conexão | `runConnectionFlood` |
| Spam de advertising | `runAdvertisingSpam` |
| Teste multi-alvo | `runMultiTargetAttack` |

Submenus por perfil: FastPair (`showFastPairSubMenu`), HFP (`showHFPSubMenu`), áudio, HID, memória, DoS, payload e testes.

## FastPair (`fastpair_crypto`)

`runFastPairScan`, teste de vulnerabilidade, corrupção de memória, confusão de estado, overflow cripto (`runFastPairCryptoOverflow`), spam de popup (`runFastPairPopupSpam`), corrente HFP↔HID (`runFastPairHIDChain`) e bateria de todos os exploits (`runFastPairAllExploits`).

## HFP (`HFP_Exploit`)

`runHFPVulnerabilityTest`, `runHFPAttackChain`, pivô HFP→HID (`runHFPHIDPivotAttack`, `runSmartHFPPivot`).

## Jam via NRF24

Menu **NRF24 → Ataques NRF24 → Jam BLE (NRF)** usa `src/modules/NRF24/nrf_jammer_api.h`:

```cpp
startBLEJammer(BLE_JAM_ADV_CHANNELS); // 37/38/39
// ... updateBLEJammer() em loop, ESC sai ...
stopBLEJammer();
```

Modos: `BLE_JAM_ADV_CHANNELS`, `BLE_JAM_ALL_CHANNELS`, `BLE_JAM_TARGET_CHANNEL`, `BLE_JAM_HOP_ADV`, `BLE_JAM_HOP_ALL`, `BLE_JAM_CONNECT_ATTACK`. Exige NRF24 detectado (`isNRF24Available()`).

## Diagnóstico

`takeSnapshot()` / `printReport()` / `showAttackProgress()` / `showAttackResult()` e log de exploits (`logExploitResult`). Clientes rastreados com `registerClient`/`cleanupAllClients`; `cleanupBLEStack()` no teardown.
