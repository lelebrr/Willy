# Estrutura do Cartão SD - Willy

Para que o firmware Willy funcione corretamente, o Cartão SD deve estar organizado conforme a estrutura abaixo.

## 📁 Diretórios Principais

### `/BadUSB`

- **Função**: Armazena scripts BadUSB (.txt).
- **Uso**: Scripts no formato Ducky Script que podem ser executados no alvo.

### `/IR`

- **Função**: Biblioteca de códigos Infravermelho.
- **Subpastas**: `/TV`, `/AC`, `/Audio`.
- **Formato**: Arquivos `.ir` compatíveis com Flipper Zero.

### `/SubGHz`

- **Função**: Capturas e payloads de RF.
- **Uso**: Armazena sinais gravados via CC1101.

### `/NFC`

- **Função**: Dumps e chaves de cartões NFC/RFID.
- **Uso**: Armazena arquivos `.nfc` e dicionários de chaves.

### `/WillyGPS`

- **Função**: Logs de rastreamento GPS.
- **Formato**: Arquivos `.gpx` para visualização em mapas.

### `/WillyWardriving`

- **Função**: Capturas de redes WiFi (Wardriving).
- **Formato**: CSV compatível com Wigle.net.

### `/WILLY_LOGS`

- **Função**: Logs forenses do sistema.
- **Uso**: Registra erros, atividades e depuração.

---

## 📄 Arquivos de Configuração

- `/willy_config.json`: Configurações gerais do sistema.
- `/theme.css`: Folha de estilo customizada para a Interface Web.
- `/web_credentials.txt`: (Opcional) Cache de credenciais de acesso.

---

## 💡 Dicas Importantes

1. **Formatação**: Utilize **FAT32**. Cartões exFAT podem não ser reconhecidos por algumas placas CYD mais antigas.
2. **Capacidade**: Cartões de 4GB a 32GB são os mais estáveis. Capacidades maiores podem levar mais tempo para montar.
3. **Organização**: Mantenha os nomes dos diretórios exatamente como listados acima (case-sensitive) para garantir a compatibilidade com o firmware.
