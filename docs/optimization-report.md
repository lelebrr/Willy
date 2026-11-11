# Otimização de funções — Willy (CYD-2USB)

Branch: `optimize/all-functions` (base: `c5e9a81` "FDSD")

## Contexto

O `main` do repositório continha apenas artefatos de build (`.pio/`, logs).
O código-fonte real vive nos branches; a base adotada foi `c5e9a81`, ponto de
convergência dos 48 branches de otimização que estavam abertos no remoto.

Antes de otimizar, o build **não compilava**. Três quebras pré-existentes
foram corrigidas (commit `e53b423`):

1. `src/modules/rf/rf_utils.cpp` — marcadores de conflito de merge
   commitados no código (merge `ea2366f` do PR #45). Resolvidos seguindo a
   resolução já adotada upstream no branch `feature/persist-recent-rf-codes`.
2. `src/core/config.h` — declaração de `BruceConfig::setSetting()` perdida
   no merge `5957458`; o `.cpp` continuava com a definição.
3. 12 call-sites de `wifiConnectMenu()` / `wifiConnected` obsoletos após os
   refactors `e6613d9` (classe `WifiCommon`) e `ec4d939` (namespace
   `WifiState`).

## Otimizações aplicadas (52 arquivos)

### Eliminação de alocação de heap por byte/caractere
`String` do Arduino aloca no heap a cada `+`, `substring()` ou `+=`. Os
caminhos quentes foram reescritos para escrever direto em buffer de pilha:

| Local | Antes | Depois |
|---|---|---|
| `type_convertion.cpp` `hexToStr` | 1 `String` temporário por byte | escrita direta no acumulador |
| `config.cpp` `encryptString`/`decryptString` | `snprintf` + `substring` por byte | tabela hex + `reserve()` |
| `display.cpp` `padprintln` | `String(str).substring()` por linha (O(n²)) | `memcpy` em buffer de pilha |
| `display.cpp` `drawOptions`/`listFiles` | 3 Strings temporários por item | buffer de pilha |
| `rfid/*` (PN532, RFID2, chameleon, pn532ble, rfid125, srix_tool) | `String(v, HEX)` por byte | tabela hex direta |
| `ir/ir_read.cpp` | `String(v, HEX)` / `String(v)` por amostra | tabela hex / `itoa()` direta |
| `sniffer.cpp` `extractSsid` | append por caractere | buffer de pilha |
| `mqjs_bindings.cpp` | `String(path).startsWith()` | `strncmp` |

### Redução de complexidade (O(n²) → O(n))
- `rfid/chameleon.cpp`, `rfid/PN532.cpp`, `rfid/RFID2.cpp` — `strAllPages` era
  recriado a cada página (`substring` do resto). Agora percorre por índice.
- `ir/custom_ir.cpp` — parse do sinal RAW usava `remove(0, ...)`, deslocando
  a string inteira a cada token.

### Redução de I/O
- `rfid/pn532ble.cpp` `saveHfDumpBinFile` — dump gravado em 1 `write()` em vez
  de 1 por byte.
- `rfid/srix_tool.cpp` — dump de 128 blocos montado em memória e gravado de
  uma vez (128 `write()` → 1).
- `wifi/wps_atks.cpp` — o arquivo de redes quebradas era relido do SD **por
  AP escaneado**; agora é carregado uma vez e consultado em memória.

### Redução de trabalho repetido
- `modules/others/mic.cpp` — o plano FFT (`fft_init`/`fft_destroy`) era criado
  e destruído **a cada quadro** (~46/s). Agora vive uma vez.
- `wifi/wifi_atks.cpp` — `WiFi.SSID(i)`/`BSSIDstr(i)`/`channel(i)` devolvem
  `String` por valor e eram chamados 2–3× por AP. Agora são cacheados.
- `core/main_menu.cpp` — o vetor de menus desabilitados era **copiado** a cada
  item do menu.
- `core/willy_web.cpp` — leitura do log byte a byte (5 000 `read()`) → 1 bloco.

### Assinaturas com `const String&`
`display.h`, `sd_functions.h`, `mykeyboard.h`, `audio.h`, `wifi_atks.h`,
`rf_send.h`, `rf_scan.h`, `file_utils.h`, `custom_ir.h`, `utils.h` — evita a
cópia da `String` na entrada da função.

### Segurança de stack
- `core/massStorage.cpp` — removida VLA de 512 bytes na pilha (por setor no
  callback USB) e o `memcpy` redundante.
- `core/sd_functions.cpp` `pasteFile` — buffer de 1 KB da pilha → estático.
- `modules/wifi/karma_attack.cpp` — VLA `char ssid[len+1]` → buffer de 33.

## Verificação

### Build
`pio run -e CYD-2USB` — **SUCCESS** (sem erros nem warnings novos).

| | baseline (`c5e9a81`) | final |
|---|---|---|
| RAM estática | 99 184 B (30,3 %) | 100 240 B (30,6 %) |
| Flash | 3 892 991 B (99,0 %) | 3 886 159 B (98,8 %) |

O aumento de 1 KB de RAM é o buffer estático de `pasteFile` (troca de stack
para heap estático). O flash **diminuiu** apesar de todas as otimizações.

### Teste de equivalência
`test/verify_opt_equivalence.py` — 22 testes que comparam a saída das
funções otimizadas com a das originais. Roda sem dependências:

```
python test/verify_opt_equivalence.py
```

Cobre: `hexToStr`, `checkExt`, `sortList`, `encrypt`/`decrypt` (round-trip e
hex ímpar), `hexCharToDecimal`, `padprintln`, `beaconSpamSingle`,
`srix dump`, `rfid page`, `ir state`, split de páginas do chameleon, parse IR
RAW, cache do WPS, case do hex no pn532ble, terminador CRLF, contrato
público do `checkExt`, progresso do chameleon, cache do WPS, ausência de
cache de handles no sniffer e ausência de use-after-free em `/api/logs`.

## Semântica de referência (verificada no ROM do ESP32)

`String(uint8_t v, HEX)` → `utoa(v, buf, 16)`. A tabela do newlib é
`0123456789abcdef` — **minúscula e sem zero-padding**
(`utoa(0x0a, 16) == "a"`). Verificado no `esp32_rev0_rom.elf`, seção
`.rodata` em `0x3ff9c4e2`.

Consequências aplicadas:
- o `'0'` manual do original foi preservado onde completava o segundo dígito;
- `toUpperCase()` é `void` (altera in-place) — nunca usado em expressão;
- `sortList` original era `toUpperCase()` + `operator<` (= `strcmp`); `strcasecmp`
  **não** é equivalente (ordena `_` diferente de letras) e foi substituído por
  comparação byte a byte com `toupper()`.

## Correções de bugs introduzidos pelas otimizações

Detectados por revisão e pelo teste de equivalência, e corrigidos (commit
`c456d6c`):

- **use-after-free** em `willy_web.cpp` `/api/logs`: `AsyncProgmemResponse`
  guarda o ponteiro do conteúdo e o lê depois; o `free()` liberava a memória
  antes do envio.
- **Esgotamento de handles do SD** no sniffer: o cache de arquivos PCAP mantinha
  um handle por handshake, mas o SD aceita no máximo `max_files` (5) abertos
  — handshakes passavam a ser perdidos silenciosamente. Cache revertido.
- **Hex maiúsculo virou minúsculo** em 3 blocos do `pn532ble.cpp`.
- **Barra de progresso travada em 0 %** no `chameleon.cpp`.
- **CRLF → LF** no dump do `srix_tool.cpp` (`file.println()` gravava CRLF).
- **Cache envenenado** no `wps_atks.cpp` quando o SD não está montado.
- **Hex ímpar / dígitos inválidos** no `decryptString` divergiam do `strtol`
  original.
- `checkExt` deixou de normalizar o `pattern` (contrato da API pública).
- Buffer fixo de 160 bytes truncava nomes LFN (até 255 chars) na WebUI.
- Log do RF passou a gravar o payload pré-conversão em BinRAW.
- Nome do arquivo em reprodução de áudio voltou a ser minúsculo.

## Commits

```
e53b423 fix: resolve pre-existing build breaks
31cd46c perf: optimize string handling and hot paths across core and modules
4debded perf: remove per-byte String allocations in RFID/IR hex formatting
9b95c7a perf: fix hex formatting equivalence and add equivalence test suite
c456d6c fix: address review findings in optimizations
75b4d7f perf: fix strtol semantics and apply second round of optimizations
2c5f609 perf: optimize scrollableTextArea and karma_attack hot paths
```
