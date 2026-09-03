#include "qrcode_menu.h"
#include "../lib/TFT_eSPI_QRcode/src/qrcode.h"
#include "core/config.h"
#include "core/display.h"
#include "core/mykeyboard.h"
#include "core/settings.h"
#include "core/utils.h"

uint16_t crc_ccitt_update(uint16_t crc, uint8_t data) {
    crc = (uint8_t)(crc >> 8) | (crc << 8);
    crc ^= data;
    crc ^= (uint8_t)(crc & 0xff) >> 4;
    crc ^= crc << 12;
    crc ^= (crc & 0x00ff) << 5;
    return crc;
}

String calculate_crc(String input) {
    size_t len = input.length();
    uint8_t *data = (uint8_t *)input.c_str();
    uint16_t crc = 0xffff;

    for (size_t i = 0; i < len; i++) { crc = crc_ccitt_update(crc, data[i]); }

    String crc_str = String(crc, HEX);
    crc_str.toUpperCase();
    while (crc_str.length() < 4) crc_str = "0" + crc_str; // Pad with zeros if needed

    return crc_str;
}

void qrcode_display(String qrcodeUrl) {
#ifdef HAS_SCREEN
    QRcode qrcode(&tft);
    qrcode.init();
    qrcode.create(qrcodeUrl);
    delay(300); // Due to M5 sel press, it could be confusing with next line
    while (!check(EscPress) && !check(SelPress)) { vTaskDelay(10 / portTICK_PERIOD_MS); }
    tft.fillScreen(bruceConfig.bgColor);
#endif
}

void display_custom_qrcode() {
    String message = keyboard("", 100, "Texto do QR:");
    return qrcode_display(message);
}

void wifi_qrcode() {
    String ssid = keyboard("", 32, "SSID WiFi:");
    if (ssid == "\x1B" || ssid.isEmpty()) return;
    String pwd = keyboard("", 63, "Senha (vazio=aberta):");
    if (pwd == "\x1B") return;
    String auth = pwd.isEmpty() ? "nopass" : "WPA";
    return qrcode_display("WIFI:T:" + auth + ";S:" + ssid + ";P:" + pwd + ";;");
}

void pix_qrcode() {
    String key = keyboard("", 25, "PIX Key:");
    String key_length = key.length() >= 10 ? String(key.length()) : "0" + String(key.length());
    String amount = num_keyboard("1000.00", 10, "Int amount:");
    amount = String(amount.toFloat());
    String amount_length = amount.length() >= 10 ? String(amount.length()) : "0" + String(amount.length());

    String data0 = "0014BR.GOV.BCB.PIX01" + key_length + key;

    String pix_code = "00020126" + String(data0.length()) + data0 + "52040000530398654" + amount_length +
                      amount + "5802BR5909Bruce PIX6014Rio de Janeiro62070503***6304";
    String crc = calculate_crc(pix_code);

    return qrcode_display(pix_code + crc);
}

void qrcode_menu() {

    std::vector<Option> options;

    // Add QR codes from the config
    for (const auto &entry : bruceConfig.qrCodes) {
        options.push_back({entry.menuName.c_str(), lambdaHelper(qrcode_display, entry.content)});
    }

    options.push_back({"PIX", pix_qrcode});
    options.push_back({"WiFi", wifi_qrcode});
    options.push_back({"Custom", custom_qrcode_menu});
    addOptionToMainMenu();

    loopOptions(options);
    options.clear();
}

void custom_qrcode_menu() {
    options = {
        {"Exibir",         display_custom_qrcode  },
        {"Salvar+Exibir",  save_and_display_qrcode},
        {"Voltar",         qrcode_menu            }
    };
    loopOptions(options);
}

void save_and_display_qrcode() {

    String name = keyboard("", 100, "Nome do QR:");
    if (name.isEmpty()) {
        displayError("Nome vazio!");
        delay(1000);
        return;
    }

    if (std::any_of(
            bruceConfig.qrCodes.begin(),
            bruceConfig.qrCodes.end(),
            [&](const BruceConfig::QrCodeEntry &entry) { return entry.menuName == name; }
        )) {
        displayError("Nome ja existe!");
        delay(1000);
        return;
    }

    String text = keyboard("", 100, "Texto do QR:");

    bruceConfig.addQrCodeEntry(name, text);
    return qrcode_display(text);
}

void remove_custom_qrcode() {
    if (bruceConfig.qrCodes.empty()) {
        displayInfo("Nada a remover!");
        delay(1000);
        custom_qrcode_menu();
    }
    std::vector<Option> options;

    // Populate options with the QR codes from the config
    for (const auto &entry : bruceConfig.qrCodes) {
        options.emplace_back(entry.menuName.c_str(), [=]() {
            bruceConfig.removeQrCodeEntry(entry.menuName);
            log_i("Removed QR code: %s", entry.menuName.c_str());
            custom_qrcode_menu();
        });
    }

    options.emplace_back("Voltar", [=]() { custom_qrcode_menu(); });

    loopOptions(options);
}
