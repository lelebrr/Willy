# 🌐 Captive Portals

![ESP32](https://img.shields.io/badge/ESP32-WROOM--32-red.svg)
![Arduino](https://img.shields.io/badge/Arduino-IDE-blue.svg)
![C++](https://img.shields.io/badge/C++-17-blue.svg)
![Python](https://img.shields.io/badge/Python-3.9+-yellow.svg)
![WiFi](https://img.shields.io/badge/WiFi-802.11%20a/b/g/n/ac/ax-green.svg)
![Security](https://img.shields.io/badge/Security-Pentest-red.svg)
![Hacking](https://img.shields.io/badge/Hacking-Tools-red.svg)
![Red Team](https://img.shields.io/badge/Red%20Team-Offensive-red.svg)
![Phishing](https://img.shields.io/badge/Phishing-Social%20Engineering-orange.svg)
![HTML](https://img.shields.io/badge/HTML5-Web%20Dev-blue.svg)
![JavaScript](https://img.shields.io/badge/JavaScript-ES6+-yellow.svg)

Templates de portais captive para Evil Twin e phishing WiFi - Proprietário

---

## ⚠️ Aviso Legal

**USE APENAS PARA TREINAMENTO E TESTES AUTORIZADOS!** Phishing é crime.

---

## 📁 Estrutura

```
portals/
├── en/                    # Templates em inglês
│   ├── facebook.html
│   ├── google.html
│   ├── instagram.html
│   ├── linkedin.html
│   ├── microsoft.html
│   ├── twitter.html
│   ├── amazon.html
│   └── router_update.html
└── pt-br/                 # Templates em português
    ├── facebook.html
    ├── google.html
    ├── instagram.html
    ├── microsoft.html
    ├── twitter.html
    ├── banco.html
    └── router_update.html
```

---

## 📖 O Que é Evil Twin?

Evil Twin é um ataque onde você cria um ponto de acesso falso que imita uma rede legítima. Quando vítimas conectam, são redirecionadas para um portal captive que captura credenciais.

---

## 🚀 Como Usar

### 1. Configurar Evil Portal

```
Menu → WiFi → Evil Portal
Selecione "Start Evil Portal"
Escolha o template HTML
```

### 2. Aguardar Conexões

Vítimas que conectarem ao AP verão o portal e podem inserir credenciais.

### 3. Capturar Credenciais

Credenciais são salvas automaticamente no log.

---

## 📋 Templates Disponíveis

### Redes Sociais (EN)

| Template | Alvo | Coleta |
|----------|------|--------|
| `facebook.html` | Facebook | Email/Senha |
| `google.html` | Google | Email/Senha |
| `instagram.html` | Instagram | Usuário/Senha |
| `twitter.html` | Twitter/X | Usuário/Senha |
| `linkedin.html` | LinkedIn | Email/Senha |

### Comerciais (EN)

| Template | Alvo | Coleta |
|----------|------|--------|
| `amazon.html` | Amazon | Email/Senha |
| `microsoft.html` | Microsoft | Email/Senha |

### Brasileiros (PT-BR)

| Template | Alvo | Coleta |
|----------|------|--------|
| `banco.html` | Internet Banking | Agência/Conta/Senha |
| `router_update.html` | Roteador | Senha WiFi |

---

## 🔧 Estrutura dos Arquivos HTML

```html
<!DOCTYPE html>
<html>
<head>
    <title>Título do Portal</title>
    <style>
        /* Estilos CSS */
    </style>
</head>
<body>
    <form action="/creds" method="POST">
        <input type="text" name="username">
        <input type="password" name="password">
        <button type="submit">Enviar</button>
    </form>
</body>
</html>
```

### Pontos Importantes

- `action="/creds"`: Action padrão para capturar credenciais
- `method="POST"`: Sempre use POST
- Nomes de campo: `username`, `email`, `password`, etc.

---

## 💡 Criando Novos Templates

### 1. Copie um template existente

```bash
cp en/facebook.html en/meu_template.html
```

### 2. Edite conforme necessário

Altere:

- Logo/Título
- Cores
- Campos do formulário
- Textos

### 3. Teste

Use o Evil Portal para testar antes de usar em produção.

---

## 🛡️ Detecção e Prevenção

### Como Detectar

- Verificar certificado SSL
- Desconfiar de portais não-HTTPS
- URLs suspeitas

### Como Prevenir

- Usar HTTPS em todos os sites
- Verificar certificados
- Educar usuários
- Usar MFA

---

## 📚 Referências

- [WiFi Pineapple](https://www.wifipineapple.com/)
- [Evil Twin Attack](https://en.wikipedia.org/wiki/Evil_twin_(wireless_networks))
- [Captive Portal](https://en.wikipedia.org/wiki/Captive_portal)

---

## 📜 Licença

**⚠️ SOFTWARE PROPRIETÁRIO - TODOS OS DIREITOS RESERVADOS**

Estes templates de portais captive são propriedade exclusiva da Willy Firmware e são fornecidos sob licença estritamente controlada.

### Termos de Uso

- **Uso Exclusivo**: Estes templates podem ser utilizados apenas pelo proprietário autorizado
- **Modificação Proibida**: Qualquer modificação ou distribuição é estritamente proibida
- **Responsabilidade**: O usuário é o único responsável pelo uso dos templates
- **Restrições**: Uso permitido apenas em sistemas autorizado por escrito

### Limitações de Responsabilidade

A Willy Firmware não se responsabiliza por:

- Qualquer dano resultante do uso não autorizado dos templates
- Consequências legais de atividades realizadas com phishing
- Uso indevido ou violação dos termos de licença

### Direitos Autorais

© 2026 Willy Firmware. Todos os direitos reservados.

### Contato

Para informações sobre licenciamento ou suporte, entre em contato através dos canais oficiais da Willy Firmware.

---

**⚠️ AVISO LEGAL**: O uso destes templates implica concordância total com os termos de licença proprietária. Qualquer violação resultará em ações legais cabíveis.

---
*Use com responsabilidade! 🛡️*
