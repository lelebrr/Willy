#include <iostream>
#include <cassert>
#include "Arduino.h"
#include "LittleFS.h"

LittleFSClass LittleFS;

#include "willy_extracted.cpp"

void reset_config() {
    willyCfg.velocidade = 1;
    willyCfg.somAtivado = true;
    willyCfg.tipoSom = 0;
    willyCfg.corPrimaria = 0x9B00FF;
}

void test_default_config() {
    LittleFS.files.clear();
    reset_config();
    load_willy_config();

    assert(willyCfg.velocidade == 1);
    assert(willyCfg.somAtivado == true);
    assert(willyCfg.tipoSom == 0);
    assert(willyCfg.corPrimaria == 0x9B00FF);
    std::cout << "test_default_config passed" << std::endl;
}

void test_load_valid_config() {
    LittleFS.files["/willy_splash.conf"] = "2\n1\n1\nFF0000\n";
    reset_config();
    load_willy_config();

    assert(willyCfg.velocidade == 2);
    assert(willyCfg.somAtivado == true);
    assert(willyCfg.tipoSom == 1);
    assert(willyCfg.corPrimaria == 0xFF0000);
    std::cout << "test_load_valid_config passed" << std::endl;
}

void test_load_partial_config() {
    LittleFS.files["/willy_splash.conf"] = "0\n0\n";
    reset_config();
    load_willy_config();

    assert(willyCfg.velocidade == 0);
    assert(willyCfg.somAtivado == false);
    assert(willyCfg.tipoSom == 0);
    assert(willyCfg.corPrimaria == 0);
    std::cout << "test_load_partial_config passed" << std::endl;
}

void test_littlefs_fail() {
    LittleFS.files["/willy_splash.conf"] = "2\n1\n1\nFF0000\n";
    LittleFS.begin_result = false;
    reset_config();
    load_willy_config();

    assert(willyCfg.velocidade == 1);
    assert(willyCfg.somAtivado == true);
    assert(willyCfg.tipoSom == 0);
    assert(willyCfg.corPrimaria == 0x9B00FF);
    std::cout << "test_littlefs_fail passed" << std::endl;
    LittleFS.begin_result = true;
}

int main() {
    test_default_config();
    test_load_valid_config();
    test_load_partial_config();
    test_littlefs_fail();
    std::cout << "All tests passed!" << std::endl;
    return 0;
}
