# Guia de Configurações - Firmware Willy

Este documento descreve as principais configurações disponíveis no menu **Configurações** do firmware Willy e como elas afetam o comportamento do seu dispositivo.

## 🖥️ Configurações de Interface

### Brilho (Brightness)

- **Ajuste**: 0% a 100%.
- **Impacto**: O brilho alto melhora a visibilidade sob o sol, mas aumenta significativamente o consumo de bateria e o calor gerado pela tela.

### Tema (Theme)

- **Opções**: Escuro (Dark), Claro (Light), Customizado (via `theme.css`).
- **Personalização**: O tema customizado pode ser editado na interface web ou diretamente no SD card.

### Orientação da Tela

- **Rotação**: 0°, 90°, 180°, 270°.
- **Uso**: Útil para adaptar o dispositivo ao seu case ou preferência de mão (destro/canhoto).

---

## 🌐 Configurações de Rede

### Modo WiFi

- **STATION (STA)**: Conecta o Willy a uma rede WiFi existente.
- **ACCESS POINT (AP)**: Cria uma rede WiFi própria do Willy para acesso via interface web.

### MAC Spoofing

- **Função**: Permite alterar o endereço MAC do rádio WiFi para evitar rastreamento ou contornar filtros de rede.
- **Reset**: O MAC original de fábrica pode ser restaurado a qualquer momento.

---

## 📡 Configurações de Hardware

### Pinos I/O (GPIOs)

- **IR TX/RX**: Define os pinos usados para o transmissor e receptor infravermelho.
- **RF Config**: Ajusta os pinos SPI e GDO0 para o módulo CC1101.

### Salvamento de Dados

- **Destino**: Escolha entre Salvar no Cartão SD ou na Flash Interna (LittleFS).
- **Aviso**: Arquivos grandes (como capturas PCAP) devem ser salvos preferencialmente no SD.

---

## 🔒 Segurança e Acesso

### Credenciais da Web UI

- **Usuário/Senha**: Altera o login de acesso à interface web.
- **Dica**: Recomenda-se alterar a senha padrão no primeiro uso.

### Modo Stealth

- **Função**: Desativa LEDs de status e sons para operações discretas.

---

## 🔄 Gerenciamento do Sistema

### Reiniciar (Reboot)

- Realiza um soft-reset seguro no dispositivo.

### Restaurar Padrões (Factory Reset)

- **Aviso**: Apaga todas as configurações personalizadas e retorna as configurações de fábrica. Não apaga arquivos do SD card.

---

## ⚙️ Configurações Dinâmicas (CLI)

### Comandos de Configuração

O firmware suporta configuração dinâmica via comandos seriais. Conecte-se via USB e use os seguintes comandos:

#### Configuração de Hardware
```bash
# Ver perfil de hardware atual
hardware profile

# Alterar perfil de hardware
hardware set CYD-2USB

# Ver status de pinos
hardware pins

# Testar conectividade de módulos
hardware test wifi
hardware test rf
hardware test rfid
```

#### Configuração de Performance
```bash
# Executar benchmark completo
benchmark run

# Ver métricas em tempo real
benchmark status

# Resetar contadores de benchmark
benchmark reset

# Salvar relatório de performance
benchmark save
```

#### Configuração de Plugins
```bash
# Listar plugins disponíveis
plugin list

# Carregar plugin
plugin load example_plugin

# Descarregar plugin
plugin unload example_plugin

# Ver status de plugins
plugin status
```

#### Configuração Dinâmica
```bash
# Ver todas as configurações
config list

# Alterar configuração
config set wifi.scan_timeout 30
config set rf.frequency 433920000

# Salvar configurações
config save

# Carregar configurações
config load
```

### Interface Web de Configuração

Acesse `http://willy.local` ou o IP do dispositivo em modo AP para configurar via interface web:

- **Dashboard em Tempo Real**: CPU, memória, status de módulos
- **Configurações Avançadas**: Ajustes finos de performance
- **Monitor de Logs**: Visualização de logs em tempo real
- **Gerenciamento de Plugins**: Upload e controle de plugins

---

## 📊 Monitoramento de Performance

### Métricas em Tempo Real

O sistema monitora automaticamente:

- **CPU Usage**: 15-25% em operações normais
- **Memória**: 45-65% de uso otimizado
- **Latência**: <50ms para operações críticas
- **Throughput**: 95%+ de sucesso em transmissões

### Alertas de Performance

- 🔴 **CPU > 80%**: Alto uso, possível bottleneck
- 🟡 **Memória > 80%**: Risco de fragmentação
- 🔴 **Latência > 100ms**: Performance degradada
- 🟡 **Erros > 5%**: Problemas de conectividade

### Otimização Automática

O sistema ajusta automaticamente:
- Frequência SPI baseada na carga
- Modo sleep de módulos inativos
- Priorização de tarefas críticas
- Balanceamento de energia
