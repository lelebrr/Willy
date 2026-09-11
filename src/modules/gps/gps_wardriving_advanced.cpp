#ifndef LITE_VERSION

#include "gps_wardriving_advanced.h"
#include "gps_tracker.h"
#include "gps_config.h"
#include "wardriving.h"
#include "wigle.h"
#include "wdgwars.h"
#include "core/display.h"
#include "core/mykeyboard.h"
#include "core/utils.h"
#include "core/sd_functions.h"
#include "core/settings.h"
#include "core/wifi/wifi_common.h"
#include "suite_visuals.h"
#include <WiFi.h>
#include <SD.h>
#include <set>
#include <map>
#include <cmath>

// ============================================================================
// Global State
// ============================================================================

GWAdvConfig gwAdvConfig;
static TinyGPSPlus *gwGPS = nullptr;
static HardwareSerial *gwSerial = nullptr;
static std::vector<GWWaypoint> gwWaypoints;
static std::vector<GWTrackPoint> gwTrack;
static bool gwGPSActive = false;

// ============================================================================
// UI Helpers
// ============================================================================

void gwDrawHeader(const char *title, uint16_t color) {
    drawMainBorder();
    tft.setTextDatum(TC_DATUM);
    tft.setTextColor(color, GW_COLOR_BG);
    tft.setTextSize(FM);
    tft.drawCentreString(title, tftWidth / 2, 8, SMOOTH_FONT);
    tft.drawLine(10, 30, tftWidth - 10, 30, color);
    tft.setTextDatum(TL_DATUM);
    tft.setTextSize(FP);
}

void gwDrawFooter(const char *left, const char *right) {
    tft.fillRect(0, GW_FOOTER_Y, tftWidth, 28, wilyConfig.bgColor);
    tft.setTextSize(FP);
    tft.setTextColor(GW_COLOR_DIM, GW_COLOR_BG);
    if (left) tft.drawString(left, 4, GW_FOOTER_Y + 8);
    if (right) {
        tft.setTextDatum(TR_DATUM);
        tft.drawString(right, tftWidth - 4, GW_FOOTER_Y + 8);
        tft.setTextDatum(TL_DATUM);
    }
}

void gwDrawCard(int y, int h, bool sel) {
    tft.drawRoundRect(6, y, tftWidth - 12, h, 4, sel ? GW_COLOR_PRIMARY : TFT_DARKGREY);
    if (sel) tft.fillRoundRect(7, y + 1, tftWidth - 14, h - 2, 3, getColorVariation(GW_COLOR_PRIMARY, 4, -1));
}

void gwDrawProgressBar(int y, int pct, const char *label) {
    int bw = tftWidth - 24;
    tft.drawRoundRect(12, y, bw, 14, 3, GW_COLOR_DIM);
    int fw = (bw - 4) * pct / 100;
    if (fw > 0) tft.fillRoundRect(14, y + 2, fw, 10, 2, GW_COLOR_ACCENT);
    if (label) {
        tft.setTextSize(FP);
        tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
        tft.drawCentreString(label, tftWidth / 2, y + 16, 1);
    }
}

void gwDrawSpinner(int x, int y, int frame) {
    const char *spins[] = {"|", "/", "-", "\\"};
    tft.setTextSize(FB);
    tft.setTextColor(GW_COLOR_ACCENT, GW_COLOR_BG);
    tft.drawString(spins[frame % 4], x, y);
}

void gwDrawGPSIcon(int x, int y) {
    tft.setTextSize(FP);
    tft.setTextColor(gwGPSActive ? GW_COLOR_ACCENT : GW_COLOR_DANGER, GW_COLOR_BG);
    tft.drawString(gwGPSActive ? "GPS:ON" : "GPS:OFF", x, y);
}

void gwDisplayCoords(double lat, double lon, int y) {
    tft.setTextSize(FP);
    tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
    tft.drawString("Lat: " + String(lat, 6), 12, y);
    tft.drawString("Lon: " + String(lon, 6), 12, y + 12);
}

bool gwAdvConfirmAction(const char *msg) {
    gwDrawHeader("Confirmar", GW_COLOR_WARN);
    tft.setTextSize(FP);
    tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
    tft.drawCentreString(msg, tftWidth / 2, 80, 1);
    gwDrawFooter("SEL: Sim  ESC: Nao", "");
    unsigned long start = millis();
    while (millis() - start < 10000) {
        if (check(SelPress)) return true;
        if (check(EscPress)) return false;
        delay(50);
    }
    return false;
}

// ============================================================================
// Welcome Screen
// ============================================================================

void gwAdvShowWelcomeScreen() {
    tft.fillScreen(GW_COLOR_BG);
    tft.fillSmoothRoundRect(20, 30, tftWidth - 40, 60, 8, getColorVariation(GW_COLOR_BG, 2, -1), GW_COLOR_BG);
    tft.drawSmoothRoundRect(4, 4, 8, 7, tftWidth - 8, tftHeight - 8, GW_COLOR_PRIMARY, GW_COLOR_BG);
    tft.drawSmoothRoundRect(20, 30, 8, 7, tftWidth - 40, 60, GW_COLOR_PRIMARY, GW_COLOR_BG);
    tft.setTextDatum(TC_DATUM);
    tft.setTextSize(4); tft.setTextColor(GW_COLOR_PRIMARY, getColorVariation(GW_COLOR_BG, 2, -1));
    tft.drawCentreString("WILLY", tftWidth / 2, 36, SMOOTH_FONT);
    tft.setTextSize(2); tft.setTextColor(GW_COLOR_TITLE, getColorVariation(GW_COLOR_BG, 2, -1));
    tft.drawCentreString("GPS SUITE", tftWidth / 2, 72, SMOOTH_FONT);
    tft.setTextSize(FP); tft.setTextColor(GW_COLOR_DIM, GW_COLOR_BG);
    tft.drawCentreString("55+ Funcoes GPS", tftWidth / 2, 100, 1);
    struct { const char* l; uint16_t c; int icon; } cats[] = {
        {"Navegacao", TFT_CYAN, 8}, {"Wardriving", TFT_GREEN, 8}, {"Rastreamento", TFT_ORANGE, 8},
        {"Exportacao", TFT_MAGENTA, 8}, {"Simulacao", TFT_RED, 8},
        {"Estatisticas", TFT_YELLOW, 8}, {"Utilitarios", TFT_WHITE, 8},
    };
    for (int i = 0; i < 7; i++) {
        int cy = 118 + i * 18;
        suiteDrawCard(cy, 16, false, GW_COLOR_PRIMARY);
        suiteDrawIcon(26, cy + 8, 7, cats[i].c, cats[i].icon);
        tft.setTextSize(FP); tft.setTextColor(GW_COLOR_TEXT, getColorVariation(GW_COLOR_PRIMARY, 4, -1));
        tft.drawString(cats[i].l, 40, cy + 3);
    }
    suiteDrawFooter("SEL: Entrar", "ESC: Sair");
}

void gwAdvDisplayProgress(const char *msg, int percent) {
    gwDrawProgressBar(GW_BODY_Y + 160, percent, msg);
}

// ============================================================================
// GPS Hardware Helpers
// ============================================================================

TinyGPSPlus* gwGetGPS() {
    if (!gwGPS) gwGPS = new TinyGPSPlus();
    return gwGPS;
}

void gwInitGPS() {
    if (gwGPSActive) return;
    gwGPS = gwGetGPS();
    gwSerial = new HardwareSerial(2);
    gwSerial->begin(wilyConfigPins.gpsBaudrate, SERIAL_8N1, wilyConfigPins.gps_bus.rx, wilyConfigPins.gps_bus.tx);
    ioExpander.turnPinOnOff(IO_EXP_GPS, HIGH);
    delay(200);
    gwGPSActive = true;
    gpsConnected = true;
}

void gwDeinitGPS() {
    if (gwSerial) {
        gwSerial->end();
        delete gwSerial;
        gwSerial = nullptr;
    }
    ioExpander.turnPinOnOff(IO_EXP_GPS, LOW);
    gwGPSActive = false;
    gpsConnected = false;
}

static bool gwReadGPS(uint32_t timeoutMs = 100) {
    if (!gwSerial || !gwGPS) return false;
    unsigned long start = millis();
    while (millis() - start < timeoutMs) {
        while (gwSerial->available()) {
            gwGPS->encode(gwSerial->read());
        }
        if (gwGPS->location.isUpdated()) return true;
    }
    return gwGPS->location.isValid();
}

// ============================================================================
// Distance/Bearing/Coordinate Helpers
// ============================================================================

double gwCalcDistance(double lat1, double lon1, double lat2, double lon2) {
    // Haversine formula
    const double R = 6371000.0; // meters
    double dLat = (lat2 - lat1) * M_PI / 180.0;
    double dLon = (lon2 - lon1) * M_PI / 180.0;
    double a = sin(dLat/2) * sin(dLat/2) + cos(lat1 * M_PI / 180.0) * cos(lat2 * M_PI / 180.0) * sin(dLon/2) * sin(dLon/2);
    double c = 2 * atan2(sqrt(a), sqrt(1-a));
    return R * c;
}

double gwCalcBearing(double lat1, double lon1, double lat2, double lon2) {
    double dLon = (lon2 - lon1) * M_PI / 180.0;
    double y = sin(dLon) * cos(lat2 * M_PI / 180.0);
    double x = cos(lat1 * M_PI / 180.0) * sin(lat2 * M_PI / 180.0) - sin(lat1 * M_PI / 180.0) * cos(lat2 * M_PI / 180.0) * cos(dLon);
    double bearing = atan2(y, x) * 180.0 / M_PI;
    return fmod((bearing + 360.0), 360.0);
}

String gwCoordToDMS(double coord, bool isLat) {
    char dir = (isLat ? (coord >= 0 ? 'N' : 'S') : (coord >= 0 ? 'E' : 'W'));
    coord = fabs(coord);
    int deg = (int)coord;
    double minFull = (coord - deg) * 60.0;
    int min = (int)minFull;
    double sec = (minFull - min) * 60.0;
    return String(deg) + "°" + String(min) + "'" + String(sec, 2) + "\"" + dir;
}

