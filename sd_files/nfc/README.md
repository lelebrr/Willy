# 💳 NFC/RFID Resources

Arquivos de exemplo e dumps para o módulo NFC do Bruce.

---

## ⚠️ Aviso Legal

**USE APENAS EM TAGS AUTORIZADAS!** Clonar tags de terceiros pode ser crime.

---

## 📁 Arquivos Disponíveis

| Arquivo | Tipo | Descrição |
|---------|------|-----------|
| `bruce_repo.rfid` | Mifare Classic 1K | Dump de demonstração |
| `rickroll.rfid` | Mifare Classic | Tag de demonstração |
| `amex_demo.rfid` | Mifare | Demo de cartão |
| `mifare_classic_1k.rfid` | Mifare Classic 1K | Template vazio |
| `mifare_ultralight.rfid` | Mifare Ultralight | Template vazio |
| `ntag213.rfid` | NTAG213 | Template vazio |
| `hotel_key.rfid` | Hotel Key Card | Demo de cartão de hotel |

---

## 📖 O Que é Cada Tipo

### Mifare Classic 1K
- **Capacidade**: 1024 bytes
- **Setores**: 16
- **Blocos por setor**: 4
- **Uso**: Cartões de acesso, transporte público

### Mifare Ultralight
- **Capacidade**: 512 bits (64 bytes)
- **Páginas**: 16
- **Uso**: Tickets descartáveis, eventos

### NTAG213
- **Capacidade**: 144 bytes
- **Páginas**: 45
- **Uso**: Tags NFC para smartphones, marketing

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

*Use com responsabilidade! 💳*