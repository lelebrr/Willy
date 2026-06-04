#include <iostream>
#include <cassert>
#include "Arduino.h"
#include "../include/willy_config.h"

void test_save_willy_config() {
    std::cout << "Testing save_willy_config()...\n";

    // Set config values
    WillyConfig& cfg = getWillyCfg();
    cfg.velocidade = 2;
    cfg.somAtivado = false;
    cfg.tipoSom = 1;
    cfg.corPrimaria = 0xFF00AA;

    // Save to mock FS
    save_willy_config();

    // %04lX on 0xFF00AA prints FF00AA
    std::string expected_content = "2\n0\n1\nFF00AA\n";

    std::cout << "Actual content: '" << LittleFS.file_content << "'\n";
    std::cout << "Expected content: '" << expected_content << "'\n";

    assert(LittleFS.file_content == expected_content);
    std::cout << "test_save_willy_config passed!\n";
}

void test_save_willy_config_edge_case() {
    std::cout << "Testing save_willy_config() edge case...\n";

    // Set config values
    WillyConfig& cfg = getWillyCfg();
    cfg.velocidade = 0;
    cfg.somAtivado = true;
    cfg.tipoSom = 0;
    cfg.corPrimaria = 0x5;

    // Save to mock FS
    save_willy_config();

    // %04lX on 0x5 prints 0005
    std::string expected_content = "0\n1\n0\n0005\n";

    assert(LittleFS.file_content == expected_content);
    std::cout << "test_save_willy_config_edge_case passed!\n";
}

void test_load_willy_config() {
    std::cout << "Testing load_willy_config()...\n";

    // Mock file content
    LittleFS.file_content = "0\n1\n0\nFFFF\n";

    // Load from mock FS
    load_willy_config();

    // Verify config values
    WillyConfig& cfg = getWillyCfg();
    assert(cfg.velocidade == 0);
    assert(cfg.somAtivado == true);
    assert(cfg.tipoSom == 0);
    assert(cfg.corPrimaria == 0xFFFF);
    std::cout << "test_load_willy_config passed!\n";
}

int main() {
    test_save_willy_config();
    test_save_willy_config_edge_case();
    test_load_willy_config();
    std::cout << "All tests passed successfully!\n";
    return 0;
}
