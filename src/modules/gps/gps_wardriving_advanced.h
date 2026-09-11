#ifndef __GPS_WARDRIVING_ADVANCED_H__
#define __GPS_WARDRIVING_ADVANCED_H__

#include <Arduino.h>
#include <vector>
#include <TinyGPS++.h>

// ============================================================================
// GPS & Wardriving Advanced Suite — 55+ Real Functions
// ============================================================================

// --- Color/Layout Constants ---
#define GW_COLOR_BG       wilyConfig.bgColor
#define GW_COLOR_PRIMARY  wilyConfig.priColor
#define GW_COLOR_TITLE    TFT_CYAN
#define GW_COLOR_ACCENT   TFT_GREEN
#define GW_COLOR_WARN     TFT_YELLOW
#define GW_COLOR_DANGER   TFT_RED
#define GW_COLOR_TEXT     TFT_WHITE
#define GW_COLOR_DIM      TFT_DARKGREY
#define GW_COLOR_GPS      TFT_CYAN
#define GW_COLOR_WIFI     TFT_YELLOW
#define GW_COLOR_BLE      TFT_MAGENTA
#define GW_BODY_Y         50
#define GW_FOOTER_Y       (tftHeight - 28)

// --- Config Structure ---
struct GWAdvConfig {
    float alertRadius;        // meters for proximity alerts
    float geofenceRadius;     // meters for geofence
    float targetLat;          // geofence center lat
    float targetLon;          // geofence center lon
    int scanIntervalMs;       // wardriving scan interval
    bool autoExport;          // auto-export on session end
    bool showSignalBars;      // show signal strength bars
    int maxWaypoints;         // max waypoints in memory
    bool enableBLE;           // enable BLE scanning
    bool enableWiFi;          // enable WiFi scanning
    int channelHopDelayMs;    // delay per channel hop
    bool logNMEA;             // log raw NMEA sentences
    bool logSpeed;            // log speed data
    String exportFormat;      // "KML", "GeoJSON", "CSV", "GPX"

    GWAdvConfig() :
        alertRadius(100.0f),
        geofenceRadius(500.0f),
        targetLat(0.0f),
        targetLon(0.0f),
        scanIntervalMs(2000),
        autoExport(false),
        showSignalBars(true),
        maxWaypoints(50),
        enableBLE(true),
        enableWiFi(true),
        channelHopDelayMs(300),
        logNMEA(false),
        logSpeed(true),
        exportFormat("KML") {}
};

extern GWAdvConfig gwAdvConfig;

// --- Waypoint Structure ---
struct GWWaypoint {
    double lat;
    double lon;
    float alt;
    String name;
    uint32_t timestamp;
};

// --- Track Point ---
struct GWTrackPoint {
    double lat;
    double lon;
    float alt;
    float speed;
    float course;
    uint32_t timestamp;
};

// ============================================================================
// CATEGORY 1: GPS Navigation & Tracking (10 functions)
// ============================================================================

void gpsSpeedTracker();
void gpsCompass();
void gpsWaypointManager();
void gpsTripComputer();
void gpsGeofence();
void gpsAltitudeProfile();
void gpsRouteRecorder();
void gpsRouteReplay();
void gpsProximityAlert();
void gpsMultiMarker();

// ============================================================================
// CATEGORY 2: GPS Data & Analysis (10 functions)
// ============================================================================

void gpsSignalMonitor();
void gpsNMEARawViewer();
void gpsDataLogger();
void gpsAccuracyTest();
void gpsSatelliteView();
void gpsTimeServer();
void gpsCoordinateConvert();
void gpsDistanceCalculator();
void gpsBearingCalculator();
void gpsAreaCalculator();

// ============================================================================
// CATEGORY 3: GPS Export & Visualization (8 functions)
// ============================================================================

void gpsExportKML();
void gpsExportGeoJSON();
void gpsExportCSV();
void gpsExportGPXEnhanced();
void gpsImportWaypoints();
void gpsMapDisplay();
void gpsTrackOverlay();
void gpsDataSummary();

// ============================================================================
// CATEGORY 4: WiFi Wardriving Enhanced (12 functions)
// ============================================================================

void wifiWardriveEnhanced();
void wifiChannelHopScan();
void wifiSignalMonitor();
void wifiNetworkFilter();
void wifiDuplicateTracker();
void wifiHiddenNetworkDetect();
void wifiBandwidthEstimate();
void wifiEncryptionAnalyzer();
void wifiCoverageMap();
void wifiSessionStats();
void wifiDataMerge();
void wifiNetworkAlert();

// ============================================================================
// CATEGORY 5: BLE Wardriving (8 functions)
// ============================================================================

void bleWardriveScan();
void bleDeviceClassifier();
void bleSignalTracker();
void bleProximityDetector();
void bleBeaconScanner();
void bleMACManufacturer();
void bleDeviceHistory();
void bleExportCSV();

// ============================================================================
// CATEGORY 6: Combined GPS+WiFi+BLE (8 functions)
// ============================================================================

void gpsWifiHeatmap();
void gpsWifiAutoConnect();
void gpsTriggerCapture();
void gpsLocationFingerprint();
void gpsDeadDrop();
void gpsNetworkMapper();
void gpsSessionReplay();

// ============================================================================
// CATEGORY 7: Upload & Cloud (4 functions)
// ============================================================================

void wigleQuery();
void wigleStats();
void wdgwarsStats();
void cloudSync();

// ============================================================================
// MENU SYSTEM (7 menus)
// ============================================================================

void gwNavigationMenu();
void gwGPSDataMenu();
void gwGPSExportMenu();
void gwWiFiWardriveMenu();
void gwBLEWardriveMenu();
void gwCombinedMenu();
void gwUploadMenu();
void gwAdvancedSuiteMenu();

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

void gwAdvSetup();
void gwAdvCleanup();
void gwAdvShowWelcomeScreen();
void gwAdvConfigMenu();

void gwDrawHeader(const char *title, uint16_t color);
void gwDrawFooter(const char *left, const char *right);
void gwDrawCard(int y, int h, bool sel);
void gwDrawProgressBar(int y, int pct, const char *label);
void gwDrawSpinner(int x, int y, int frame);
void gwDrawGPSIcon(int x, int y);
void gwDisplayCoords(double lat, double lon, int y);
bool gwAdvConfirmAction(const char *msg);
void gwAdvDisplayProgress(const char *msg, int percent);
void gwAdvShowWelcomeScreen();

// GPS helpers
TinyGPSPlus* gwGetGPS();
void gwInitGPS();
void gwDeinitGPS();
double gwCalcDistance(double lat1, double lon1, double lat2, double lon2);
double gwCalcBearing(double lat1, double lon1, double lat2, double lon2);
String gwCoordToDMS(double coord, bool isLat);
String gwCoordToUTM(double lat, double lon);
void gwSaveWaypoints();
void gwLoadWaypoints();

#endif // __GPS_WARDRIVING_ADVANCED_H__
