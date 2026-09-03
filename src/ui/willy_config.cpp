#include "../../include/willy_config.h"

// If we are testing, this should use the mock Arduino.h
// Otherwise, it should use the real Arduino.h and LittleFS
#ifdef WILLY_TESTING
#include "../test/Arduino.h"
#else
#include <Arduino.h>
#include <LittleFS.h>
#endif

static WillyConfig willyCfg;

WillyConfig& getWillyCfg() {
    return willyCfg;
}

bool load_willy_config() {
    if (!LittleFS.begin()) return false;
    File f = LittleFS.open("/willy_splash.conf", "r");
    if (f) {
        willyCfg.velocidade = f.readStringUntil('\n').toInt();
        willyCfg.somAtivado = f.readStringUntil('\n') == "1";
        willyCfg.tipoSom = f.readStringUntil('\n').toInt();
        willyCfg.corPrimaria = strtol(f.readStringUntil('\n').c_str(), NULL, 16);
        f.close();
        return true;
    }
    return false;
}

void save_willy_config() {
    File f = LittleFS.open("/willy_splash.conf", "w");
    if (f) {
        f.println(willyCfg.velocidade);
        f.println(willyCfg.somAtivado ? "1" : "0");
        f.println(willyCfg.tipoSom);
        f.printf("%04lX\n", (unsigned long)willyCfg.corPrimaria);
        f.close();
    }
}
