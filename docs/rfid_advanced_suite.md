# RFID Advanced Suite - Willy Firmware

## Visão Geral
Suite completa de 55+ funções RFID para o Willy Firmware. Implementações REAIS usando drivers PN532, RFID2 (MFRC522) e ST25R3916. Acesse via menu **RFID → RFID Suite Avancada**.

## Arquitetura
- **Header**: `src/modules/rfid/rfid_advanced.h` (200 linhas)
- **Implementação**: `src/modules/rfid/rfid_advanced.cpp` (~2800 linhas)
- **ST25R3916 wrappers**: `src/modules/rfid/ST25R3916.h` (11 novos métodos públicos)
- **Integração**: `src/core/menu_items/RFIDMenu.cpp`
- **Depende de**: RFIDInterface, PN532, RFID2, ST25R3916, crypto1.h, apdu.h

## Categorias (7 menus)

### 1. MIFARE Classic (12 funções)
| Função | Descrição | Requisitos |
|--------|-----------|------------|
| `rfidMifareClassicFullDump` | Dump completo de todos os setores com recuperação de chaves | ST25R3916 |
| `rfidMifareClassicSectorAuth` | Testa todas as chaves conhecidas contra setor específico | ST25R3916 |
| `rfidMifareClassicKeyRecovery` | Recupera chaves de autenticações bem-sucedidas | ST25R3916 |
| `rfidMifareClassicSectorMap` | Mapeia todos os setores com status de auth | ST25R3916 |
| `rfidMifareClassicBlockRead` | Lê bloco específico com autenticação | ST25R3916 |
| `rfidMifareClassicBlockWrite` | Escreve dados em bloco específico | ST25R3916 |
| `rfidMifareClassicValueBlock` | Lê/escreve blocos de valor (incremento/decremento) | ST25R3916 |
| `rfidMifareClassicAccessBits` | Analisa condições de acesso de todos os setores | ST25R3916 |
| `rfidMifareClassicKeyDictionary` | Brute-force com dicionário de 16 chaves | ST25R3916 |
| `rfidMifareClassicClone` | Clona dump MFC para tag magic Gen1/Gen2 | ST25R3916 |
| `rfidMifareClassicEmulate` | Emula tag MIFARE Classic | ST25R3916 |
| `rfidMifareStaticNonceDetect` | Detecta vulnerabilidade de nonce estático | ST25R3916 |

### 2. Ultralight & NTAG (10 funções)
| Função | Descrição | Requisitos |
|--------|-----------|------------|
| `rfidUltralightFullDump` | Dump completo de todas as páginas | Qualquer |
| `rfidUltralightPageRead` | Lê página específica | Qualquer |
| `rfidUltralightPageWrite` | Escreve em página específica | Qualquer |
| `rfidUltralightPasswordAuth` | Testa autenticação 3-step | Qualquer |
| `rfidUltralightOTPRead` | Lê bytes OTP com explicação | Qualquer |
| `rfidUltralightCounterRead` | Lê contadores monônicos | ST25R3916 |
| `rfidNTAGVersionInfo` | Exibe informações de versão NTAG | ST25R3916 |
| `rfidNTAGSignatureRead` | Lê assinatura ECC-P256 | ST25R3916 |
| `rfidNTAGPasswordProtect` | Configura/verifica proteção por senha | ST25R3916 |
| `rfidNTAGOriginalityCheck` | Verifica assinatura de originalidade | ST25R3916 |

### 3. DESFire & ISO-DEP (8 funções)
| Função | Descrição | Requisitos |
|--------|-----------|------------|
| `rfidDESFireCardInfo` | Lê informações do cartão DESFire | ST25R3916 |
| `rfidDESFireAppList` | Lista todas as aplicações | ST25R3916 |
| `rfidDESFireReadFile` | Lê arquivo de dados DESFire | ST25R3916 |
| `rfidDESFireWriteFile` | Escreve arquivo de dados DESFire | ST25R3916 |
| `rfidDESFireAuthenticate` | Autentica em aplicação DESFire | ST25R3916 |
| `rfidDESFireFormatCard` | Formata/reseta cartão DESFire | ST25R3916 |
| `rfidISODEPProbe` | Sonda dispositivo ISO-DEP (SELECT + GET VERSION) | ST25R3916 |
| `rfidISODEPNdefRead` | Lê dados NDEF de Type 4 Tag | ST25R3916 |

### 4. ISO15693 & FeliCa (6 funções)
| Função | Descrição | Requisitos |
|--------|-----------|------------|
| `rfidISO15693Inventory` | Inventário NFC-V — detecta todas as tags | ST25R3916 |
| `rfidISO15693ReadBlock` | Lê bloco ISO15693 | ST25R3916 |
| `rfidISO15693WriteBlock` | Escreve bloco ISO15693 | ST25R3916 |
| `rfidISO15693SystemInfo` | Informações do sistema ISO15693 | ST25R3916 |
| `rfidFeliCaRead` | Lê bloco FeliCa sem criptografia | Qualquer |
| `rfidFeliCaWrite` | Escreve bloco FeliCa | Qualquer |

