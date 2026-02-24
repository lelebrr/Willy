// Controle de LED RGB
print("=== Controle LED RGB ===");

function setColor(r, g, b) {
    analogWrite(LED_R, r);
    analogWrite(LED_G, g);
    analogWrite(LED_B, b);
}

// Demo de cores
print("Vermelho...");
setColor(255, 0, 0);
delay(1000);

print("Verde...");
setColor(0, 255, 0);
delay(1000);

print("Azul...");
setColor(0, 0, 255);
delay(1000);

print("Branco...");
setColor(255, 255, 255);
delay(1000);

print("Desligado");
setColor(0, 0, 0);