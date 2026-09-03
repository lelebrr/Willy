# IMPLEMENTACOES.md – Roadmap e Planejamento de Novas Funcionalidades

Este arquivo é o **documento vivo** de todas as ideias, melhorias e novas features planejadas para o Willy.

Objetivo: ter um lugar centralizado para registrar **o que queremos implementar**, **como pretendemos fazer**, **quais hardwares/serviços são necessários**, **dificuldade estimada**, **prioridade** e **status atual**.

## Regras para este documento
- Use tabelas para listar as features sempre que possível
- Mantenha atualizado com status: 🟢 Concluído | 🟡 Em progresso | 🔵 Planejado | ⚪ Ideia inicial
- Coloque data aproximada ou versão alvo quando souber (ex: v0.9.5, Q3/2025)
- Se for algo que depende de pull request ou issue, linke aqui
- Seja o mais técnico possível: mencione bibliotecas, pinos, conflitos de hardware, consumo de memória, etc.

## Lista de Implementações Planejadas

| # | Feature / Melhoria                              | Descrição breve                                      | Hardware necessário              | Dificuldade | Prioridade | Status | Versão alvo | Notas / Dependências |
|---|-------------------------------------------------|------------------------------------------------------|----------------------------------|-------------|------------|--------|-------------|----------------------|
| 3 | Integração Voice AI / LLM Offline Completo | Voice AI com Xiaozhi + Assets Generator no Micro SD | Speaker I²S recomendado, SD já existente | Muito Alta | Crítica | 🔵 Planejado | v1.0.0 | Usa https://github.com/78/xiaozhi-esp32 + https://github.com/78/xiaozhi-assets-generator. Wake word custom, ASR/TTS/LLM offline, controle por voz mantendo pentest 100% funcional. |

| 1 |                                                 |                                                      |                                  |             |            |        |             |                      |
| 2 |                                                 |                                                      |                                  |             |            |        |             |                      |

## Integração Voice AI / LLM Offline Completo (Xiaozhi ESP32 + Assets Generator)

### Descrição e Objetivo
Transformar o Willy em um assistente de voz inteligente completo mantendo 100% das funções atuais de pentest. Utilizar a stack Xiaozhi para wake word detection, ASR streaming, TTS synthesis e LLM via MCP (Model Context Protocol), tudo controlado por voz. A combinação dos repositórios xiaozhi-esp32 (runtime) e xiaozhi-assets-generator (web tool) permite criar um assistente personalizado com wake-word custom, fontes, emojis, backgrounds e prompts específicos para pentest, tudo rodando offline no ESP32-S3.

### Estratégia de Armazenamento no Micro SD (obrigatório incluir isso)
Todos os assets pesados serão armazenados no cartão Micro SD (já suportado no Willy). O gerador https://github.com/78/xiaozhi-assets-generator criará um único arquivo `assets.bin` (SPIFFS-compatible). No boot do Willy, uma nova task FreeRTOS monta o SD e copia/carrega o assets.bin para PSRAM ou lê diretamente via LVGL filesystem driver. Modelos grandes (wake-word, MultiNet, fontes, emojis, backgrounds) ficam na raiz do SD em /assets/assets.bin e /assets/models/. ASR/TTS/LLM prompts e arquivos .bin grandes também vão para o SD (carregados on-demand para PSRAM só quando o Voice Mode for ativado). Vantagens: zero consumo de flash interno, fácil atualização (basta trocar o arquivo no SD), suporte a múltiplos temas/idiomas, wake-word custom sem recompilar. Como implementar: usar lv_fs_sd para ler assets.bin, criar uma função willy_load_xiaozhi_assets() que verifica SD primeiro.

### O que vai mudar no projeto atual
- Alterações no menu LVGL/Cyber Menu (novo item "Voice AI Mode")
- Novas tasks FreeRTOS (voice task, asset loader)
- Mudanças no platformio.ini, sdkconfig e partition table
- Impacto nas funções atuais (WiFi/BLE/Sub-GHz/etc.) — tudo continua funcionando normalmente
- Como evitar quebras: modo separado + fallback, voice mode não interfere em funções existentes

### Requisitos de Hardware
- Componentes extras: Speaker I²S recomendado (ex: MAX98357A com amplificador), câmera OV2640 opcional para visão computacional
- Pinos I2S (já usado pelo INMP441 — multiplexar com software switch), SPI (SD já usa pino 5, 18, 23, 19)
- Consumo adicional estimado: 150-200mA durante ASR+TTS, 100mA em standby voice

