// Willy: informacoes do aparelho no console JS
// Uso: Interpretador JS -> info
print("=== Willy ===");
print("Placa: " + compat.getBoardName());
print("CPU: " + compat.getCpuFreqMHz() + " MHz");
print("Flash: " + compat.getFlashChipSize() + " bytes");
print("SDK: " + compat.getSdkVersion());
print("Heap livre: " + device.freeHeap() + " bytes");
print("Bateria: " + device.battery() + "%");
