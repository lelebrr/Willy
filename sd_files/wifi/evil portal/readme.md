### **Setting AP Name from HTML**

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

#### **Overview**

The `EvilPortal` system now supports the ability to define an Access Point (AP) name directly within your HTML files. By including a specific tag in the first line of your HTML file, the system will automatically extract and set the AP name, streamlining the setup process.

**EvilPortal System - Proprietário**

---

#### **How It Works**

1. Add the following tag in the **first line** of your HTML file:

   ```html
   <!-- AP="YourCustomAPName" -->
   ```

   Replace `YourCustomAPName` with the desired name for your Access Point.

2. When the HTML file is loaded, the system will:
   - Parse the first line of the file.
   - Detect the `AP="..."` tag.
   - Extract the value and set it as the AP name.

3. If the tag is not present it will ask you for AP name (as usual).

---

#### **Example HTML File**

```html
<!-- AP="MyCoolNetwork" -->
<!DOCTYPE html>
<html>
<head>
    <title>EvilPortal</title>
</head>
<body>
    <h1>Welcome to EvilPortal!</h1>
</body>
</html>
```

- In this example, the AP name will automatically be set to **MyCoolNetwork**.

---

#### **Benefits**

- **Dynamic Configuration**: Easily customize AP names without modifying code.
- **Ease of Use**: Set up AP names directly in your HTML files for faster deployment.

---

#### **Notes**

- Ensure the `<!-- AP="..." -->` tag is in the **very first line** of the file.
- The feature does not affect the functionality of other HTML content.

---

## 📜 Licença

**⚠️ SOFTWARE PROPRIETÁRIO - TODOS OS DIREITOS RESERVADOS**

Este EvilPortal system e recursos são propriedade exclusiva da Willy Firmware e são fornecidos sob licença estritamente controlada.

### Termos de Uso

- **Uso Exclusivo**: Este sistema pode ser utilizado apenas pelo proprietário autorizado
- **Modificação Proibida**: Qualquer modificação ou distribuição é estritamente proibida
- **Responsabilidade**: O usuário é o único responsável pelo uso do sistema
- **Restrições**: Uso permitido apenas em sistemas autorizado por escrito

### Limitações de Responsabilidade

A Willy Firmware não se responsabiliza por:

- Qualquer dano resultante do uso não autorizado do sistema
- Consequências legais de atividades realizadas com Evil Twin
- Uso indevido ou violação dos termos de licença

### Direitos Autorais

© 2026 Willy Firmware. Todos os direitos reservados.

### Contato

Para informações sobre licenciamento ou suporte, entre em contato através dos canais oficiais da Willy Firmware.

---

**⚠️ AVISO LEGAL**: O uso deste sistema implica concordância total com os termos de licença proprietária. Qualquer violação resultará em ações legais cabíveis.
