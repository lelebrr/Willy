#ifndef WILLY_CONFIG_H
#define WILLY_CONFIG_H

#include <stdint.h>

struct WillyConfig {
    int velocidade = 1;           // 0 = lento, 1 = normal, 2 = rápido
    bool somAtivado = true;
    int tipoSom = 0;              // 0 = rugido + esguicho, 1 = só esguicho
    uint32_t corPrimaria = 0x9B00FF; // Roxo neon
};

void load_willy_config();
void save_willy_config();
WillyConfig& getWillyCfg();

#endif
