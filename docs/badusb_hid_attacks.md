# BadUSB & HID Attacks - Documentação Completa

## ⚠️ AVISO LEGAL

**Este código é fornecido EXCLUSIVAMENTE para fins educacionais e de pesquisa em segurança.**

O uso não autorizado destas ferramentas pode:
- Violar leis federais e estaduais
- Constituir crime de invasão de dispositivo informático
- Resultar em penalidades civis e criminais

**Use apenas em:**
- Dispositivos próprios e ambientes de laboratório
- Testes de penetração com autorização por escrito
- Pesquisa acadêmica em ambiente controlado
- Treinamento de segurança com consentimento

---

## Índice

### Ataques HID Básicos (Já Existentes)
1. [BadUSB (Ducky Script)](#1-badusb-ducky-script)
2. [USB Keyboard](#2-usb-keyboard)
3. [USB Clicker](#3-usb-clicker)

### Ataques USB Avançados (Novos)
4. [Touch Macro Recorder](#4-touch-macro-recorder)
5. [Stealth Mouse Jiggler](#5-stealth-mouse-jiggler)
6. [Keyboard + Mouse Combo Worm](#6-keyboard--mouse-combo-worm)
7. [Conditional Delayed Payload](#7-conditional-delayed-payload)
8. [HID Credential Harvester](#8-hid-credential-harvester)
9. [LED Blink Exfil Channel](#9-led-blink-exfil-channel)
10. [Multi-OS Payload Switcher](#10-multi-os-payload-switcher)
11. [HID Timing Side-Channel](#11-hid-timing-side-channel)
12. [Ransomware Simulation Demo](#12-ransomware-simulation-demo)
13. [Persistent HID Backdoor](#13-persistent-hid-backdoor)

---

# ATAQUES HID BÁSICOS

---

## 1. BadUSB (Ducky Script)

### O que faz
Executa scripts no formato Ducky Script via USB, simulando um teclado real digitando comandos em alta velocidade.

### Como usar
1. Coloque seus arquivos `.txt` na pasta `/BadUSB` do cartão SD.
2. Navegue até **Outros -> BadUSB & HID -> BadUSB**.
3. Selecione o script e aguarde a execução.

---

## 2. USB Keyboard

### O que faz
Transforma o Willy em um teclado USB real.

### Como usar
1. Navegue até **Outros -> BadUSB & HID -> Teclado USB**.
2. O teclado aparecerá na tela do Willy; o que você digitar será enviado ao computador conectado.

---

## 3. USB Clicker (AutoClicker)

### O que faz
Simula cliques automáticos de mouse em intervalos configuráveis.

### Como usar
1. Navegue até **Outros -> BadUSB & HID -> Clicker USB**.
2. Configure o intervalo e inicie.

---

# ATAQUES USB AVANÇADOS (Novos)

---

## 4. Touch Macro Recorder

### O que faz
Grava coordenadas do Touch Screen do CYD e as converte em movimentos e cliques de mouse USB em tempo real.

### Como usar
1. Vá em **Atks Avancados -> Macro Touch**.
2. Toque na tela para gravar o caminho.
3. Pressione **SEL** para dar play no computador alvo.

---

## 5. Stealth Mouse Jiggler

### O que faz
Mantém a sessão do computador ativa realizando micro-movimentos de 1px e pressionando a tecla SHIFT em intervalos aleatórios.

### Como usar
1. Selecione **Mouse Jiggler** no menu avançado.
2. Deixe o Willy conectado; ele impedirá que a tela bloqueie sem que o usuário note o cursor se movendo.

---

## 6. Keyboard + Mouse Combo Worm

### O que faz
Executa uma sequência agressiva no Windows PowerShell para simular a propagação de um arquivo worm para outros drives USB.

### Como usar
1. Conecte ao Windows e execute **Combo Worm**.
2. Ele abrirá o PowerShell e tentará copiar um script simulado para drives removíveis.

---

## 7. Conditional Delayed Payload

### O que faz
Fica em estado de "espera" (armado) e só executa o payload após um tempo de inatividade ou condição detectada (simulado).

### Como usar
1. Execute **Delayed Payload**.
2. Após 15 segundos (simulando espera por ausência do usuário), o payload é disparado.

---

## 8. HID Credential Harvester

### O que faz
Exibe uma janela de login falsa do Windows (via PowerShell) e salva a senha digitada em um arquivo local no computador alvo (`C:\Users\Public\creds.txt`).

### Como usar
1. Selecione **Cred Harvester**.
2. Uma janela de credenciais oficial do sistema aparecerá; qualquer senha digitada será salva furtivamente.

---

## 9. LED Blink Exfil Channel

### O que faz
Demonstra um canal de recepção de dados usando os LEDs de status do teclado (Caps Lock, Num Lock) para receber informações sem tráfego de rede.

---

## 10. Multi-OS Payload Switcher

### O que faz
Tenta detectar e disparar comandos específicos para Windows, Linux e macOS em sequência, garantindo que o ataque funcione não importa o sistema.

---

## 11. HID Timing Side-Channel

### O que faz
Simula o monitoramento de delays milimétricos entre o envio de relatórios USB para tentar adivinhar caracteres ou estados internos (Prova de Conceito).

---

## 12. Ransomware Simulation Demo

### O que faz
Gera um arquivo de aviso na área de trabalho e abre o bloco de notas para simular as consequências de um ataque de ransomware para fins de treinamento.

---

## 13. Persistent HID Backdoor

### O que faz
Cria uma "Tarefa Agendada" (Scheduled Task) no Windows que executa um comando persistente em modo oculto toda vez que o usuário loga.
