# Willy Firmware

## *O Kit Definitivo de Segurança para ESP32* — **Projeto Unificado**

Este repositório é o resultado da unificação de múltiplos variantes do projeto Willy:
- `Willy_ESP32_S3` (base principal com arquitetura MVC)
- `Willy_work` (suporte multi-placa, BruceConfig, cyber menu)
- `Cyd_Lele` (variante CYD/Lele)
- `Esp32-S3` (variante ESP32-S3)
- `Willy_ESP_s3` (variante Bruce com ILI9341 custom)

Todos os fontes, scripts e dependências foram consolidados nesta única pasta.

[![ESP32-S3](https://img.shields.io/badge/ESP32--S3--DevKitC--1-red.svg)](https://www.espressif.com/products/socs/esp32)
[![Arduino](https://img.shields.io/badge/Arduino-C%2B%2B-blue.svg)](https://www.arduino.cc/)
[![PlatformIO](https://img.shields.io/badge/PlatformIO-Compat%C3%ADvel-green.svg)](https://platformio.org/)
[![C++](https://img.shields.io/badge/C%2B%2B-17-yellow.svg)](https://isocpp.org/)

Transforme seu ESP32-S3 em uma poderosa ferramenta de pesquisa em segurança inspirada em projetos como Flipper Zero e Pwnagotchi.

## 📁 Estrutura do Projeto

```
src/          Código fonte principal (core, modules, ui, web_files)
include/      Headers compartilhados
lib/          Bibliotecas de terceiros
scripts/      Scripts de automação de build
boards/       Definições de boards customizadas
docs/         Documentação
pcbs/         Projetos de PCB
media/        Recursos de mídia
embedded_resources/ Recursos embarcados
sd_files/     Arquivos para cartão SD
```

## 🚀 Compilação

```bash
platformio run
platformio run --upload
```

## ⚠️ Aviso Legal

Este firmware é fornecido exclusivamente para fins educacionais e de pesquisa em segurança.