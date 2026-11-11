# Guia de Solução de Problemas (Troubleshooting)

Este guia ajuda a resolver os problemas mais comuns encontrados ao montar ou operar o dispositivo Willy.

## 🔧 Problemas de Hardware

### 1. Tela Branca (White Screen)

- **Causa**: Falha na comunicação com o controlador do display ou fiação incorreta.
- **Solução**: Verifique se o pino `TFT_CS` está bem conectado. No CYD, certifique-se de que está usando o ambiente correto no PlatformIO (`CYD-2USB`).

### 2. Touch Screen Não Responde

- **Causa**: Cabo flat do touch mal conectado ou calibração corrompida.
- **Solução**: Tente recalibrar no menu **Configurações -> Tela -> Calibrar Touch**. Se o problema persistir, verifique a continuidade dos pinos `T_CS`, `T_CLK` e `T_DIN`.

### 3. Cartão SD Não Monta ("SD Card Fail")

- **Causa**: Cartão mal inserido, formatado incorretamente ou alta velocidade de clock.
- **Solução**: Formate o cartão em **FAT32** (limite de 32GB para melhor compatibilidade). Verifique se o pino 5 (CS) está livre para uso do SD.

---

## 📡 Problemas de Módulos

### 4. GPS Sem Sinal (No Fix)

- **Causa**: Falta de visão do céu ou interferência de componentes eletrônicos.
- **Solução**: Vá para um local aberto. O primeiro "lock" pode levar até 5 minutos. Verifique se a chave de energia do GPS está ligada.

### 5. RF (CC1101) Não Transmite/Recebe

- **Causa**: Módulo mal energizado (requer 3.3V estável) ou pino GDO0 incorreto.
- **Solução**: Verifique se o interruptor do CC1101 está ligado e o do NFC desligado. Confira se o pino `GDO0` está mapeado para o GPIO 27.

---

## 💻 Problemas de Software/Compilação

### 6. Erro de Compilação no PlatformIO

- **Causa**: Falta de bibliotecas ou versão do framework incompatível.
- **Solução**: Execute `pio lib install` e certifique-se de que o hardware selecionado no arquivo `platformio.ini` corresponde ao seu dispositivo.

### 7. Interface Web Inacessível

- **Causa**: Willy não está no modo Access Point ou IP incorreto.
- **Solução**: Verifique se a rede WiFi "Willy-AP" aparece no seu celular. Acesse `http://192.168.4.1` no navegador.

### 8. Alto Consumo de CPU/Memória

- **Causa**: Múltiplos módulos ativos simultaneamente ou benchmark em execução.
- **Solução**: Use `benchmark status` para verificar uso. Desative módulos não utilizados. Reinicie o dispositivo se necessário.

### 9. Problemas no Barramento SPI

- **Causa**: Conflito entre dispositivos SPI ou velocidade muito alta.
- **Solução**: Verifique se apenas um dispositivo SPI tem CS ativo por vez. Use `hardware test` para diagnosticar. Ajuste frequência SPI se necessário.

### 10. Módulos Não Detectados Automaticamente

- **Causa**: Detecção de hardware falhou ou pinagem incorreta.
- **Solução**: Execute `hardware detect` para forçar detecção. Verifique conexões físicas e perfil de hardware ativo.

### 11. Plugins Não Carregam

- **Causa**: Arquivo JSON malformado ou dependências faltando.
- **Solução**: Valide JSON do plugin. Verifique logs em `/WILLY_LOGS/plugins.log`. Use `plugin status` para diagnóstico.

### 12. Configurações Dinâmicas Não Persistem

- **Causa**: Falha ao salvar no SD card ou permissões incorretas.
- **Solução**: Execute `config save` manualmente. Verifique espaço no SD card. Formate SD se corrompido.

---

---

## ❓ Perguntas Frequentes (FAQ)

### 📊 Performance e Otimizações

**P: O dispositivo está lento após as otimizações?**
R: Não, as otimizações melhoraram a performance. Se notar lentidão, execute `benchmark run` para diagnosticar gargalos.

**P: Como verificar se as otimizações estão ativas?**
R: Use `config list` para ver configurações ativas. As otimizações são aplicadas automaticamente na compilação.

**P: O consumo de energia aumentou?**
R: Não, as otimizações incluem modos sleep automáticos. Total pico máximo: 942mA (dentro dos limites).

### 🔧 Hardware e Configuração

**P: Posso usar pinos diferentes dos otimizados?**
R: Sim, mas as configurações otimizadas oferecem melhor performance. Use `hardware set` para alterar perfis.

**P: Como adicionar novos módulos?**
R: Implemente a interface `IModule` e registre no `SystemManager`. Consulte `docs/architecture_diagrams.md`.

**P: O SPI compartilhado é confiável?**
R: Sim, testado extensivamente. Usa chip select dedicado para cada dispositivo, evitando conflitos.

### 💻 Desenvolvimento e Plugins

**P: Como criar um plugin personalizado?**
R: Use o template em `sd_files/plugins/example_plugin.json`. Implemente callbacks necessários e coloque na pasta plugins.

**P: Posso modificar o firmware sem perder otimizações?**
R: Sim, mas mantenha as configurações de build em `platformio.ini`. As otimizações são aplicadas automaticamente.

**P: Como contribuir com melhorias?**
R: Abra uma issue no GitHub descrevendo a proposta. Siga as melhores práticas documentadas.

### 🔒 Segurança e Uso

**P: As otimizações afetam a segurança?**
R: Não, melhoram-na com validações adicionais e leak detection. Todas as funcionalidades de segurança permanecem intactas.

**P: Posso reverter para versão anterior?**
R: Sim, checkout de commit anterior no Git. Mas as otimizações são recomendadas para melhor estabilidade.

**P: Como reportar bugs relacionados às otimizações?**
R: Use `benchmark save` para gerar relatório, anexe logs de `/WILLY_LOGS/` e abra issue detalhada.

---

## 🆘 Suporte Adicional

Se o seu problema não estiver listado aqui:

1. Verifique os logs no Serial Monitor (Baudrate 115200).
2. Consulte os arquivos de log no SD na pasta `/WILLY_LOGS/`.
3. Execute `system info` para diagnóstico completo.
4. Abra uma issue no repositório oficial do GitHub com relatório de benchmark.
