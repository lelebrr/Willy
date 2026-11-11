/**
 * @file gps_tracker.cpp
 * @author Rennan Cockles (https://github.com/rennancockles)
 * @brief GPS tracker
 * @version 0.1
 * @date 2024-11-20
 */

#include "gps_tracker.h"
#include "core/display.h"
#include "willy_logger.h"
#include "core/mykeyboard.h"
#include "core/sd_functions.h"
#include "current_year.h"
#include "core/serialcmds.h"
#include "gps_config.h"
#include <stdio.h>

#define MAX_WAIT 5000

GPSTracker::GPSTracker() { setup(); }

GPSTracker::~GPSTracker() {
    add_final_file_data();
    if (gpsConnected) end();
    ioExpander.turnPinOnOff(IO_EXP_GPS, LOW);
#ifdef USE_BOOST
    PPM.disableOTG();
#endif
    if (serialcmdsTaskHandle != NULL) vTaskResume(serialcmdsTaskHandle);
}

void GPSTracker::setup() {
    if (serialcmdsTaskHandle != NULL) vTaskSuspend(serialcmdsTaskHandle);
    ioExpander.turnPinOnOff(IO_EXP_GPS, HIGH);
#ifdef USE_BOOST /// ENABLE 5V OUTPUT
    PPM.enableOTG();
#endif
    display_banner();
    padprintln("Inicializando...");

#ifdef BRUCE_IR_SERIAL
    displayWarning("ATIVE A CHAVE GPS!", true);
    display_banner();
#endif

    if (!begin_gps()) return;

    return loop();
}