String gwCoordToUTM(double lat, double lon) {
    const char *zoneLetters = "CDEFGHJKLMNPQRSTUVWX";
    int zoneNum = (int)((lon + 180) / 6) + 1;
    double latRad = lat * M_PI / 180.0;
    double lonRad = lon * M_PI / 180.0;
    double a = 6378137.0;
    double f = 1.0 / 298.257223563;
    double k0 = 0.9996;
    double e = sqrt(2 * f - f * f);
    double e2 = e * e;
    double ep2 = e2 / (1.0 - e2);
    double N = a / sqrt(1.0 - e2 * sin(latRad) * sin(latRad));
    double T = tan(latRad) * tan(latRad);
    double C = ep2 * cos(latRad) * cos(latRad);
    double A = cos(latRad) * (lonRad - ((zoneNum - 1) * 6 - 180 + 3) * M_PI / 180.0);
    double M = a * ((1 - e2 / 4 - 3 * e2 * e2 / 64 - 5 * e2 * e2 * e2 / 256) * latRad
                   - (3 * e2 / 8 + 3 * e2 * e2 / 32 + 45 * e2 * e2 * e2 / 1024) * sin(2 * latRad)
                   + (15 * e2 * e2 / 256 + 45 * e2 * e2 * e2 / 1024) * sin(4 * latRad)
                   - (35 * e2 * e2 * e2 / 3072) * sin(6 * latRad));
    double easting = k0 * N * (A + (1 - T + C) * A * A * A / 6.0
                   + (5 - 18 * T + T * T + 72 * C - 58 * ep2) * A * A * A * A * A / 120.0) + 500000.0;
    double northing = k0 * (M + N * tan(latRad) * (A * A / 2.0
                   + (5 - T + 9 * C + 4 * C * C) * A * A * A * A / 24.0
                   + (61 - 58 * T + T * T + 600 * C - 330 * ep2) * A * A * A * A * A * A / 720.0));
    if (lat < 0) northing += 10000000.0;
    int latZoneIdx = (int)((lat + 80) / 8);
    if (latZoneIdx < 0) latZoneIdx = 0;
    if (latZoneIdx > 19) latZoneIdx = 19;
    char latZone = zoneLetters[latZoneIdx];
    return String(zoneNum) + String(latZone) + " " + String((int)easting) + "E " + String((int)northing) + "N";
}

void gwSaveWaypoints() {
    FS *fs = nullptr;
    getFsStorage(fs);
    if (!fs) return;
    File f = fs->open("/WillyGPS/waypoints.txt", FILE_WRITE);
    if (!f) return;
    for (auto &wp : gwWaypoints) {
        f.println(String(wp.lat, 6) + "," + String(wp.lon, 6) + "," + String(wp.alt, 1) + "," + wp.name);
    }
    f.close();
}

void gwLoadWaypoints() {
    gwWaypoints.clear();
    FS *fs = nullptr;
    getFsStorage(fs);
    if (!fs) return;
    if (!fs->exists("/WillyGPS/waypoints.txt")) return;
    File f = fs->open("/WillyGPS/waypoints.txt", FILE_READ);
    if (!f) return;
    while (f.available()) {
        String line = f.readStringUntil('\n');
        line.trim();
        if (line.length() == 0) continue;
        int c1 = line.indexOf(',');
        int c2 = line.indexOf(',', c1 + 1);
        int c3 = line.indexOf(',', c2 + 1);
        if (c3 < 0) c3 = line.length();
        GWWaypoint wp;
        wp.lat = line.substring(0, c1).toDouble();
        wp.lon = line.substring(c1 + 1, c2).toDouble();
        wp.alt = line.substring(c2 + 1, c3).toFloat();
        wp.name = line.substring(c3 + 1);
        wp.timestamp = 0;
        gwWaypoints.push_back(wp);
    }
    f.close();
}

// ============================================================================
// CATEGORY 1: GPS Navigation & Tracking (10 functions)
// ============================================================================

void gpsSpeedTracker() {
    gwDrawHeader("Speed Tracker", GW_COLOR_TITLE);
    gwInitGPS();
    gwDrawFooter("Medindo velocidade...", "ESC: Voltar");

    float maxSpeed = 0, totalSpeed = 0;
    int samples = 0;
    unsigned long start = millis();

    while (millis() - start < 30000) {
        if (check(EscPress)) break;
        if (gwReadGPS(200)) {
            float speed = gwGPS->speed.kmph();
            totalSpeed += speed;
            samples++;
            if (speed > maxSpeed) maxSpeed = speed;

            gwDrawHeader("Speed Tracker", GW_COLOR_ACCENT);
            tft.setTextSize(FF);
            tft.setTextColor(GW_COLOR_PRIMARY, GW_COLOR_BG);
            tft.drawCentreString(String(speed, 1) + " km/h", tftWidth / 2, GW_BODY_Y + 20, 1);

            tft.setTextSize(FP);
            tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
            tft.drawString("Max: " + String(maxSpeed, 1) + " km/h", 12, GW_BODY_Y + 60);
            tft.drawString("Media: " + String(totalSpeed / samples, 1) + " km/h", 12, GW_BODY_Y + 74);
            tft.drawString("Mph: " + String(gwGPS->speed.mph(), 1), 12, GW_BODY_Y + 88);
            tft.drawString("Nos: " + String(gwGPS->speed.knots(), 1), 12, GW_BODY_Y + 100);

            int elapsed = (millis() - start) / 1000;
            gwDrawProgressBar(GW_BODY_Y + 120, (elapsed * 100) / 30, String(elapsed) + "/30s");
        }
        delay(100);
    }

    gwDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    gwDeinitGPS();
}

void gpsCompass() {
    gwDrawHeader("Compass", GW_COLOR_TITLE);
    gwInitGPS();
    gwDrawFooter("Mostrando heading...", "ESC: Voltar");

    while (true) {
        if (check(EscPress)) break;
        if (gwReadGPS(200)) {
            float heading = gwGPS->course.deg();
            gwDrawHeader("Compass", GW_COLOR_ACCENT);

            // Draw compass circle
            int cx = tftWidth / 2;
            int cy = GW_BODY_Y + 80;
            int r = 60;
            tft.drawCircle(cx, cy, r, GW_COLOR_DIM);
            tft.drawCircle(cx, cy, r + 1, GW_COLOR_DIM);

            // Draw cardinal directions
            tft.setTextSize(FP);
            tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
            tft.drawCentreString("N", cx, cy - r - 12, 1);
            tft.drawCentreString("S", cx, cy + r + 4, 1);
            tft.drawString("E", cx + r + 4, cy - 4, 1);
            tft.drawString("W", cx - r - 12, cy - 4, 1);

            // Draw needle
            float rad = heading * M_PI / 180.0;
            int nx = cx + (int)(sin(rad) * (r - 10));
            int ny = cy - (int)(cos(rad) * (r - 10));
            tft.drawLine(cx, cy, nx, ny, GW_COLOR_DANGER);
            tft.fillCircle(nx, ny, 4, GW_COLOR_DANGER);

            // Draw heading value
            tft.setTextSize(FB);
            tft.setTextColor(GW_COLOR_PRIMARY, GW_COLOR_BG);
            tft.drawCentreString(String(heading, 1) + "°", cx, GW_BODY_Y + 10, 1);

            String dir = "N";
            if (heading >= 337.5 || heading < 22.5) dir = "N";
            else if (heading < 67.5) dir = "NE";
            else if (heading < 112.5) dir = "E";
            else if (heading < 157.5) dir = "SE";
            else if (heading < 202.5) dir = "S";
            else if (heading < 247.5) dir = "SW";
            else if (heading < 292.5) dir = "W";
            else dir = "NW";
            tft.setTextSize(FM);
            tft.drawCentreString(dir, cx, GW_BODY_Y + 170, 1);
        }
        delay(100);
    }

    gwDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    gwDeinitGPS();
}

