# Protocolos RF + Keeloq (`src/modules/rf/protocols/`)

Stack OOK/Sub-GHz reescrita: registro central, decoder/encoder compartilhados, presets e Keeloq.

## Arquivos

| Arquivo | Papel |
|---|---|
| `rf_protocol.h` / `rf_registry.{cpp,h}` | tabela estática de protocolos + `rf_find_protocol()` |
| `rf_decoder.{cpp,h}` / `rf_encoder.{cpp,h}` | engines compartilhados de decode/encode |
| `rf_config.h` | `RF_DBG()` (Serial) e flags |
| `rf_presets.{cpp,h}` | presets de rádio + `rf_find_preset()` |
| `rf_keeloq.{cpp,h}` | Keeloq: `keeloq_build_hop()`, encrypt/decrypt, `keeloq_derive_man()`, `kl_check()` |
| `rf_keeloq_mfcodes_data.h` | keystore embarcado (gerado por `tools/gen_mfcodes.py`, AES-256-CBC) |
| `rf_legacy_migrate.{cpp,h}` | migração dos protocolos legados (`Ansonic.h`, `Came.h`, …) |

Os `protocols/*.h` legados de um arquivo por fabricante continuam existindo e funcionando.

## Keystore `/mfcodes`

`keeloq_mfcodes_fs()` procura `/mfcodes` no armazenamento ativo (SD quando montado, senão LittleFS). `KeeloqKeystore` lê as chaves; `tools/gen_mfcodes.py` regenera o header embarcado.

## Autotestes

Menu **RF → Autotestes RF** roda `rf_encoder_selftest()` + `rf_keeloq_selftest()` e mostra PASS/FAIL na tela. CLI serial (`rf`, alias `subghz`):

```
subghz selftest        # golden test do encoder
subghz keeloqtest      # round-trip por fabricante
subghz keeloqfiletest  # round-trip com o /mfcodes real (falha se ausente/vazio)
```
