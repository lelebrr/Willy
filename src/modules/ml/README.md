# Módulo de Machine Learning - Willy ESP32-S3

Este módulo implementa algoritmos de aprendizado de máquina leves para detecção de padrões em dispositivos WiFi, RF e RFID.

## Funcionalidades

### Algoritmos Implementados
- **k-Nearest Neighbors (k-NN)**: Classificação baseada em similaridade
- **Detecção de Anomalias**: Baseada em Z-score e padrões históricos
- **Análise Estatística**: Média, desvio padrão, correlação
- **Aprendizado Incremental**: Modelos que melhoram com novos dados

### Integrações
- **WiFi**: Classificação de tipos de dispositivos (roteador, smartphone, IoT, etc.)
- **RF**: Classificação de sinais (controle remoto, sensor, porta de garagem)
- **RFID**: Detecção de comportamentos suspeitos em tags

## Uso

### Comandos Disponíveis
- `ml_train`: Treina os modelos com dados coletados
- `ml_save`: Salva modelos treinados
- `ml_load`: Carrega modelos salvos
- `ml_stats`: Mostra estatísticas dos modelos
- `ml_classify_wifi <rssi> <freq> <channel>`: Classifica dispositivo WiFi
- `ml_classify_rf <freq> <duration> <amplitude>`: Classifica sinal RF

### Exemplo de Integração

```cpp
// Obter instância do módulo ML
auto systemManager = SystemManager::getInstance();
auto mlModule = static_cast<MLModule*>(systemManager.getModule("ML"));

if (mlModule) {
    // Classificar dispositivo WiFi
    auto result = mlModule->classifyWiFiDevice(-50.0f, 2412.0f, 1);
    Serial.printf("Dispositivo classificado como: %d (confiança: %.2f)\n",
                  static_cast<int>(result.predictedType), result.confidence);

    // Adicionar dados de treinamento
    mlModule->addWiFiTrainingData(-50.0f, 2412.0f, 1, MLModule::WIFI_ROUTER);

    // Treinar modelo
    mlModule->trainModels();
}
```

## Tipos de Dispositivos

### WiFi
- `WIFI_ROUTER`: Roteadores e access points
- `WIFI_SMARTPHONE`: Smartphones e tablets
- `WIFI_IOT_DEVICE`: Dispositivos IoT (smart home, etc.)
- `WIFI_LAPTOP`: Laptops e computadores

### RF
- `RF_REMOTE_CONTROL`: Controles remotos
- `RF_SENSOR`: Sensores sem fio
- `RF_GARAGE_DOOR`: Portas de garagem

### RFID
- `RFID_CARD`: Cartões de acesso
- `RFID_TAG`: Tags genéricas
- `RFID_KEY`: Chaves RFID

## Arquitetura

### Estrutura de Dados
- `TrainingData`: Dados de treinamento com features, label e timestamp
- `ClassificationResult`: Resultado de classificação com tipo e confiança
- `AnomalyResult`: Resultado de detecção de anomalia

### Métodos Principais
- `classifyWiFiDevice()`: Classifica dispositivos WiFi
- `classifyRFSignal()`: Classifica sinais RF
- `detectRFIDAnomaly()`: Detecta anomalias RFID
- `addTrainingData()`: Adiciona dados para treinamento
- `trainModels()`: Treina os modelos
- `saveModels()` / `loadModels()`: Persistência

## Limitações

- Implementação otimizada para ESP32-S3 (memória limitada)
- Algoritmos simples mas eficazes
- Treinamento offline recomendado
- Não suporta deep learning ou redes neurais complexas

## Desenvolvimento Futuro

- Suporte a mais algoritmos (SVM, Decision Trees)
- Compressão de modelos para economia de memória
- Integração com sensores adicionais
- Aprendizado federado para múltiplos dispositivos