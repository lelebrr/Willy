// Sistema de Informações do Bruce
print("=== Informações do Sistema ===");
print("Bruce Firmware v1.0");
print("Placa: " + getBoardName());
print("CPU: " + ESP.getCpuFreqMHz() + " MHz");
print("RAM Livre: " + ESP.getFreeHeap() + " bytes");
print("Flash: " + ESP.getFlashChipSize() + " bytes");
print("SDK: " + ESP.getSdkVersion());
print("Tempo ativo: " + millis() + " ms");