### 5. Operações 125kHz (8 funções)
| Função | Descrição | Requisitos |
|--------|-----------|------------|
| `rfidEM4100Read` | Lê tag EM4100 via UART | RFID125 |
| `rfidEM4100Clone` | Clona EM4100 para T5577 | RFID125 + T5577 |
| `rfidT5577WriteConfig` | Escreve configuração T5577 | T5577 writer |
| `rfidT5577ReadConfig` | Lê configuração T5577 | T5577 reader |
| `rfidHIDProxRead` | Lê formato HID Prox | T5577 |
| `rfidIndalaRead` | Lê formato Indala | T5577 |
| `rfidEM4x02Write` | Escreve dados EM4x02 | EM4x02 writer |
| `rfid125kHzRawDisplay` | Exibe dados brutos 125kHz | RFID125 |

### 6. Operações Avançadas (8 funções)
| Função | Descrição |
|--------|-----------|
| `rfidMultiTagDetect` | Detecta múltiplas tags simultaneamente |
| `rfidTagMemoryMap` | Mapa visual de memória da tag |
| `rfidTagFormatDetect` | Auto-detecta formato da tag |
| `rfidTagDumpCompare` | Compara dois dumps lado a lado |
| `rfidTagDumpMerge` | Mescla dois dumps (preenche faltantes) |
| `rfidTagDumpAnalyze` | Analisa estrutura do dump |
| `rfidTagDumpExport` | Exporta em formato Flipper/Proxmark |
| `rfidTagDumpImport` | Importa de arquivo |

### 7. Testes de Segurança (6 funções)
| Função | Descrição |
|--------|-----------|
| `rfidReadRateTest` | Mede taxa de sucesso de leitura |
| `rfidAuthSuccessRate` | Testa taxa de autenticação por setor |
| `rfidTimingAnalysis` | Analisa tempos de resposta |
| `rfidStressTest` | Teste de estresse (15s) |
| `rfidFieldStrengthTest` | Testa força do campo RF |
| `rfidCompatibilityCheck` | Verifica compatibilidade tag/módulo |

## Estrutura de Configuração

### RFIDAdvConfig
```cpp
struct RFIDAdvConfig {
    int defaultBaudRate;      // 0=MIFARE, 1=FeliCa
    int keyAttackMaxTries;    // Max tentativas de chave por setor
    int readDelayMs;          // Delay entre leituras
    bool autoSave;            // Auto-salvar dumps no SD
    bool showProgress;        // Mostrar barras de progresso
    int emulTimeoutMs;        // Timeout de emulação
    int scanDurationSec;      // Duração do scan
};
```

## Métodos Públicos Adicionados ao ST25R3916

Para expor capacidades privadas do driver ST25R3916, foram adicionados 11 métodos públicos:

| Método | Descrição |
|--------|-----------|
| `mifareAuthBlock()` | Autentica bloco MIFARE Classic com chave |
| `mifareReadBlock()` | Lê bloco MIFARE Classic autenticado |
| `mifareWriteBlock()` | Escreve bloco MIFARE Classic autenticado |
| `mifareHalt()` | Envia comando HALT ao MIFARE Classic |
| `isoDepApdu()` | Envia/recebe APDU via ISO-DEP |
| `readDESFireInfoPublic()` | Lê informações DESFire |
| `readNtagSignaturePublic()` | Lê assinatura NTAG |
| `readNtagCountersPublic()` | Lê contadores NTAG |
| `buildEmuPagesPublic()` | Constrói páginas de emulação |
| `setupListenModePublic()` | Configura modo listen |
| `handleListenLoopPublic()` | Processa loop de escuta |

## Visuais
- Cards coloridos por categoria
- Header animado com título
- Footer com instruções
- Barras de progresso
- Spinners de carregamento
- Tela de boas-vindas com categorias

## Dependencias
- `RFIDInterface` - Interface abstrata
- `PN532` - Driver PN532
- `RFID2` - Driver MFRC522
- `ST25R3916` - Driver ST25R3916 (mais completo)
- `crypto1.h` - Cifra MIFARE Classic
- `apdu.h` - Constantes APDU/NDEF
- `tag_o_matic.h` - Operações básicas de tag

## Para Acessar
```
Menu → RFID → RFID Suite Avancada
```

## Notas de Segurança
- Todas as funções são para fins **educacionais e de pesquisa em segurança**
- Use apenas em ambientes controlados e autorizados
- Operações de clonagem/escrita podem ser ilegais em alguns países
- Funções avançadas requerem ST25R3916 para funcionalidade completa
