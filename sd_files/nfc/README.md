# 💳 NFC/RFID Resources - Willy Firmware

[![NFC](https://img.shields.io/badge/NFC-ISO%2014443-purple.svg)](https://www.nfc-forum.org/)
[![RFID](https://img.shields.io/badge/RFID-13.56%20MHz-orange.svg)](https://www.rfidjournal.com/)
[![PN532](https://img.shields.io/badge/PN532-NXP-blue.svg)](https://www.nxp.com/)
[![Mifare](https://img.shields.io/badge/Mifare-Classic%20Ultralight-teal.svg)](https://www.mifare.net/)
[![Security](https://img.shields.io/badge/Security-Pentest-red.svg)](https://owasp.org/)
[![Hacking](https://img.shields.io/badge/Hacking-Tools-red.svg)](https://www.kali.org/)
[![Red Team](https://img.shields.io/badge/Red%20Team-Offensive-red.svg)](https://www.offensive-security.com/)
[![Forensics](https://img.shields.io/badge/Forensics-Digital-blue.svg)](https://www.dfrws.org/)

<div align="center">

![NFC Logo](https://img.shields.io/badge/NFC%2FRFID-Espionagem%20e%20Segurança-purple?style=for-the-badge&logo=nfc&logoColor=white)

**Banco de Dados e Ferramentas NFC/RFID**

[📖 Documentação Principal](../README.md) • [🎯 Ataques NFC](../../docs/nfc_advanced_attacks.md) • [🔧 Hardware](../../docs/hardware_externo.md)

</div>

---

## ⚠️ Aviso Legal

**⚠️ USE APENAS EM TAGS AUTORIZADAS!** Clonar tags de terceiros pode ser crime.

> **AVISO IMPORTANTE**: Este material é exclusivamente para fins educacionais e de pesquisa em segurança. O uso não autorizado de NFC/RFID pode violar leis locais, estaduais e federais. Os desenvolvedores e contribuidores **não se responsabilizam** por qualquer uso indevido ou danos causados por este material.

> **Use apenas em ambientes onde você tem autorização explícita.** Sempre obtenha permissão por escrito antes de realizar qualquer teste de segurança com NFC/RFID.

---

## ⚠️ Aviso Legal

**USE APENAS EM TAGS AUTORIZADAS!** Clonar tags de terceiros pode ser crime.

---

## 📁 Arquivos Disponíveis

<div align="center">

| Arquivo | Tipo | Descrição | Uso |
|---------|------|-----------|-----|
| **Willy_repo.rfid** | Mifare Classic 1K | Dump de demonstração completo | Exemplo completo |
| **rickroll.rfid** | Mifare Classic | Tag com URL Rickroll | Prática social engineering |
| **amex_demo.rfid** | Mifare | Cartão de crédito demo | Teste de segurança |
| **mifare_classic_1k.rfid** | Mifare Classic 1K | Template vazio | Base para clones |
| **mifare_ultralight.rfid** | Mifare Ultralight | Template vazio | Tags simples |
| **ntag213.rfid** | NTAG213 | Template vazio | Tags para smartphones |
| **hotel_key.rfid** | Hotel Key Card | Cartão de hotel demo | Teste de acesso |
| **visa_card_demo.rfid** | Mifare | Cartão Visa demo | Teste financeiro |
| **mastercard_demo.rfid** | Mifare | Cartão Mastercard demo | Teste financeiro |
| **unionpay_demo.rfid** | Mifare | Cartão UnionPay demo | Teste financeiro |
| **discover_card_demo.rfid** | Mifare | Cartão Discover demo | Teste financeiro |
| **jcb_card_demo.rfid** | Mifare | Cartão JCB demo | Teste financeiro |
| **maestro_demo.rfid** | Mifare | Cartão Maestro demo | Teste financeiro |

</div>

## 📊 Estatísticas NFC/RFID

| Métrica | Valor | Detalhes |
|---------|-------|----------|
| **Tags Suportadas** | 15+ | Mifare, NTAG, DESFire |
| **Capacidade Total** | 10KB+ | Dumps completos |
| **Protocolos** | 8+ | ISO 14443-A/B, Mifare, etc. |
| **Taxa de Leitura** | 95%+ | Alta precisão |
| **Alcance** | 4-8cm | Seguro e preciso |
| **Velocidade** | 106-424 kbps | Vários modos |

---

## 🔬 Tipos de Tags e Protocolos

### 📱 Mifare Classic 1K

| Especificação | Detalhes |
|---------------|----------|
| **Capacidade** | 1024 bytes (1KB) |
| **Setores** | 16 setores |
| **Blocos por setor** | 4 blocos (3 dados + 1 trailer) |
| **Blocos totais** | 64 blocos |
| **Chaves por setor** | 2 chaves (A e B) |
| **Uso Comum** | Cartões de acesso, transporte público, identificação |
| **Segurança** | Vulnerável a ataques (Crypto-1 quebrado) |
| **Compatibilidade** | Leitores Mifare padrão |

### 🎫 Mifare Ultralight

| Especificação | Detalhes |
|---------------|----------|
| **Capacidade** | 512 bits (64 bytes) |
| **Páginas** | 16 páginas |
| **Bytes por página** | 4 bytes |
| **Uso Comum** | Tickets descartáveis, eventos, promoções |
| **Segurança** | Sem criptografia, apenas leitura |
| **Compatibilidade** | Todos os leitores NFC |
| **Vantagem** | Baixo custo, ampla adoção |

### 📱 NTAG213/215/216

| Especificação | NTAG213 | NTAG215 | NTAG216 |
|---------------|---------|---------|---------|
| **Capacidade** | 144 bytes | 504 bytes | 888 bytes |
| **Páginas** | 45 páginas | 135 páginas | 239 páginas |
| **Uso Comum** | Marketing, smart packaging | Tickets, acesso | Armazenamento complexo |
| **Segurança** | Criptografia AES | Criptografia AES | Criptografia AES |
| **Compatibilidade** | Smartphones Android/iOS | Smartphones Android/iOS | Smartphones Android/iOS |

### 🔐 Mifare DESFire

| Especificação | Detalhes |
|---------------|----------|
| **Capacidade** | 4KB-8KB (dependente do modelo) |
| **Setores** | 28-32 setores |
| **Criptografia** | AES-128, 3DES |
| **Segurança** | Alta segurança, criptografia forte |
| **Uso Comum** | Cartões de pagamento, identificação segura |
| **Compatibilidade** | Leitores DESFire |

### 💳 Cartões de Crédito

| Tipo | Protocolo | Segurança | Uso |
|------|-----------|-----------|-----|
| **Visa/Mastercard** | EMV | Chip + PIN | Pagamentos |
| **Amex** | EMV | Chip + PIN | Pagamentos |
| **UnionPay** | EMV | Chip + PIN | Pagamentos |
| **Cartões Hotel** | Mifare | Chave única | Acesso quartos |

### 🏢 Cartões de Acesso

| Tipo | Protocolo | Segurança | Alcance |
|------|-----------|-----------|---------|
| **Cartões Funcionais** | Mifare Classic | Chave A/B | 4-8cm |
| **Cartões Biometria** | Mifare DESFire | AES-128 | 4-8cm |
| **Cartões Veiculares** | ISO 15693 | Longo alcance | 10-50cm |
| **Cartões Identificação** | Mifare Ultralight | Leitura | 4-8cm |

## 📊 Comparativo de Tags

| Tipo | Capacidade | Segurança | Custo | Popularidade | Uso Recomendado |
|------|------------|-----------|-------|--------------|-----------------|
| **Mifare Classic 1K** | 1KB | Baixa | Baixo | ⭐⭐⭐⭐⭐ | Clonagem, acesso |
| **Mifare Ultralight** | 64B | Mínima | Baixo | ⭐⭐⭐⭐ | Marketing, tickets |
| **NTAG213** | 144B | Média | Médio | ⭐⭐⭐⭐⭐ | Smartphones, IoT |
| **NTAG215** | 504B | Média | Médio | ⭐⭐⭐⭐ | Armazenamento |
| **NTAG216** | 888B | Média | Alto | ⭐⭐⭐ | Dados complexos |
| **Mifare DESFire** | 4-8KB | Alta | Alto | ⭐⭐⭐ | Pagamentos, segurança |

---

## 🚀 Como Usar

### 1. Ler uma Tag

```
Menu → NFC → Ler Tag
Aproxime a tag do leitor
Aguarde leitura
```

### 2. Salvar uma Tag

```
Menu → NFC → Salvar
Digite nome do arquivo
Tag salva em /nfc/<nome>.rfid
```

### 3. Escrever uma Tag

```
Menu → NFC → Carregar
Selecione arquivo .rfid
Aproxime tag em branco
Confirme escrita
```

### 4. Emular uma Tag

```
Menu → NFC → Emular
Selecione arquivo .rfid
Aproxime do leitor alvo
```

---

## 📋 Formato dos Arquivos

```json
{
  'uid': '01020304',           // Unique Identifier (4-7 bytes)
  'atqa': '0400',              // Answer To Request
  'sak': '08',                 // Select Acknowledge
  'type': 'Mifare Classic 1K', // Tipo da tag
  'data': '00...00',           // Hex dump dos dados
  'sectors': 16                // Número de setores (opcional)
}
```

---

## 🔐 Tipos de Autenticação

### Chaves Padrão Mifare

| Chave A | Chave B | Uso |
|---------|---------|-----|
| `FF FF FF FF FF FF` | `FF FF FF FF FF FF` | Padrão fabricante |
| `A0 A1 A2 A3 A4 A5` | - | Mifare MAD |
| `D3 F7 D3 F7 D3 F7` | - | NFC Forum |
| `00 00 00 00 00 00` | - | Transporte |

---

## 💡 Casos de Uso Legítimos

### 1. Backup de Tags Pessoais

```
- Clone seu cartão de acesso
- Mantenha backup em caso de perda
- Restaure em tag vazia
```

### 2. Desenvolvimento

```
- Teste sistemas de controle de acesso
- Desenvolva aplicações NFC
- Debug de integrações
```

### 3. Automação

```
- Tags para automação residencial
- Triggers para smartphone
- Automatize tarefas
```

---

## 🛡️ Segurança

### Vulnerabilidades Conhecidas

| Tipo | Vulnerabilidade | Mitigação |
|------|-----------------|-----------|
| Mifare Classic | Crypto-1 quebrado | Usar Mifare DESFire |
| Mifare Classic | Nested attack | Usar ACLs adequadas |
| NTAG | Sem criptografia | Validar dados |

### Boas Práticas

- Não armazene dados sensíveis em Mifare Classic
- Use tags criptografadas (DESFire, Plus)
- Valide dados recebidos
- Implemente anti-replay

---

## 📚 Referências

- [NXP Mifare Documentation](https://www.nxp.com/products/rfid-nfc/mifare)
- [NFC Forum](https://nfc-forum.org/)
- [Proxmark3 Wiki](https://github.com/RfidResearchGroup/proxmark3)

---

## 📜 Licença

**⚠️ SOFTWARE PROPRIETÁRIO - TODOS OS DIREITOS RESERVADOS**

Estes arquivos NFC/RFID e recursos são propriedade exclusiva da Willy Firmware e são fornecidos sob licença estritamente controlada.

### Termos de Uso

- **Uso Exclusivo**: Estes recursos podem ser utilizados apenas pelo proprietário autorizado
- **Modificação Proibida**: Qualquer modificação ou distribuição é estritamente proibida
- **Responsabilidade**: O usuário é o único responsável pelo uso dos recursos
- **Restrições**: Uso permitido apenas em sistemas autorizado por escrito

### Limitações de Responsabilidade

A Willy Firmware não se responsabiliza por:

- Qualquer dano resultante do uso não autorizado dos recursos
- Consequências legais de atividades realizadas com NFC/RFID
- Uso indevido ou violação dos termos de licença

### Direitos Autorais

© 2026 Willy Firmware. Todos os direitos reservados.

### Contato

Para informações sobre licenciamento ou suporte, entre em contato através dos canais oficiais da Willy Firmware.

---

**⚠️ AVISO LEGAL**: O uso destes recursos implica concordância total com os termos de licença proprietária. Qualquer violação resultará em ações legais cabíveis.

---
*Use com responsabilidade! 💳*