### Requisitos de Software e Bibliotecas
- Versão mínima do ESP-IDF 4.4+ com suporte a ESP32-S3
- Uso obrigatório do gerador online https://github.com/78/xiaozhi-assets-generator (browser-side, sem servidor)
- Modelos recomendados: WakeNet wn9 (lightweight), MultiNet mn7_en/mn7_cn (ASR), Whisper tiny (fallback)
- Compatibilidade com PlatformIO + LVGL 8.3 + FreeRTOS
- Bibliotecas ESP-IDF: esp_audio, esp_llm, esp_nn

### Estimativa de Tamanho e Esforço
- Flash adicional estimado: ~500KB (somente runtime, modelos no SD)
- PSRAM adicional: 2-4MB para assets.bin + buffers ASR/LLM
- RAM em runtime durante ASR + LLM + TTS simultâneo: ~6-8MB
- Tempo estimado de desenvolvimento: 8-12 semanas (meio período)
- Complexidade geral: Muito Alta (integração profunda com sistema existente)
- Linhas de código aproximadas: 3000-5000

### Passos de Implementação (em ordem cronológica numerada)
1. Configurar ambiente de desenvolvimento com ESP-IDF 4.4+ e clonar xiaozhi-esp32
2. Integrar xiaozhi-esp32 no projeto Willy via PlatformIO (adicionar dependências, modificar platformio.ini)
3. Modificar partition table para suportar PSRAM usage e assets externos
4. Implementar driver LVGL filesystem para Micro SD (lv_fs_sd integration)
5. Criar função willy_load_xiaozhi_assets() que carrega assets.bin do SD para PSRAM
6. Integrar xiaozhi-assets-generator: criar script Python para gerar assets.bin personalizados
7. Modificar Cyber Menu para adicionar "Voice AI Mode" com ícone adequado
8. Implementar task FreeRTOS para voice processing (prioridade média, baixa latência)
9. Configurar I2S multiplexing para microfone + speaker (software switching)
10. Implementar wake word detection com WakeNet custom (gerado via assets-generator)
11. Criar sistema de comandos por voz ("Ei Willy, faz deauth na rede X", "scan BLE", etc.)
12. Integrar LLM com prompts específicos para pentest (Willy-specific prompts)
13. Implementar TTS com voz personalizada e suporte a múltiplos idiomas
14. Adicionar sistema de fallback (voz + tela + joystick) para melhor UX
15. Criar interface LVGL para visualização de comandos de voz e respostas do LLM
16. Implementar sistema de atualização de assets via SD (swap sem reboot)
17. Adicionar logging de comandos de voz para análise forense
18. Otimizar consumo de memória com streaming de modelos do SD
19. Implementar sistema de wake word switching (várias wake words custom)
20. Testes completos: compatibilidade com todas as funções existentes, desempenho, estabilidade

### Riscos e Desafios Principais
- Conflitos de I2S/SPI com SD e microfone: necessidade de multiplexing software cuidadoso
- Latência de carregamento do SD: modelos grandes podem causar delays iniciais
- Memória (2 MB PSRAM limita LLM completo): estratégia híbrida (pequenos LLM + prompts específicos)
- Aquecimento quando tudo roda junto: necessidade de thermal throttling management
- Complexidade de debug: sistema distribuído com múltiplos componentes

### Abordagem Técnica Recomendada
- Modo "Voice AI Mode" separado no menu: ativado/desativado sem reiniciar sistema
- Arquitetura de tarefas: main task (LVGL), voice task (ASR/LLM), asset loader task (SD), network task (WiFi/BLE)
- Como executar comandos do Willy por voz: parser de comandos com regex + funções existentes
- Estratégia de fallback: se reconhecimento falhar, mostrar opções na tela + joystick
- Como o usuário gera e coloca o assets.bin no SD:
  1. Acessar https://github.com/78/xiaozhi-assets-generator
  2. Fazer upload de wake word, selecionar fontes/emojis/backgrounds
  3. Gerar assets.bin e baixar
  4. Copiar para cartão SD na pasta /assets/
  5. Reiniciar Willy e ativar Voice AI Mode

### Status e Próximos Passos
- Status: 🔵 Planejado
- Versão alvo: v1.0.0
- Dependências de outras features: Suporte a PSRAM otimizado, sistema de arquivos LVGL para SD
- Pull requests sugeridos: #78 (xiaozhi-esp32 integration), #79 (assets generator tool), #80 (voice menu system)

## Ideias em brainstorm (sem prioridade definida ainda)

-
-
-

## Como contribuir com ideias novas
1. Abra uma issue com label "enhancement" e "roadmap"
2. Ou edite diretamente este arquivo via pull request
3. Use o template abaixo para propor algo novo:

**Título da feature:**
**Descrição:**
**Hardware extra (se precisar):**
**Bibliotecas sugeridas:**
**Conflitos conhecidos:**
**Estimativa de RAM/Flash extra:**
**Prioridade sugerida:** (Alta/Média/Baixa)

Última atualização: 09/03/2026
