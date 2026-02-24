# 🔧 Interpreter Scripts

Scripts JavaScript para o interpretador do Willy Firmware.

---

## 📖 O Que é o Interpreter?

O Willy possui um interpretador JavaScript integrado que permite executar scripts customizados diretamente no dispositivo.

---

## 🚀 Como Usar

```
Menu → Interpreter → Selecione arquivo .js
```

---

## 📁 Scripts Disponíveis

| Arquivo | Função |
|---------|--------|
| `system_info.js` | Informações do sistema |
| `led_control.js` | Controle de LED RGB |
| `calculator_t-embed.js` | Calculadora |
| `crypto-prices.js` | Preços de criptomoedas |
| `dino_game.js` | Jogo do dinossauro |
| `pingpong.js` | Jogo de ping-pong |
| `Snake_Cardputer.js` | Jogo da cobrinha |
| `tamagochi.js` | Pet virtual |
| `wifi_brute.js` | Brute force WiFi |
| `ir_brute.js` | Brute force IR |
| `rf_brute.js` | Brute force RF |

---

## 📝 Criar Novo Script

### Exemplo Básico

```javascript
// Meu primeiro script
print("Olá Willy!");

// Loop
for(var i = 0; i < 5; i++) {
    print("Contagem: " + i);
}

print("Concluído!");
```

### Funções Disponíveis

| Função | Descrição |
|--------|-----------|
| `print(texto)` | Imprime no display |
| `delay(ms)` | Aguarda milissegundos |
| `getBoardName()` | Nome da placa |
| `millis()` | Tempo ativo |

---

## 📚 Referências

- [mQuickJS](https://github.com/niclasr/mQuickJS)