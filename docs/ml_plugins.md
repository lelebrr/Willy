# Frameworks Willy (ML, plugins, sistema)

> Estado: framework presente e compilado; **sem registro automático no boot**. Ative conforme abaixo.

## ML (`src/modules/ml/MLModule.{cpp,h}`)

kNN + Z-score leves: `classifyWiFiDevice()`, `classifyRFSignal()`, `detectRFIDAnomaly()`, treino (`addWiFiTrainingData`, `addRFTrainingData`, `addRFIDTrainingData`, `trainModels`), persistência (`saveModels`/`loadModels`) e estatísticas (`getModelStats`). `WiFiModule::classifyDevice()` consome via `SystemManager::getInstance().getModule("ML")` (retorna `-1` com log se ausente).

Para ativar, registre no boot (ex. no `setup()`):

```cpp
auto model = std::make_shared<SystemModel>();
auto view = std::make_shared<SystemView>();
SystemManager::getInstance().registerModule(
    std::make_unique<MLModule>(model, view));
```

## Plugins (`src/modules/plugins/`)

`IPlugin` + `PluginManager` + `PluginRegistry`: plugins JSON em `/plugins` no SD (`example_plugin.json` como modelo), comandos `plugin ...` no serial. Execução de scripts Lua/Python e verificação de assinatura estão marcados como TODO no código.

## Sistema (`src/core/System*`, `IModule.h`)

`SystemManager` (singleton: `registerModule`, `initAllModules`, `processAllModules`, `getModule`, `listModules`), `SystemController` (ciclo init/process/deinit), `SystemModel`/`SystemView`, `IModule`. Wrappers prontos: `WiFiModule`, `RFModule` (`rf/`), `RFIDModule`.

## Apoio

- `DynamicConfigManager` (`/config/system_config.json` no SD + CLI `dynamic_config`), `HardwareDetector`/`HardwareProfiles`, `PinAbstraction`/`PeripheralAbstraction`, `SecurityUtils`, `advanced_logger` (CLI usa `AdvancedLogger`), `BenchmarkManager` (CLI `benchmark`, inclui `wifi_scan`, `rf_transmit`, `rfid_read`, `ml_inference`).