bool GPSTracker::begin_gps() {
    releasePins();
    GPSserial.begin(
        bruceConfigPins.gpsBaudrate, SERIAL_8N1, bruceConfigPins.gps_bus.rx, bruceConfigPins.gps_bus.tx
    );

    // Apply GPS configuration if in advanced mode
    if (gpsConfig.mode == GPS_MODE_ADVANCED) {
        padprintln("Aplicando config avancada...");
        vTaskDelay(500 / portTICK_PERIOD_MS);
        gpsConfig.applyConfiguration(GPSserial);

        // If baudrate was changed, reinitialize serial
        if (gpsConfig.advancedBaudrate != bruceConfigPins.gpsBaudrate && gpsConfig.advancedBaudrate > 0) {
            GPSserial.end();
            GPSserial.begin(
                gpsConfig.advancedBaudrate, SERIAL_8N1, bruceConfigPins.gps_bus.rx, bruceConfigPins.gps_bus.tx
            );
        }
    }

    int count = 0;
    padprintln("Aguardando dados GPS");
    while (GPSserial.available() <= 0) {
        if (check(EscPress)) {
            end();
            return false;
        }
        displayTextLine("Aguardando GPS: " + String(count) + "s");
        count++;
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    gpsConnected = true;
    return true;
}

void GPSTracker::end() {
    GPSserial.end();
    restorePins();

    returnToMenu = true;
    gpsConnected = false;
}

void GPSTracker::loop() {
    int count = 0;
    returnToMenu = false;
    while (1) {
        display_banner();

        if (check(EscPress) || returnToMenu) return end();

        if (GPSserial.available() > 0) {
            count = 0;
            while (GPSserial.available() > 0) gps.encode(GPSserial.read());

            if (gps.location.isUpdated()) {
                padprintln("Localizacao GPS atualizada");
                set_position();
                add_coord();
            } else {
                padprintln("Localizacao GPS nao atualiz.");
                dump_gps_data();

                if (filename == "" && gps.date.year() >= CURRENT_YEAR && gps.date.year() < CURRENT_YEAR + 5)
                    create_filename();
            }
        } else {
            if (count > 5) {
                displayError("GPS nao Encontrado!");
                return end();
            }
            padprintln("Sem dados de GPS");
            count++;
        }

        int tmp = millis();
        while (millis() - tmp < MAX_WAIT && !gps.location.isUpdated()) {
            if (check(EscPress) || returnToMenu) return end();
            vTaskDelay(50 / portTICK_PERIOD_MS);
        }
    }
}

void GPSTracker::set_position() {
    double lat = gps.location.lat();
    double lng = gps.location.lng();

    if (initial_position_set) distance += gps.distanceBetween(cur_lat, cur_lng, lat, lng);
    else initial_position_set = true;

    cur_lat = lat;
    cur_lng = lng;
}

void GPSTracker::display_banner() {
    drawMainBorderWithTitle("GPS Tracker");
    padprintln("");

    if (gpsCoordCount > 0) {
        padprintln("Arquivo: " + filename.substring(0, filename.length() - 4), 2);
        padprintln("Coordenadas GPS: " + String(gpsCoordCount), 2);
        padprintf(2, "Distancia: %.2fkm\n", distance / 1000);
    }

    padprintln("");
}

void GPSTracker::dump_gps_data() {
    if (!date_time_updated && (!gps.date.isUpdated() || !gps.time.isUpdated())) {
        padprintln("Aguardando dados GPS validos");
        return;
    }
    date_time_updated = true;
    padprintf(2, "Data: %02d-%02d-%02d\n", gps.date.year(), gps.date.month(), gps.date.day());
    padprintf(2, "Hora: %02d:%02d:%02d\n", gps.time.hour(), gps.time.minute(), gps.time.second());
    padprintf(2, "Sat:  %d\n", gps.satellites.value());
    padprintf(2, "HDOP: %.2f\n", gps.hdop.hdop());
}

void GPSTracker::create_filename() {
    char timestamp[20];
    sprintf(
        timestamp,
        "%02d%02d%02d_%02d%02d%02d",
        gps.date.year() % 100,
        gps.date.month() % 100,
        gps.date.day() % 100,
        gps.time.hour() % 100,
        gps.time.minute() % 100,
        gps.time.second() % 100
    );
    filename = String(timestamp) + "_gps_tracker.gpx";
}

void GPSTracker::add_initial_file_data(File file) {
    file.println("<?xml version=\"1.0\" encoding=\"ISO-8859-1\" standalone=\"yes\"?>");
    file.println("<?xml-stylesheet type=\"text/xsl\" href=\"details.xsl\"?>");
    file.println("<gpx");
    file.println("  version=\"1.1\"");
    file.println("  creator=\"Willy Firmware\"");
    file.println("  xmlns=\"http://www.topografix.com/GPX/1/1\"");
    file.println("  xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"");
    file.println(
        "  xsi:schemaLocation=\"http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd\""
    );
    file.println(">");
    file.println("  <metadata>");
    file.println("    <name>Willy GPS Tracker</name>");
    file.println("    <desc>GPS Tracker using Willy Firmware</desc>");
    file.println("    <link href=\"https://willy.computer\">");
    file.println("      <text>Willy Website</text>");
    file.println("    </link>");
    file.println("  </metadata>");
    file.println("  <trk>");
    file.println("    <name>Willy Route</name>");
    file.println("    <desc>GPS route captured by Willy firmware</desc>");
    file.println("    <trkseg>");
}

void GPSTracker::add_final_file_data() {
    FS *fs;
    if (!getFsStorage(fs)) return;
    if (filename == "" || !(*fs).exists("/WillyGPS/" + filename)) return;

    File file = (*fs).open("/WillyGPS/" + filename, FILE_APPEND);

    if (!file) return;
    file.println("    </trkseg>");
    file.println("  </trk>");
    file.println("</gpx>");

    file.close();
}

void GPSTracker::add_coord() {
    FS *fs;
    if (!getFsStorage(fs)) {
        padprintln("Erro config armaz.");
        returnToMenu = true;
        return;
    }

    if (filename == "") create_filename();

    if (!(*fs).exists("/WillyGPS")) (*fs).mkdir("/WillyGPS");

    bool is_new_file = false;
    if (!(*fs).exists("/WillyGPS/" + filename)) is_new_file = true;
    String dir = getHierarchicalPath("/WillyGPS");
    File file = (*fs).open(dir + "/" + filename, FILE_APPEND);

    if (!file) {
        padprintln("Falha ao abrir arquivo para gravacao");
        returnToMenu = true;
        return;
    }

    if (is_new_file) add_initial_file_data(file);

    file.printf("      <trkpt lat=\"%f\" lon=\"%f\">\n", gps.location.lat(), gps.location.lng());
    file.println("        <sym>Waypoint</sym>");
    file.printf("        <ele>%f</ele>\n", gps.altitude.meters());
    file.printf("        <hdop>%f</hdop>\n", gps.hdop.hdop());
    file.printf("        <sat>%u</sat>\n", (unsigned int)gps.satellites.value());
    file.println("      </trkpt>");

    gpsCoordCount++;

    file.close();

    willyLogger.logGPS(gps.location.lat(), gps.location.lng(), gps.altitude.meters(),
                       gps.satellites.value(), gps.location.age(), gps.location.isValid());

    padprintf(2, "Coord: %.6f, %.6f\n", gps.location.lat(), gps.location.lng());
}

void GPSTracker::releasePins() {
    rxPinReleased = false;
    if (bruceConfigPins.CC1101_bus.checkConflict(bruceConfigPins.gps_bus.rx) ||
        bruceConfigPins.NRF24_bus.checkConflict(bruceConfigPins.gps_bus.rx) ||
#if !defined(LITE_VERSION)
        bruceConfigPins.W5500_bus.checkConflict(bruceConfigPins.gps_bus.rx) ||
        bruceConfigPins.LoRa_bus.checkConflict(bruceConfigPins.gps_bus.rx) ||
#endif
        bruceConfigPins.SDCARD_bus.checkConflict(bruceConfigPins.gps_bus.rx)) {
        // T-Embed CC1101 and T-Display S3 Touch ties this pin to the NRF24 CS; switch it to input so the GPS
        // UART can drive it.
        pinMode(bruceConfigPins.gps_bus.rx, INPUT);
        rxPinReleased = true;
    }
}

void GPSTracker::restorePins() {
    if (rxPinReleased) {
        if (bruceConfigPins.CC1101_bus.checkConflict(bruceConfigPins.gps_bus.rx) ||
            bruceConfigPins.NRF24_bus.checkConflict(bruceConfigPins.gps_bus.rx) ||
#if !defined(LITE_VERSION)
            bruceConfigPins.W5500_bus.checkConflict(bruceConfigPins.gps_bus.rx) ||
            bruceConfigPins.LoRa_bus.checkConflict(bruceConfigPins.gps_bus.rx) ||
#endif
            bruceConfigPins.SDCARD_bus.checkConflict(bruceConfigPins.gps_bus.rx)) {
            // Restore the original board state after leaving the GPS app s
            // o the radio/other peripherals behave as expected
            pinMode(bruceConfigPins.gps_bus.rx, OUTPUT);
            if (bruceConfigPins.gps_bus.rx == bruceConfigPins.CC1101_bus.cs ||
                bruceConfigPins.gps_bus.rx == bruceConfigPins.NRF24_bus.cs ||
#if !defined(LITE_VERSION)
                bruceConfigPins.gps_bus.rx == bruceConfigPins.W5500_bus.cs ||
                bruceConfigPins.gps_bus.rx == bruceConfigPins.W5500_bus.cs ||
#endif
                bruceConfigPins.gps_bus.rx == bruceConfigPins.SDCARD_bus.cs) {
                // If it is conflicting to an SPI CS pin, keep it HIGH
                digitalWrite(bruceConfigPins.gps_bus.rx, HIGH);
            } else {
                // If it is conflicting with any other SPI pin, keep it LOW
                // Avoids CC1101 Jamming and nRF24 radio to keep enabled
                digitalWrite(bruceConfigPins.gps_bus.rx, LOW);
            }
        }
        rxPinReleased = false;
    }
}