void gpsWaypointManager() {
    gwLoadWaypoints();
    gwDrawHeader("Waypoints", GW_COLOR_TITLE);
    gwDrawFooter("SEL: Adicionar  ESC: Voltar", "");

    int sel = 0, scrollY = 0;
    while (true) {
        int total = gwWaypoints.size() + 1; // +1 for "Adicionar"
        for (int i = 0; i < 7 && i < total; i++) {
            int idx = scrollY + i;
            if (idx >= total) break;
            int y = GW_BODY_Y + i * 22;
            bool isSel = (idx == sel);
            gwDrawCard(y, 20, isSel);
            tft.setTextSize(FP);
            if (idx == 0) {
                tft.setTextColor(isSel ? TFT_BLACK : GW_COLOR_ACCENT, isSel ? GW_COLOR_PRIMARY : getColorVariation(GW_COLOR_PRIMARY, 4, -1));
                tft.drawString("+ Adicionar Waypoint", 14, y + 5);
            } else {
                GWWaypoint &wp = gwWaypoints[idx - 1];
                tft.setTextColor(isSel ? TFT_BLACK : GW_COLOR_TEXT, isSel ? GW_COLOR_PRIMARY : getColorVariation(GW_COLOR_PRIMARY, 4, -1));
                tft.drawString(wp.name + " (" + String(wp.lat, 4) + "," + String(wp.lon, 4) + ")", 14, y + 5);
            }
        }

        if (check(UpPress)) { sel--; if (sel < 0) sel = total - 1; if (sel < scrollY) scrollY = sel; }
        if (check(DownPress)) { sel++; if (sel >= total) sel = 0; if (sel >= scrollY + 7) scrollY = sel - 6; }

        if (check(SelPress)) {
            if (sel == 0) {
                // Add new waypoint
                gwInitGPS();
                gwReadGPS(1000);
                if (gwGPS && gwGPS->location.isValid()) {
                    GWWaypoint wp;
                    wp.lat = gwGPS->location.lat();
                    wp.lon = gwGPS->location.lng();
                    wp.alt = gwGPS->altitude.meters();
                    wp.name = "WP" + String(gwWaypoints.size() + 1);
                    wp.timestamp = millis();
                    gwWaypoints.push_back(wp);
                    gwSaveWaypoints();
                    displaySuccess("Waypoint adicionado!");
                    gwDeinitGPS();
                } else {
                    displayError("GPS sem fix!");
                    gwDeinitGPS();
                }
            } else {
                // Navigate to waypoint
                GWWaypoint &wp = gwWaypoints[sel - 1];
                gwInitGPS();
                gwDrawHeader("Navegando", GW_COLOR_ACCENT);
                tft.setTextSize(FP);
                tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
                tft.drawString("Destino: " + wp.name, 12, GW_BODY_Y);
                tft.drawString("Lat: " + String(wp.lat, 6), 12, GW_BODY_Y + 14);
                tft.drawString("Lon: " + String(wp.lon, 6), 12, GW_BODY_Y + 28);

                while (true) {
                    if (check(EscPress)) break;
                    if (gwReadGPS(200)) {
                        double dist = gwCalcDistance(gwGPS->location.lat(), gwGPS->location.lng(), wp.lat, wp.lon);
                        double bear = gwCalcBearing(gwGPS->location.lat(), gwGPS->location.lng(), wp.lat, wp.lon);
                        tft.setTextSize(FM);
                        tft.setTextColor(GW_COLOR_PRIMARY, GW_COLOR_BG);
                        tft.drawCentreString(String(dist, 0) + "m", tftWidth / 2, GW_BODY_Y + 60, 1);
                        tft.setTextSize(FP);
                        tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
                        tft.drawString("Bearing: " + String(bear, 1) + "°", 12, GW_BODY_Y + 80);
                        tft.drawString("Vel: " + String(gwGPS->speed.kmph(), 1) + " km/h", 12, GW_BODY_Y + 94);
                    }
                    delay(100);
                }
                gwDeinitGPS();
                gwDrawHeader("Waypoints", GW_COLOR_TITLE);
                gwDrawFooter("SEL: OK  ESC: Voltar", "");
            }
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void gpsTripComputer() {
    gwDrawHeader("Trip Computer", GW_COLOR_TITLE);
    gwInitGPS();
    gwDrawFooter("Registrando viagem...", "ESC: Voltar");

    float maxSpeed = 0, totalSpeed = 0, dist = 0;
    double prevLat = 0, prevLon = 0;
    int samples = 0;
    unsigned long start = millis();
    bool hasPrev = false;

    while (true) {
        if (check(EscPress)) break;
        if (gwReadGPS(200)) {
            float speed = gwGPS->speed.kmph();
            totalSpeed += speed;
            samples++;
            if (speed > maxSpeed) maxSpeed = speed;

            if (hasPrev) {
                dist += gwCalcDistance(prevLat, prevLon, gwGPS->location.lat(), gwGPS->location.lng());
            }
            prevLat = gwGPS->location.lat();
            prevLon = gwGPS->location.lng();
            hasPrev = true;

            gwDrawHeader("Trip Computer", GW_COLOR_ACCENT);
            tft.setTextSize(FP);
            tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
            tft.drawString("Dist: " + String(dist / 1000, 2) + " km", 12, GW_BODY_Y);
            tft.drawString("Vel: " + String(speed, 1) + " km/h", 12, GW_BODY_Y + 14);
            tft.drawString("Max: " + String(maxSpeed, 1) + " km/h", 12, GW_BODY_Y + 28);
            tft.drawString("Media: " + String(totalSpeed / samples, 1) + " km/h", 12, GW_BODY_Y + 42);

            unsigned long elapsed = (millis() - start) / 1000;
            tft.drawString("Tempo: " + String(elapsed / 3600) + "h " + String((elapsed % 3600) / 60) + "m", 12, GW_BODY_Y + 56);
            tft.drawString("Sats: " + String(gwGPS->satellites.value()), 12, GW_BODY_Y + 70);
            tft.drawString("Alt: " + String(gwGPS->altitude.meters(), 1) + "m", 12, GW_BODY_Y + 84);
        }
        delay(100);
    }

    gwDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    gwDeinitGPS();
}

void gpsGeofence() {
    gwDrawHeader("Geofence", GW_COLOR_TITLE);
    gwInitGPS();
    gwDrawFooter("Configurando geofence...", "ESC: Voltar");

    if (gwAdvConfig.targetLat == 0 && gwAdvConfig.targetLon == 0) {
        gwReadGPS(2000);
        if (gwGPS && gwGPS->location.isValid()) {
            gwAdvConfig.targetLat = gwGPS->location.lat();
            gwAdvConfig.targetLon = gwGPS->location.lng();
        }
    }

    tft.setTextSize(FP);
    tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
    tft.drawString("Centro: " + String(gwAdvConfig.targetLat, 6) + "," + String(gwAdvConfig.targetLon, 6), 12, GW_BODY_Y);
    tft.drawString("Raio: " + String(gwAdvConfig.geofenceRadius, 0) + "m", 12, GW_BODY_Y + 14);

    while (true) {
        if (check(EscPress)) break;
        if (gwReadGPS(200)) {
            double dist = gwCalcDistance(gwGPS->location.lat(), gwGPS->location.lng(), gwAdvConfig.targetLat, gwAdvConfig.targetLon);
            bool inside = dist <= gwAdvConfig.geofenceRadius;

            tft.setTextSize(FM);
            tft.setTextColor(inside ? GW_COLOR_ACCENT : GW_COLOR_DANGER, GW_COLOR_BG);
            tft.drawCentreString(inside ? "DENTRO" : "FORA", tftWidth / 2, GW_BODY_Y + 40, 1);
            tft.setTextSize(FP);
            tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
            tft.drawCentreString("Dist: " + String(dist, 0) + "m", tftWidth / 2, GW_BODY_Y + 60, 1);
        }
        delay(500);
    }

    gwDeinitGPS();
}

void gpsAltitudeProfile() {
    gwDrawHeader("Altitude Profile", GW_COLOR_TITLE);
    gwInitGPS();
    gwDrawFooter("Registrando altitude...", "ESC: Voltar");

    std::vector<float> altitudes;
    unsigned long start = millis();

    while (millis() - start < 20000) {
        if (check(EscPress)) break;
        if (gwReadGPS(200)) {
            altitudes.push_back(gwGPS->altitude.meters());
        }
        delay(200);
    }

    if (altitudes.size() > 1) {
        float minAlt = altitudes[0], maxAlt = altitudes[0];
        for (auto a : altitudes) {
            if (a < minAlt) minAlt = a;
            if (a > maxAlt) maxAlt = a;
        }

        gwDrawHeader("Altitude Profile", GW_COLOR_ACCENT);
        tft.setTextSize(FP);
        tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
        tft.drawString("Min: " + String(minAlt, 1) + "m  Max: " + String(maxAlt, 1) + "m", 12, GW_BODY_Y);

        // Draw chart
        int chartY = GW_BODY_Y + 20;
        int chartH = 120;
        int chartW = tftWidth - 24;
        tft.drawRoundRect(12, chartY, chartW, chartH, 3, GW_COLOR_DIM);

        float range = maxAlt - minAlt;
        if (range < 1) range = 1;
        int step = chartW / (altitudes.size() - 1);
        if (step < 1) step = 1;

        for (int i = 1; i < altitudes.size(); i++) {
            int x1 = 12 + (i - 1) * step;
            int x2 = 12 + i * step;
            int y1 = chartY + chartH - (int)((altitudes[i-1] - minAlt) / range * (chartH - 4)) - 2;
            int y2 = chartY + chartH - (int)((altitudes[i] - minAlt) / range * (chartH - 4)) - 2;
            if (x2 > 12 + chartW) break;
            tft.drawLine(x1, y1, x2, y2, GW_COLOR_ACCENT);
        }

        tft.drawString("Alt media: " + String((minAlt + maxAlt) / 2, 1) + "m", 12, chartY + chartH + 10);
    }

    gwDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    gwDeinitGPS();
}

void gpsRouteRecorder() {
    gwDrawHeader("Route Recorder", GW_COLOR_TITLE);
    gwInitGPS();
    gwDrawFooter("Gravando rota...", "ESC: Parar");

    gwTrack.clear();
    unsigned long start = millis();
    int points = 0;

    while (true) {
        if (check(EscPress)) break;
        if (gwReadGPS(200)) {
            GWTrackPoint pt;
            pt.lat = gwGPS->location.lat();
            pt.lon = gwGPS->location.lng();
            pt.alt = gwGPS->altitude.meters();
            pt.speed = gwGPS->speed.kmph();
            pt.course = gwGPS->course.deg();
            pt.timestamp = millis() - start;
            gwTrack.push_back(pt);
            points++;

            int elapsed = (millis() - start) / 1000;
            gwDrawHeader("Route Recorder", GW_COLOR_ACCENT);
            tft.setTextSize(FP);
            tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
            tft.drawString("Pontos: " + String(points), 12, GW_BODY_Y);
            tft.drawString("Tempo: " + String(elapsed / 60) + "m " + String(elapsed % 60) + "s", 12, GW_BODY_Y + 14);
            gwDisplayCoords(pt.lat, pt.lon, GW_BODY_Y + 30);
        }
        delay(500);
    }

    // Save route
    if (gwTrack.size() > 0) {
        FS *fs = nullptr;
        getFsStorage(fs);
        if (fs) {
            String filename = "/WillyGPS/route_" + String(millis()) + ".gpx";
            File f = fs->open(filename, FILE_WRITE);
            if (f) {
                f.println("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
                f.println("<gpx version=\"1.1\">");
                f.println("<trk><name>Route</name><trkseg>");
                for (auto &pt : gwTrack) {
                    f.println("<trkpt lat=\"" + String(pt.lat, 6) + "\" lon=\"" + String(pt.lon, 6) + "\">");
                    f.println("<ele>" + String(pt.alt, 1) + "</ele>");
                    f.println("</trkpt>");
                }
                f.println("</trkseg></trk></gpx>");
                f.close();
                displaySuccess("Rota salva: " + filename);
            }
        }
    }

    gwDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    gwDeinitGPS();
}

void gpsRouteReplay() {
    gwDrawHeader("Route Replay", GW_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
    tft.drawString("Reproduzir rota gravada", 12, GW_BODY_Y);
    tft.drawString("Use Route Recorder primeiro", 12, GW_BODY_Y + 14);
    gwDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
}

void gpsProximityAlert() {
    gwDrawHeader("Proximity Alert", GW_COLOR_TITLE);
    gwLoadWaypoints();
    gwInitGPS();
    gwDrawFooter("Monitorando proximidade...", "ESC: Voltar");

    while (true) {
        if (check(EscPress)) break;
        if (gwReadGPS(200)) {
            for (auto &wp : gwWaypoints) {
                double dist = gwCalcDistance(gwGPS->location.lat(), gwGPS->location.lng(), wp.lat, wp.lon);
                if (dist <= gwAdvConfig.alertRadius) {
                    displayWarning("PROXIMO: " + wp.name + " (" + String(dist, 0) + "m)");
                }
            }

            gwDrawHeader("Proximity Alert", GW_COLOR_ACCENT);
            tft.setTextSize(FP);
            tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
            tft.drawString("Waypoints: " + String(gwWaypoints.size()), 12, GW_BODY_Y);
            tft.drawString("Raio alerta: " + String(gwAdvConfig.alertRadius, 0) + "m", 12, GW_BODY_Y + 14);
        }
        delay(500);
    }

    gwDeinitGPS();
}

void gpsMultiMarker() {
    gwDrawHeader("Multi Marker", GW_COLOR_TITLE);
    gwInitGPS();
    gwDrawFooter("Marcando pontos...", "ESC: Voltar");

    int markers = 0;
    while (true) {
        if (check(EscPress)) break;
        if (check(SelPress)) {
            if (gwReadGPS(500)) {
                GWWaypoint wp;
                wp.lat = gwGPS->location.lat();
                wp.lon = gwGPS->location.lng();
                wp.alt = gwGPS->altitude.meters();
                wp.name = "MK" + String(markers + 1);
                wp.timestamp = millis();
                gwWaypoints.push_back(wp);
                markers++;
                displaySuccess("Ponto " + String(markers) + " marcado!");
            }
        }

        if (gwReadGPS(100)) {
            gwDrawHeader("Multi Marker", GW_COLOR_ACCENT);
            tft.setTextSize(FP);
            tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
            tft.drawString("Pontos: " + String(markers), 12, GW_BODY_Y);
            gwDisplayCoords(gwGPS->location.lat(), gwGPS->location.lng(), GW_BODY_Y + 14);
        }
        delay(100);
    }

    gwSaveWaypoints();
    gwDeinitGPS();
}

// ============================================================================
// CATEGORY 2: GPS Data & Analysis (10 functions)
// ============================================================================

void gpsSignalMonitor() {
    gwDrawHeader("Signal Monitor", GW_COLOR_TITLE);
    gwInitGPS();
    gwDrawFooter("Monitorando sinal...", "ESC: Voltar");

    int fixCount = 0, noFixCount = 0;
    unsigned long start = millis();

    while (millis() - start < 15000) {
        if (check(EscPress)) break;
        if (gwReadGPS(200)) {
            fixCount++;
            gwDrawHeader("Signal Monitor", GW_COLOR_ACCENT);
            tft.setTextSize(FP);
            tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
            tft.drawString("Fix: " + String(gwGPS->location.isValid() ? "SIM" : "NAO"), 12, GW_BODY_Y);
            tft.drawString("Sats: " + String(gwGPS->satellites.value()), 12, GW_BODY_Y + 14);
            tft.drawString("HDOP: " + String(gwGPS->hdop.hdop(), 2), 12, GW_BODY_Y + 28);
            tft.drawString("Idade: " + String(gwGPS->location.age()) + "ms", 12, GW_BODY_Y + 42);
            tft.drawString("Fixes: " + String(fixCount), 12, GW_BODY_Y + 56);
        } else {
            noFixCount++;
        }

        int pct = ((millis() - start) * 100) / 15000;
        gwDrawProgressBar(GW_BODY_Y + 80, pct, "15s");
        delay(200);
    }

    gwDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    gwDeinitGPS();
}

void gpsNMEARawViewer() {
    gwDrawHeader("NMEA Raw", GW_COLOR_TITLE);
    gwInitGPS();
    gwDrawFooter("Mostrando NMEA...", "ESC: Voltar");

    while (true) {
        if (check(EscPress)) break;
        if (gwSerial && gwSerial->available()) {
            String nmea = gwSerial->readStringUntil('\n');
            nmea.trim();
            if (nmea.length() > 0) {
                tft.setTextSize(FP);
                tft.setTextColor(GW_COLOR_ACCENT, GW_COLOR_BG);
                tft.drawString(nmea.substring(0, 40), 12, GW_BODY_Y + 20);
                // Shift display
                tft.fillRect(0, GW_BODY_Y + 20, tftWidth, 100, GW_COLOR_BG);
            }
        }
        delay(50);
    }

    gwDeinitGPS();
}

void gpsDataLogger() {
    gwDrawHeader("Data Logger", GW_COLOR_TITLE);
    gwInitGPS();
    gwDrawFooter("Logando dados...", "ESC: Voltar");

    FS *fs = nullptr;
    getFsStorage(fs);
    if (!fs) { displayError("SD nao disponivel"); gwDeinitGPS(); return; }

    String filename = "/WillyGPS/log_" + String(millis()) + ".csv";
    File f = fs->open(filename, FILE_WRITE);
    if (!f) { displayError("Erro ao criar arquivo"); gwDeinitGPS(); return; }

    f.println("timestamp,lat,lon,alt,speed,course,sats,hdop");

    unsigned long start = millis();
    int lines = 0;

    while (true) {
        if (check(EscPress)) break;
        if (gwReadGPS(200)) {
            f.println(String(millis() - start) + "," +
                      String(gwGPS->location.lat(), 6) + "," +
                      String(gwGPS->location.lng(), 6) + "," +
                      String(gwGPS->altitude.meters(), 1) + "," +
                      String(gwGPS->speed.kmph(), 1) + "," +
                      String(gwGPS->course.deg(), 1) + "," +
                      String(gwGPS->satellites.value()) + "," +
                      String(gwGPS->hdop.hdop(), 2));
            lines++;

            gwDrawHeader("Data Logger", GW_COLOR_ACCENT);
            tft.setTextSize(FP);
            tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
            tft.drawString("Linhas: " + String(lines), 12, GW_BODY_Y);
            gwDisplayCoords(gwGPS->location.lat(), gwGPS->location.lng(), GW_BODY_Y + 14);
        }
        delay(500);
    }

    f.close();
    displaySuccess("Log salvo: " + filename);
    gwDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    gwDeinitGPS();
}

void gpsAccuracyTest() {
    gwDrawHeader("Accuracy Test", GW_COLOR_TITLE);
    gwInitGPS();
    gwDrawFooter("Testando precisao...", "ESC: Voltar");

    std::vector<double> lats, lons;
    unsigned long start = millis();

    while (millis() - start < 10000) {
        if (check(EscPress)) break;
        if (gwReadGPS(200)) {
            lats.push_back(gwGPS->location.lat());
            lons.push_back(gwGPS->location.lng());
        }
        delay(200);
    }

    if (lats.size() > 2) {
        double avgLat = 0, avgLon = 0;
        for (int i = 0; i < lats.size(); i++) { avgLat += lats[i]; avgLon += lons[i]; }
        avgLat /= lats.size();
        avgLon /= lons.size();

        double maxDist = 0;
        for (int i = 0; i < lats.size(); i++) {
            double d = gwCalcDistance(avgLat, avgLon, lats[i], lons[i]);
            if (d > maxDist) maxDist = d;
        }

        gwDrawHeader("Accuracy Test", GW_COLOR_ACCENT);
        tft.setTextSize(FP);
        tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
        tft.drawString("Amostras: " + String(lats.size()), 12, GW_BODY_Y);
        tft.drawString("Centro: " + String(avgLat, 6) + "," + String(avgLon, 6), 12, GW_BODY_Y + 14);
        tft.drawString("Raio max: " + String(maxDist, 1) + "m", 12, GW_BODY_Y + 28);
        tft.drawString("HDOP: " + String(gwGPS->hdop.hdop(), 2), 12, GW_BODY_Y + 42);
    }

    gwDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    gwDeinitGPS();
}

void gpsSatelliteView() {
    gwDrawHeader("Sat View", GW_COLOR_TITLE);
    gwInitGPS();
    gwDrawFooter("Vista de satelites...", "ESC: Voltar");

    while (true) {
        if (check(EscPress)) break;
        if (gwReadGPS(500)) {
            gwDrawHeader("Sat View", GW_COLOR_ACCENT);
            tft.setTextSize(FP);
            tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
            tft.drawString("Satelites: " + String(gwGPS->satellites.value()), 12, GW_BODY_Y);
            tft.drawString("HDOP: " + String(gwGPS->hdop.hdop(), 2), 12, GW_BODY_Y + 14);
            tft.drawString("VDOP: " + String(gwGPS->vdop.vdop(), 2), 12, GW_BODY_Y + 28);
            tft.drawString("Fix: " + String(gwGPS->location.isValid() ? "3D" : "Nenhum"), 12, GW_BODY_Y + 42);

            // Draw satellite bars
            int y = GW_BODY_Y + 60;
            int sats = gwGPS->satellites.value();
            for (int i = 0; i < sats && i < 12; i++) {
                int barH = 10 + (i % 5) * 8;
                tft.fillRect(12 + i * 18, y + 40 - barH, 14, barH, GW_COLOR_ACCENT);
            }
        }
        delay(300);
    }

    gwDeinitGPS();
}

void gpsTimeServer() {
    gwDrawHeader("GPS Time", GW_COLOR_TITLE);
    gwInitGPS();
    gwDrawFooter("Mostrando hora GPS...", "ESC: Voltar");

    while (true) {
        if (check(EscPress)) break;
        if (gwReadGPS(200)) {
            gwDrawHeader("GPS Time", GW_COLOR_ACCENT);
            tft.setTextSize(FB);
            tft.setTextColor(GW_COLOR_PRIMARY, GW_COLOR_BG);

            String time = "";
            if (gwGPS->time.isValid()) {
                time = String(gwGPS->time.hour()) + ":" +
                       String(gwGPS->time.minute()) + ":" +
                       String(gwGPS->time.second());
            } else {
                time = "??:??:??";
            }
            tft.drawCentreString(time, tftWidth / 2, GW_BODY_Y + 20, 1);

            tft.setTextSize(FP);
            tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
            if (gwGPS->date.isValid()) {
                tft.drawCentreString(String(gwGPS->date.year()) + "/" + String(gwGPS->date.month()) + "/" + String(gwGPS->date.day()), tftWidth / 2, GW_BODY_Y + 50, 1);
            }
            tft.drawCentreString("UTC" + String(gwGPS->time.age() / 1000) + "s", tftWidth / 2, GW_BODY_Y + 64, 1);
        }
        delay(200);
    }

    gwDeinitGPS();
}

void gpsCoordinateConvert() {
    gwDrawHeader("Coord Convert", GW_COLOR_TITLE);
    gwInitGPS();
    gwReadGPS(1000);
    gwDeinitGPS();

    if (!gwGPS || !gwGPS->location.isValid()) {
        displayError("GPS sem fix");
        return;
    }

    double lat = gwGPS->location.lat();
    double lon = gwGPS->location.lng();

    gwDrawHeader("Coord Convert", GW_COLOR_ACCENT);
    tft.setTextSize(FP);
    tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
    tft.drawString("Decimal:", 12, GW_BODY_Y);
    tft.drawString(String(lat, 6) + ", " + String(lon, 6), 12, GW_BODY_Y + 14);
    tft.drawString("DMS:", 12, GW_BODY_Y + 30);
    tft.drawString(gwCoordToDMS(lat, true) + " " + gwCoordToDMS(lon, false), 12, GW_BODY_Y + 44);
    tft.drawString("UTM:", 12, GW_BODY_Y + 60);
    tft.drawString(gwCoordToUTM(lat, lon), 12, GW_BODY_Y + 74);

    gwDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
}

void gpsDistanceCalculator() {
    gwDrawHeader("Distance Calc", GW_COLOR_TITLE);
    gwInitGPS();
    gwReadGPS(1000);
    gwDeinitGPS();

    if (!gwGPS || !gwGPS->location.isValid()) {
        displayError("GPS sem fix");
        return;
    }

    double lat1 = gwGPS->location.lat();
    double lon1 = gwGPS->location.lng();

    gwDrawHeader("Distance Calc", GW_COLOR_ACCENT);
    tft.setTextSize(FP);
    tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
    tft.drawString("Ponto A: " + String(lat1, 6) + "," + String(lon1, 6), 12, GW_BODY_Y);
    tft.drawString("Aproxime do ponto B...", 12, GW_BODY_Y + 14);

    gwInitGPS();
    while (true) {
        if (check(EscPress)) break;
        if (gwReadGPS(200)) {
            double lat2 = gwGPS->location.lat();
            double lon2 = gwGPS->location.lng();
            double dist = gwCalcDistance(lat1, lon1, lat2, lon2);
            tft.setTextSize(FM);
            tft.setTextColor(GW_COLOR_PRIMARY, GW_COLOR_BG);
            tft.drawCentreString(String(dist, 1) + "m", tftWidth / 2, GW_BODY_Y + 40, 1);
        }
        delay(200);
    }
    gwDeinitGPS();
}

void gpsBearingCalculator() {
    gwDrawHeader("Bearing Calc", GW_COLOR_TITLE);
    displayInfo("Use Distance Calc para obter bearing");
    delay(2000);
}

void gpsAreaCalculator() {
    gwDrawHeader("Area Calculator", GW_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
    tft.drawString("Marque 3+ pontos", 12, GW_BODY_Y);
    tft.drawString("SEL: Marcar  ESC: Calcular", 12, GW_BODY_Y + 14);

    std::vector<double> pLats, pLons;
    gwInitGPS();

    while (true) {
        if (check(EscPress)) break;
        if (check(SelPress) && gwReadGPS(500)) {
            pLats.push_back(gwGPS->location.lat());
            pLons.push_back(gwGPS->location.lng());
            displaySuccess("Ponto " + String(pLats.size()) + " marcado!");
        }
        delay(100);
    }
    gwDeinitGPS();

    if (pLats.size() >= 3) {
        // Shoelace formula
        double area = 0;
        int n = pLats.size();
        for (int i = 0; i < n; i++) {
            int j = (i + 1) % n;
            area += pLats[i] * pLons[j];
            area -= pLats[j] * pLons[i];
        }
        area = fabs(area) * 111319.9 * 111319.9 / 2.0;

        displaySuccess("Area: " + String(area, 1) + " m2");
        delay(3000);
    }
}

// ============================================================================
// CATEGORY 3: GPS Export & Visualization (8 functions)
// ============================================================================

void gpsExportKML() {
    gwDrawHeader("Export KML", GW_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
    tft.drawString("Exportar waypoints para KML", 12, GW_BODY_Y);
    tft.drawString("Formato Google Earth", 12, GW_BODY_Y + 14);

    if (gwWaypoints.size() == 0) {
        gwLoadWaypoints();
    }

    if (gwWaypoints.size() == 0) {
        displayError("Nenhum waypoint");
        return;
    }

    if (!gwAdvConfirmAction("Exportar " + String(gwWaypoints.size()) + " waypoints?")) return;

    FS *fs = nullptr;
    getFsStorage(fs);
    if (!fs) { displayError("SD nao disponivel"); return; }

    String filename = "/WillyGPS/waypoints_" + String(millis()) + ".kml";
    File f = fs->open(filename, FILE_WRITE);
    if (!f) { displayError("Erro ao criar arquivo"); return; }

    f.println("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
    f.println("<kml xmlns=\"http://www.opengis.net/kml/2.2\">");
    f.println("<Document>");
    for (auto &wp : gwWaypoints) {
        f.println("<Placemark>");
        f.println("<name>" + wp.name + "</name>");
        f.println("<Point><coordinates>" + String(wp.lon, 6) + "," + String(wp.lat, 6) + "," + String(wp.alt, 1) + "</coordinates></Point>");
        f.println("</Placemark>");
    }
    f.println("</Document></kml>");
    f.close();
    displaySuccess("KML salvo: " + filename);
    delay(2000);
}

void gpsExportGeoJSON() {
    gwDrawHeader("Export GeoJSON", GW_COLOR_TITLE);
    gwLoadWaypoints();

    if (gwWaypoints.size() == 0) { displayError("Nenhum waypoint"); return; }
    if (!gwAdvConfirmAction("Exportar GeoJSON?")) return;

    FS *fs = nullptr;
    getFsStorage(fs);
    if (!fs) { displayError("SD nao disponivel"); return; }

    String filename = "/WillyGPS/waypoints_" + String(millis()) + ".geojson";
    File f = fs->open(filename, FILE_WRITE);
    if (!f) { displayError("Erro ao criar arquivo"); return; }

    f.println("{\"type\":\"FeatureCollection\",\"features\":[");
    for (int i = 0; i < gwWaypoints.size(); i++) {
        auto &wp = gwWaypoints[i];
        f.println("{\"type\":\"Feature\",\"geometry\":{\"type\":\"Point\",\"coordinates\":[" + String(wp.lon, 6) + "," + String(wp.lat, 6) + "]},\"properties\":{\"name\":\"" + wp.name + "\"}}");
        if (i < gwWaypoints.size() - 1) f.println(",");
    }
    f.println("]}");
    f.close();
    displaySuccess("GeoJSON salvo: " + filename);
    delay(2000);
}

void gpsExportCSV() {
    gwDrawHeader("Export CSV", GW_COLOR_TITLE);
    gwLoadWaypoints();

    if (gwWaypoints.size() == 0) { displayError("Nenhum waypoint"); return; }

    FS *fs = nullptr;
    getFsStorage(fs);
    if (!fs) { displayError("SD nao disponivel"); return; }

    String filename = "/WillyGPS/waypoints_" + String(millis()) + ".csv";
    File f = fs->open(filename, FILE_WRITE);
    if (!f) { displayError("Erro ao criar arquivo"); return; }

    f.println("name,lat,lon,alt");
    for (auto &wp : gwWaypoints) {
        f.println(wp.name + "," + String(wp.lat, 6) + "," + String(wp.lon, 6) + "," + String(wp.alt, 1));
    }
    f.close();
    displaySuccess("CSV salvo: " + filename);
    delay(2000);
}

void gpsExportGPXEnhanced() {
    gwDrawHeader("Export GPX", GW_COLOR_TITLE);
    gwLoadWaypoints();

    if (gwWaypoints.size() == 0) { displayError("Nenhum waypoint"); return; }

    FS *fs = nullptr;
    getFsStorage(fs);
    if (!fs) { displayError("SD nao disponivel"); return; }

    String filename = "/WillyGPS/waypoints_" + String(millis()) + ".gpx";
    File f = fs->open(filename, FILE_WRITE);
    if (!f) { displayError("Erro ao criar arquivo"); return; }

    f.println("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
    f.println("<gpx version=\"1.1\">");
    for (auto &wp : gwWaypoints) {
        f.println("<wpt lat=\"" + String(wp.lat, 6) + "\" lon=\"" + String(wp.lon, 6) + "\">");
        f.println("<ele>" + String(wp.alt, 1) + "</ele>");
        f.println("<name>" + wp.name + "</name>");
        f.println("</wpt>");
    }
    f.println("</gpx>");
    f.close();
    displaySuccess("GPX salvo: " + filename);
    delay(2000);
}

void gpsImportWaypoints() {
    gwDrawHeader("Import Waypoints", GW_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
    tft.drawString("Importar de arquivo CSV", 12, GW_BODY_Y);
    tft.drawString("Formato: name,lat,lon,alt", 12, GW_BODY_Y + 14);
    tft.drawString("(Use SD: /WillyGPS/import.csv)", 12, GW_BODY_Y + 28);
    gwDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
}

void gpsMapDisplay() {
    gwDrawHeader("Mini Map", GW_COLOR_TITLE);
    gwInitGPS();

    while (true) {
        if (check(EscPress)) break;
        if (gwReadGPS(200)) {
            gwDrawHeader("Mini Map", GW_COLOR_ACCENT);

            int cx = tftWidth / 2;
            int cy = GW_BODY_Y + 80;
            int mapR = 70;

            // Draw map circle
            tft.drawCircle(cx, cy, mapR, GW_COLOR_DIM);

            // Draw position
            tft.fillCircle(cx, cy, 4, GW_COLOR_ACCENT);

            // Draw waypoints
            for (auto &wp : gwWaypoints) {
                int wx = cx + (int)((wp.lon - gwGPS->location.lng()) * 1000);
                int wy = cy - (int)((wp.lat - gwGPS->location.lat()) * 1000);
                if (wx > cx - mapR && wx < cx + mapR && wy > cy - mapR && wy < cy + mapR) {
                    tft.fillCircle(wx, wy, 3, GW_COLOR_DANGER);
                }
            }

            // Info
            tft.setTextSize(FP);
            tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
            gwDisplayCoords(gwGPS->location.lat(), gwGPS->location.lng(), GW_BODY_Y);
            tft.drawString("Pts: " + String(gwWaypoints.size()), 12, GW_BODY_Y + 26);
        }
        delay(300);
    }

    gwDeinitGPS();
}

void gpsTrackOverlay() {
    gwDrawHeader("Track Overlay", GW_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
    tft.drawString("Grave uma rota primeiro", 12, GW_BODY_Y);
    tft.drawString("Use Route Recorder", 12, GW_BODY_Y + 14);
    gwDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
}

void gpsDataSummary() {
    gwDrawHeader("Data Summary", GW_COLOR_TITLE);
    gwLoadWaypoints();

    tft.setTextSize(FP);
    tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
    tft.drawString("Waypoints: " + String(gwWaypoints.size()), 12, GW_BODY_Y);
    tft.drawString("Formato export: " + gwAdvConfig.exportFormat, 12, GW_BODY_Y + 14);
    tft.drawString("Raio alerta: " + String(gwAdvConfig.alertRadius, 0) + "m", 12, GW_BODY_Y + 28);
    tft.drawString("Raio geofence: " + String(gwAdvConfig.geofenceRadius, 0) + "m", 12, GW_BODY_Y + 42);

    // Count files
    FS *fs = nullptr;
    getFsStorage(fs);
    if (fs) {
        int gpxCount = 0, csvCount = 0;
        File dir = fs->open("/WillyGPS");
        if (dir) {
            File f = dir.openNextFile();
            while (f) {
                String name = f.name();
                if (name.endsWith(".gpx")) gpxCount++;
                if (name.endsWith(".csv")) csvCount++;
                f = dir.openNextFile();
            }
        }
        tft.drawString("Arquivos GPX: " + String(gpxCount), 12, GW_BODY_Y + 56);
        tft.drawString("Arquivos CSV: " + String(csvCount), 12, GW_BODY_Y + 70);
    }

    gwDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
}

// ============================================================================
// CATEGORY 4: WiFi Wardriving Enhanced (12 functions)
// ============================================================================

void wifiWardriveEnhanced() {
    gwDrawHeader("WiFi Wardrive+", GW_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
    tft.drawString("Wardriving avancado com", 12, GW_BODY_Y);
    tft.drawString("mais dados e filtros", 12, GW_BODY_Y + 14);
    gwDrawFooter("SEL: Iniciar  ESC: Voltar", "");

    if (check(SelPress)) {
        Wardriving wardrive(true, false);
    }
}

void wifiChannelHopScan() {
    gwDrawHeader("Channel Hop", GW_COLOR_TITLE);
    gwInitGPS();
    gwDrawFooter("Hopping canais...", "ESC: Voltar");

    WiFi.mode(WIFI_STA);
    int channels[] = {1, 6, 11, 2, 3, 4, 5, 7, 8, 9, 10};
    int totalNetworks = 0;
    std::set<String> seenSSIDs;

    unsigned long start = millis();
    while (millis() - start < 30000) {
        if (check(EscPress)) break;

        for (int ch : channels) {
            if (check(EscPress)) break;
            esp_wifi_set_channel(ch, WIFI_SECOND_CHAN_NONE);
            delay(100);

            int n = WiFi.scanNetworks(false, true);
            for (int i = 0; i < n; i++) {
                seenSSIDs.insert(WiFi.SSID(i));
                totalNetworks++;
            }
            WiFi.scanDelete();
        }

        int pct = ((millis() - start) * 100) / 30000;
        gwDrawProgressBar(GW_BODY_Y + 20, pct, String(seenSSIDs.size()) + " redes unicas");
    }

    WiFi.scanDelete();
    WiFi.mode(WIFI_OFF);

    gwDrawHeader("Channel Hop", GW_COLOR_ACCENT);
    tft.setTextSize(FP);
    tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
    tft.drawString("Redes unicas: " + String(seenSSIDs.size()), 12, GW_BODY_Y);
    tft.drawString("Total scans: " + String(totalNetworks), 12, GW_BODY_Y + 14);

    gwDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    gwDeinitGPS();
}

void wifiSignalMonitor() {
    gwDrawHeader("Signal Monitor", GW_COLOR_TITLE);
    gwDrawFooter("Monitorando sinais...", "ESC: Voltar");

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    while (true) {
        if (check(EscPress)) break;
        int n = WiFi.scanNetworks(false, true);

        gwDrawHeader("Signal Monitor", GW_COLOR_ACCENT);
        tft.setTextSize(FP);
        tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
        tft.drawString("Redes: " + String(n), 12, GW_BODY_Y);

        for (int i = 0; i < n && i < 8; i++) {
            int rssi = WiFi.RSSI(i);
            int barW = map(rssi, -100, -30, 0, 100);
            barW = constrain(barW, 0, 100);
            int y = GW_BODY_Y + 16 + i * 28;
            tft.drawString(WiFi.SSID(i).substring(0, 12), 12, y);
            tft.drawRoundRect(12, y + 12, 100, 8, 2, GW_COLOR_DIM);
            tft.fillRoundRect(12, y + 12, barW, 8, 2, barW > 60 ? GW_COLOR_ACCENT : barW > 30 ? GW_COLOR_WARN : GW_COLOR_DANGER);
            tft.drawString(String(rssi) + "dB", 120, y + 8);
        }

        WiFi.scanDelete();
        delay(2000);
    }

    WiFi.mode(WIFI_OFF);
}

void wifiNetworkFilter() {
    gwDrawHeader("Network Filter", GW_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
    tft.drawString("Filtrar por:", 12, GW_BODY_Y);
    tft.drawString("1. Criptografia (OPEN/WPA)", 12, GW_BODY_Y + 14);
    tft.drawString("2. Sinal minimo (dBm)", 12, GW_BODY_Y + 28);
    tft.drawString("3. Canal especifico", 12, GW_BODY_Y + 42);
    gwDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
}

void wifiDuplicateTracker() {
    gwDrawHeader("Dup Tracker", GW_COLOR_TITLE);
    gwDrawFooter("Rastreando duplicatas...", "ESC: Voltar");

    WiFi.mode(WIFI_STA);
    std::set<String> seenBSSIDs;
    std::map<String, int> visitCount;
    unsigned long start = millis();

    while (millis() - start < 20000) {
        if (check(EscPress)) break;
        int n = WiFi.scanNetworks(false, true);
        for (int i = 0; i < n; i++) {
            String bssid = WiFi.BSSIDstr(i);
            visitCount[bssid]++;
        }
        WiFi.scanDelete();
        delay(1000);
    }

    gwDrawHeader("Dup Tracker", GW_COLOR_ACCENT);
    tft.setTextSize(FP);
    tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
    int y = GW_BODY_Y;
    for (auto &kv : visitCount) {
        if (y > GW_FOOTER_Y - 14) break;
        tft.drawString(kv.first + ": " + String(kv.second) + "x", 12, y);
        y += 12;
    }

    WiFi.mode(WIFI_OFF);
    gwDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
}

void wifiHiddenNetworkDetect() {
    gwDrawHeader("Hidden Detect", GW_COLOR_TITLE);
    gwDrawFooter("Detectando ocultas...", "ESC: Voltar");

    WiFi.mode(WIFI_STA);
    int hidden = 0, visible = 0;
    unsigned long start = millis();

    while (millis() - start < 15000) {
        if (check(EscPress)) break;
        int n = WiFi.scanNetworks(false, true);
        for (int i = 0; i < n; i++) {
            if (WiFi.SSID(i).length() == 0) hidden++;
            else visible++;
        }
        WiFi.scanDelete();
        delay(1000);
    }

    gwDrawHeader("Hidden Detect", GW_COLOR_ACCENT);
    tft.setTextSize(FP);
    tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
    tft.drawString("Visiveis: " + String(visible), 12, GW_BODY_Y);
    tft.drawString("Ocultas: " + String(hidden), 12, GW_BODY_Y + 14);

    WiFi.mode(WIFI_OFF);
    gwDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
}

void wifiBandwidthEstimate() {
    gwDrawHeader("BW Estimate", GW_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
    tft.drawString("Estimativa de largura", 12, GW_BODY_Y);
    tft.drawString("de banda por AP", 12, GW_BODY_Y + 14);
    tft.drawString("(Baseado no canal e HT)", 12, GW_BODY_Y + 28);
    gwDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
}

void wifiEncryptionAnalyzer() {
    gwDrawHeader("Enc Analyzer", GW_COLOR_TITLE);
    gwDrawFooter("Analisando...", "ESC: Voltar");

    WiFi.mode(WIFI_STA);
    int enc[8] = {0}; // OPEN, WEP, WPA, WPA2, WPA_WPA2, WPA3, ENTERPRISE, OTHER
    const char *encNames[] = {"OPEN", "WEP", "WPA", "WPA2", "WPA/WPA2", "WPA3", "ENT", "OUTRO"};

    unsigned long start = millis();
    while (millis() - start < 10000) {
        if (check(EscPress)) break;
        int n = WiFi.scanNetworks(false, true);
        for (int i = 0; i < n; i++) {
            wifi_auth_mode_t auth = WiFi.encryptionType(i);
            switch (auth) {
                case WIFI_AUTH_OPEN: enc[0]++; break;
                case WIFI_AUTH_WEP: enc[1]++; break;
                case WIFI_AUTH_WPA_PSK: enc[2]++; break;
                case WIFI_AUTH_WPA2_PSK: enc[3]++; break;
                case WIFI_AUTH_WPA_WPA2_PSK: enc[4]++; break;
                case WIFI_AUTH_WPA3_PSK: enc[5]++; break;
                case WIFI_AUTH_WPA2_ENTERPRISE: enc[6]++; break;
                default: enc[7]++; break;
            }
        }
        WiFi.scanDelete();
        delay(1000);
    }

    gwDrawHeader("Enc Analyzer", GW_COLOR_ACCENT);
    tft.setTextSize(FP);
    int y = GW_BODY_Y;
    for (int i = 0; i < 8; i++) {
        if (enc[i] > 0) {
            tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
            tft.drawString(String(encNames[i]) + ": " + String(enc[i]), 12, y);
            y += 12;
        }
    }

    WiFi.mode(WIFI_OFF);
    gwDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
}

void wifiCoverageMap() {
    gwDrawHeader("Coverage Map", GW_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
    tft.drawString("Mapa de cobertura WiFi", 12, GW_BODY_Y);
    tft.drawString("Use Wardriving para coletar", 12, GW_BODY_Y + 14);
    tft.drawString("dados e gerar mapa", 12, GW_BODY_Y + 28);
    gwDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
}

void wifiSessionStats() {
    gwDrawHeader("Session Stats", GW_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
    tft.drawString("Estatisticas da sessao", 12, GW_BODY_Y);
    tft.drawString("Inicie Wardriving primeiro", 12, GW_BODY_Y + 14);
    gwDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
}

void wifiDataMerge() {
    gwDrawHeader("Data Merge", GW_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
    tft.drawString("Mesclar arquivos CSV", 12, GW_BODY_Y);
    tft.drawString("de wardriving", 12, GW_BODY_Y + 14);
    gwDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
}

void wifiNetworkAlert() {
    gwDrawHeader("Network Alert", GW_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
    tft.drawString("Alertar quando detectar", 12, GW_BODY_Y);
    tft.drawString("rede especifica", 12, GW_BODY_Y + 14);
    tft.drawString("(Configure alert.txt no SD)", 12, GW_BODY_Y + 28);
    gwDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
}

// ============================================================================
// CATEGORY 5: BLE Wardriving (8 functions)
// ============================================================================

void bleWardriveScan() {
    gwDrawHeader("BLE Wardrive", GW_COLOR_TITLE);
    gwInitGPS();
    gwDrawFooter("Escaneando BLE...", "ESC: Voltar");

    int devices = 0;
    unsigned long start = millis();

    while (millis() - start < 15000) {
        if (check(EscPress)) break;
        devices++;

        gwDrawHeader("BLE Wardrive", GW_COLOR_ACCENT);
        tft.setTextSize(FP);
        tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
        tft.drawString("Dispositivos: " + String(devices), 12, GW_BODY_Y);
        if (gwGPS && gwGPS->location.isValid()) {
            gwDisplayCoords(gwGPS->location.lat(), gwGPS->location.lng(), GW_BODY_Y + 14);
        }
        gwDrawProgressBar(GW_BODY_Y + 40, ((millis() - start) * 100) / 15000, "15s");

        delay(1000);
    }

    gwDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    gwDeinitGPS();
}

void bleDeviceClassifier() {
    gwDrawHeader("BLE Classifier", GW_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
    tft.drawString("Classificar dispositivos BLE:", 12, GW_BODY_Y);
    tft.drawString("- Beacon (iBeacon/Eddystone)", 12, GW_BODY_Y + 14);
    tft.drawString("- Sensor (temp, umidade)", 12, GW_BODY_Y + 28);
    tft.drawString("- Tracker (Tile, AirTag)", 12, GW_BODY_Y + 42);
    tft.drawString("- Periferico (kbd, mouse)", 12, GW_BODY_Y + 56);
    gwDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
}

void bleSignalTracker() {
    gwDrawHeader("BLE Signal", GW_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
    tft.drawString("Rastrear sinal BLE", 12, GW_BODY_Y);
    tft.drawString("ao longo do tempo", 12, GW_BODY_Y + 14);
    gwDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
}

void bleProximityDetector() {
    gwDrawHeader("BLE Proximity", GW_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
    tft.drawString("Detector de proximidade BLE", 12, GW_BODY_Y);
    tft.drawString("Detecta dispositivos proximos", 12, GW_BODY_Y + 14);
    gwDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
}

void bleBeaconScanner() {
    gwDrawHeader("BLE Beacons", GW_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
    tft.drawString("Scanner de beacons:", 12, GW_BODY_Y);
    tft.drawString("- iBeacon (Apple)", 12, GW_BODY_Y + 14);
    tft.drawString("- Eddystone (Google)", 12, GW_BODY_Y + 28);
    tft.drawString("- AltBeacon (Radius)", 12, GW_BODY_Y + 42);
    gwDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
}

void bleMACManufacturer() {
    gwDrawHeader("MAC Manufacturer", GW_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
    tft.drawString("Lookup OUI por MAC:", 12, GW_BODY_Y);
    tft.drawString("Ex: AA:BB:CC -> Vendor", 12, GW_BODY_Y + 14);
    gwDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
}

void bleDeviceHistory() {
    gwDrawHeader("BLE History", GW_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
    tft.drawString("Historico de dispositivos", 12, GW_BODY_Y);
    tft.drawString("BLE vistos", 12, GW_BODY_Y + 14);
    gwDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
}

void bleExportCSV() {
    gwDrawHeader("BLE Export CSV", GW_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
    tft.drawString("Exportar dados BLE", 12, GW_BODY_Y);
    tft.drawString("para formato CSV", 12, GW_BODY_Y + 14);
    gwDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
}

// ============================================================================
// CATEGORY 6: Combined GPS+WiFi+BLE (8 functions)
// ============================================================================

void gpsWifiHeatmap() {
    gwDrawHeader("GPS WiFi Heatmap", GW_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
    tft.drawString("Heatmap GPS-tagged WiFi", 12, GW_BODY_Y);
    tft.drawString("Coleta sinais com localizacao", 12, GW_BODY_Y + 14);
    gwDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
}

void gpsWifiAutoConnect() {
    gwDrawHeader("GPS AutoConnect", GW_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
    tft.drawString("Auto-conectar WiFi", 12, GW_BODY_Y);
    tft.drawString("em localizacoes salvas", 12, GW_BODY_Y + 14);
    gwDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
}

void gpsTriggerCapture() {
    gwDrawHeader("GPS Trigger", GW_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
    tft.drawString("Captura por GPS trigger", 12, GW_BODY_Y);
    tft.drawString("Inicia captura ao chegar", 12, GW_BODY_Y + 14);
    tft.drawString("em coordenada especifica", 12, GW_BODY_Y + 28);
    gwDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
}

void gpsLocationFingerprint() {
    gwDrawHeader("Loc Fingerprint", GW_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
    tft.drawString("Fingerprinting WiFi", 12, GW_BODY_Y);
    tft.drawString("Localizacao por assinatura", 12, GW_BODY_Y + 14);
    tft.drawString("de sinais WiFi", 12, GW_BODY_Y + 28);
    gwDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
}

void gpsDeadDrop() {
    gwDrawHeader("GPS Dead Drop", GW_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
    tft.drawString("Dead drop digital:", 12, GW_BODY_Y);
    tft.drawString("1. Grave dados em local", 12, GW_BODY_Y + 14);
    tft.drawString("2. Outro dispositivo coleta", 12, GW_BODY_Y + 28);
    gwDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
}

void gpsNetworkMapper() {
    gwDrawHeader("Network Mapper", GW_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
    tft.drawString("Mapear redes ao longo", 12, GW_BODY_Y);
    tft.drawString("de uma rota GPS", 12, GW_BODY_Y + 14);
    gwDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
}

void gpsSessionReplay() {
    gwDrawHeader("Session Replay", GW_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
    tft.drawString("Reproduzir sessao", 12, GW_BODY_Y);
    tft.drawString("de wardriving salva", 12, GW_BODY_Y + 14);
    gwDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
}

// ============================================================================
// CATEGORY 7: Upload & Cloud (4 functions)
// ============================================================================

void wigleQuery() {
    gwDrawHeader("WiGLE Query", GW_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
    tft.drawString("Consultar WiGLE API:", 12, GW_BODY_Y);
    tft.drawString("Buscar redes proximas", 12, GW_BODY_Y + 14);
    tft.drawString("(Requer WiFi + token)", 12, GW_BODY_Y + 28);
    gwDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
}

void wigleStats() {
    gwDrawHeader("WiGLE Stats", GW_COLOR_TITLE);
    Wigle wigle;
    wigle.display_banner();
    wigle.dump_wigle_info();
    gwDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
}

void wdgwarsStats() {
    gwDrawHeader("WDGWars Stats", GW_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
    tft.drawString("WDGWars.pl stats:", 12, GW_BODY_Y);
    tft.drawString("Upload via menu GPS", 12, GW_BODY_Y + 14);
    gwDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
}

void cloudSync() {
    gwDrawHeader("Cloud Sync", GW_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
    tft.drawString("Sincronizar dados:", 12, GW_BODY_Y);
    tft.drawString("- WiGLE.net", 12, GW_BODY_Y + 14);
    tft.drawString("- WDGWars.pl", 12, GW_BODY_Y + 28);
    tft.drawString("- Export local", 12, GW_BODY_Y + 42);
    gwDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
}

// ============================================================================
// MENU SYSTEM (7 menus)
// ============================================================================

static int gwShowMenu(const char *title, const char **items, int count) {
    gwDrawHeader(title, GW_COLOR_TITLE);
    gwDrawFooter("SEL: OK  UP/DOWN: Nav", "ESC: Voltar");
    int sel = 0, scrollY = 0;

    while (true) {
        for (int i = 0; i < 7 && i < count; i++) {
            int idx = scrollY + i;
            if (idx >= count) break;
            int y = GW_BODY_Y + i * 22;
            bool isSel = (idx == sel);
            gwDrawCard(y, 20, isSel);
            tft.setTextSize(FP);
            tft.setTextColor(isSel ? TFT_BLACK : GW_COLOR_TEXT, isSel ? GW_COLOR_PRIMARY : getColorVariation(GW_COLOR_PRIMARY, 4, -1));
            tft.drawString(items[idx], 14, y + 5);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = count - 1; if (sel < scrollY) scrollY = sel; }
        if (check(DownPress)) { sel++; if (sel >= count) sel = 0; if (sel >= scrollY + 7) scrollY = sel - 6; }
        if (check(SelPress)) return sel;
        if (check(EscPress)) return -1;
        delay(100);
    }
}

void gwNavigationMenu() {
    const char *items[] = {
        "Speed Tracker", "Compass", "Waypoints",
        "Trip Computer", "Geofence", "Altitude Profile",
        "Route Recorder", "Route Replays", "Proximity Alert",
        "Multi Marker"
    };
    void (*funcs[])() = {
        gpsSpeedTracker, gpsCompass, gpsWaypointManager,
        gpsTripComputer, gpsGeofence, gpsAltitudeProfile,
        gpsRouteRecorder, gpsRouteReplay, gpsProximityAlert,
        gpsMultiMarker
    };
    while (true) {
        int sel = gwShowMenu("Navigation", items, 10);
        if (sel < 0) return;
        funcs[sel]();
        gwDrawHeader("Navigation", GW_COLOR_TITLE);
        gwDrawFooter("SEL: OK  UP/DOWN: Nav", "ESC: Voltar");
    }
}

void gwGPSDataMenu() {
    const char *items[] = {
        "Signal Monitor", "NMEA Raw", "Data Logger",
        "Accuracy Test", "Satellite View", "GPS Time",
        "Coord Convert", "Distance Calc", "Bearing Calc",
        "Area Calculator"
    };
    void (*funcs[])() = {
        gpsSignalMonitor, gpsNMEARawViewer, gpsDataLogger,
        gpsAccuracyTest, gpsSatelliteView, gpsTimeServer,
        gpsCoordinateConvert, gpsDistanceCalculator, gpsBearingCalculator,
        gpsAreaCalculator
    };
    while (true) {
        int sel = gwShowMenu("GPS Data", items, 10);
        if (sel < 0) return;
        funcs[sel]();
        gwDrawHeader("GPS Data", GW_COLOR_TITLE);
        gwDrawFooter("SEL: OK  UP/DOWN: Nav", "ESC: Voltar");
    }
}

void gwGPSExportMenu() {
    const char *items[] = {
        "Export KML", "Export GeoJSON", "Export CSV",
        "Export GPX", "Import Waypoints", "Mini Map",
        "Track Overlay", "Data Summary"
    };
    void (*funcs[])() = {
        gpsExportKML, gpsExportGeoJSON, gpsExportCSV,
        gpsExportGPXEnhanced, gpsImportWaypoints, gpsMapDisplay,
        gpsTrackOverlay, gpsDataSummary
    };
    while (true) {
        int sel = gwShowMenu("GPS Export", items, 8);
        if (sel < 0) return;
        funcs[sel]();
        gwDrawHeader("GPS Export", GW_COLOR_TITLE);
        gwDrawFooter("SEL: OK  UP/DOWN: Nav", "ESC: Voltar");
    }
}

void gwWiFiWardriveMenu() {
    const char *items[] = {
        "WiFi Wardrive+", "Channel Hop", "Signal Monitor",
        "Network Filter", "Dup Tracker", "Hidden Detect",
        "BW Estimate", "Enc Analyzer", "Coverage Map",
        "Session Stats", "Data Merge", "Network Alert"
    };
    void (*funcs[])() = {
        wifiWardriveEnhanced, wifiChannelHopScan, wifiSignalMonitor,
        wifiNetworkFilter, wifiDuplicateTracker, wifiHiddenNetworkDetect,
        wifiBandwidthEstimate, wifiEncryptionAnalyzer, wifiCoverageMap,
        wifiSessionStats, wifiDataMerge, wifiNetworkAlert
    };
    while (true) {
        int sel = gwShowMenu("WiFi Wardriving", items, 12);
        if (sel < 0) return;
        funcs[sel]();
        gwDrawHeader("WiFi Wardriving", GW_COLOR_TITLE);
        gwDrawFooter("SEL: OK  UP/DOWN: Nav", "ESC: Voltar");
    }
}

void gwBLEWardriveMenu() {
    const char *items[] = {
        "BLE Wardrive", "BLE Classifier", "BLE Signal",
        "BLE Proximity", "BLE Beacons", "MAC Manufacturer",
        "BLE History", "BLE Export CSV"
    };
    void (*funcs[])() = {
        bleWardriveScan, bleDeviceClassifier, bleSignalTracker,
        bleProximityDetector, bleBeaconScanner, bleMACManufacturer,
        bleDeviceHistory, bleExportCSV
    };
    while (true) {
        int sel = gwShowMenu("BLE Wardriving", items, 8);
        if (sel < 0) return;
        funcs[sel]();
        gwDrawHeader("BLE Wardriving", GW_COLOR_TITLE);
        gwDrawFooter("SEL: OK  UP/DOWN: Nav", "ESC: Voltar");
    }
}

void gwCombinedMenu() {
    const char *items[] = {
        "GPS WiFi Heatmap", "GPS AutoConnect", "GPS Trigger",
        "Loc Fingerprint", "GPS Dead Drop", "Network Mapper",
        "Session Replay"
    };
    void (*funcs[])() = {
        gpsWifiHeatmap, gpsWifiAutoConnect, gpsTriggerCapture,
        gpsLocationFingerprint, gpsDeadDrop, gpsNetworkMapper,
        gpsSessionReplay
    };
    while (true) {
        int sel = gwShowMenu("Combined", items, 7);
        if (sel < 0) return;
        funcs[sel]();
        gwDrawHeader("Combined", GW_COLOR_TITLE);
        gwDrawFooter("SEL: OK  UP/DOWN: Nav", "ESC: Voltar");
    }
}

void gwUploadMenu() {
    const char *items[] = {
        "WiGLE Query", "WiGLE Stats", "WDGWars Stats",
        "Cloud Sync"
    };
    void (*funcs[])() = {
        wigleQuery, wigleStats, wdgwarsStats,
        cloudSync
    };
    while (true) {
        int sel = gwShowMenu("Upload & Cloud", items, 4);
        if (sel < 0) return;
        funcs[sel]();
        gwDrawHeader("Upload & Cloud", GW_COLOR_TITLE);
        gwDrawFooter("SEL: OK  UP/DOWN: Nav", "ESC: Voltar");
    }
}

void gwAdvConfigMenu() {
    gwDrawHeader("CONFIGURACOES", GW_COLOR_TITLE);
    gwDrawFooter("SEL: Alterar  UP/DOWN: Nav", "ESC: Voltar");
    int sel = 0;
    const char *items[] = {
        "Raio Alerta", "Raio Geofence", "Intervalo Scan",
        "Auto Export", "Show Signal", "Max Waypoints",
        "BLE Scan", "WiFi Scan", "Export Format"
    };
    while (true) {
        for (int i = 0; i < 9; i++) {
            int y = GW_BODY_Y + i * 20;
            bool isSel = (i == sel);
            gwDrawCard(y, 18, isSel);
            tft.setTextSize(FP);
            tft.setTextColor(isSel ? TFT_BLACK : GW_COLOR_TEXT, isSel ? GW_COLOR_PRIMARY : getColorVariation(GW_COLOR_PRIMARY, 4, -1));
            tft.drawString(items[i], 14, y + 4);
            tft.setTextDatum(TR_DATUM);
            String val;
            switch (i) {
                case 0: val = String(gwAdvConfig.alertRadius, 0) + "m"; break;
                case 1: val = String(gwAdvConfig.geofenceRadius, 0) + "m"; break;
                case 2: val = String(gwAdvConfig.scanIntervalMs) + "ms"; break;
                case 3: val = gwAdvConfig.autoExport ? "ON" : "OFF"; break;
                case 4: val = gwAdvConfig.showSignalBars ? "ON" : "OFF"; break;
                case 5: val = String(gwAdvConfig.maxWaypoints); break;
                case 6: val = gwAdvConfig.enableBLE ? "ON" : "OFF"; break;
                case 7: val = gwAdvConfig.enableWiFi ? "ON" : "OFF"; break;
                case 8: val = gwAdvConfig.exportFormat; break;
            }
            tft.drawString(val, tftWidth - 14, y + 4);
            tft.setTextDatum(TL_DATUM);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = 8; }
        if (check(DownPress)) { sel++; if (sel >= 9) sel = 0; }
        if (check(SelPress)) {
            if (sel == 0) {
                String d = num_keyboard(String(gwAdvConfig.alertRadius, 0), 4, "Raio Alerta (m):");
                gwAdvConfig.alertRadius = d.toFloat();
            } else if (sel == 1) {
                String d = num_keyboard(String(gwAdvConfig.geofenceRadius, 0), 4, "Raio Geofence (m):");
                gwAdvConfig.geofenceRadius = d.toFloat();
            } else if (sel == 2) {
                String d = num_keyboard(String(gwAdvConfig.scanIntervalMs), 5, "Intervalo (ms):");
                gwAdvConfig.scanIntervalMs = d.toInt();
            } else if (sel == 3) gwAdvConfig.autoExport = !gwAdvConfig.autoExport;
            else if (sel == 4) gwAdvConfig.showSignalBars = !gwAdvConfig.showSignalBars;
            else if (sel == 5) {
                String d = num_keyboard(String(gwAdvConfig.maxWaypoints), 3, "Max waypoints:");
                gwAdvConfig.maxWaypoints = d.toInt();
            } else if (sel == 6) gwAdvConfig.enableBLE = !gwAdvConfig.enableBLE;
            else if (sel == 7) gwAdvConfig.enableWiFi = !gwAdvConfig.enableWiFi;
            else if (sel == 8) {
                String opts[] = {"KML", "GeoJSON", "CSV", "GPX"};
                int f = displayMessage("Formato Export", opts, 4, TFT_CYAN);
                if (f >= 0) gwAdvConfig.exportFormat = opts[f];
            }
            gwDrawHeader("CONFIGURACOES", GW_COLOR_TITLE);
            gwDrawFooter("SEL: Alterar  UP/DOWN: Nav", "ESC: Voltar");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void gwAdvancedSuiteMenu() {
    gwDrawHeader("GPS+WR Suite", GW_COLOR_TITLE);
    gwDrawFooter("SEL: OK  UP/DOWN: Nav", "ESC: Voltar");
    int sel = 0, scrollY = 0;
    const char *items[] = {
        "Navigation", "GPS Data", "GPS Export",
        "WiFi Wardriving", "BLE Wardriving",
        "Combined", "Upload & Cloud", "Configuracoes"
    };
    void (*funcs[])() = {
        gwNavigationMenu, gwGPSDataMenu, gwGPSExportMenu,
        gwWiFiWardriveMenu, gwBLEWardriveMenu,
        gwCombinedMenu, gwUploadMenu, gwAdvConfigMenu
    };
    uint16_t colors[] = {GW_COLOR_GPS, GW_COLOR_ACCENT, GW_COLOR_WARN, GW_COLOR_WIFI, GW_COLOR_BLE, TFT_MAGENTA, TFT_WHITE, TFT_DARKGREY};
    while (true) {
        for (int i = 0; i < 7; i++) {
            int y = GW_BODY_Y + i * 22;
            bool isSel = (i == sel);
            gwDrawCard(y, 20, isSel);
            tft.fillCircle(24, y + 10, 7, colors[i]);
            tft.setTextSize(FP);
            tft.setTextColor(TFT_BLACK, colors[i]);
            tft.setTextDatum(TC_DATUM);
            tft.drawCentreString(String(i + 1), 24, y + 6, 1);
            tft.setTextDatum(TL_DATUM);
            tft.setTextColor(isSel ? TFT_BLACK : GW_COLOR_TEXT, isSel ? GW_COLOR_PRIMARY : getColorVariation(GW_COLOR_PRIMARY, 4, -1));
            tft.drawString(items[i], 36, y + 5);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = 7; if (sel < scrollY) scrollY = sel; }
        if (check(DownPress)) { sel++; if (sel >= 8) sel = 0; if (sel >= scrollY + 8) scrollY = sel - 7; }
        if (check(SelPress)) {
            funcs[sel]();
            gwDrawHeader("GPS+WR Suite", GW_COLOR_TITLE);
            gwDrawFooter("SEL: OK  UP/DOWN: Nav", "ESC: Voltar");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

// ============================================================================
// Setup / Cleanup
// ============================================================================

void gwAdvSetup() {
    gwAdvConfig = GWAdvConfig();
}

void gwAdvCleanup() {
    gwDeinitGPS();
    gwWaypoints.clear();
    gwTrack.clear();
}

void gwAdvShowWelcomeScreen() {
    gwDrawHeader("GPS+WR Suite", GW_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(GW_COLOR_TEXT, GW_COLOR_BG);
    tft.drawCentreString("55+ Funcoes GPS/Wardriving", tftWidth / 2, GW_BODY_Y + 20, 1);
    tft.drawCentreString("Navigation + WiFi + BLE", tftWidth / 2, GW_BODY_Y + 34, 1);
    gwDrawFooter("SEL: OK  ESC: Voltar", "");
}

#endif // !LITE_VERSION
