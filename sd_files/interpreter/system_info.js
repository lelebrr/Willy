// Sistema de Informações do Willy
print("=== Informações do Sistema ===");
print("Willy Firmware v1.0");
print("Placa: " + getBoardName());
print("CPU: " + ESP.getCpuFreqMHz() + " MHz");
print("RAM Livre: " + ESP.getFreeHeap() + " bytes");
print("Flash: " + ESP.getFlashChipSize() + " bytes");
print("SDK: " + ESP.getSdkVersion());
print("Tempo ativo: " + millis() + " ms");
