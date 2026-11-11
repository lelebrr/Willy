var dialog = require('dialog');
var wifi = require('wifi');
var display = require('display');
var keyboard = require('keyboard');
var storage = require('storage');

var tftWidth = display.width();
var tftHeight = display.height();

var gpsData = {
    latitude: 0.0,
    longitude: 0.0,
    altitude: 0.0,
    speed: 0.0,
    satellites: 0,
    fix: false,
    hdop: 0.0,
    vdop: 0.0,
    timestamp: new Date().toISOString()
};

var gpsHistory = [];
var maxHistoryPoints = 100;
var waypoints = [];
var routes = [];
var currentRoute = null;
var isRecording = false;
var isNavigating = false;

function drawGPSController(title) {
    display.fill(0);
    display.drawRoundRect(
        5,
        5,
        tftWidth - 10,
        tftHeight - 10,
        5,
        WILLY_PRICOLOR
    );
    display.setTextSize(2);
    display.setTextAlign('center', 'top');
    display.drawText(
        title.length > 20 ? title.substring(0, 20) : title,
        tftWidth / 2,
        5
    );
    display.setTextAlign('left', 'top');
}

function drawGPSData() {
    display.setTextSize(1);
    display.setTextAlign('left', 'top');

    var y = 40;
    display.drawText('GPS Status: ' + (gpsData.fix ? 'FIX' : 'NO FIX'), 10, y);
    y += 20;
    display.drawText('Satellites: ' + gpsData.satellites, 10, y);
    y += 20;
    display.drawText('HDOP: ' + gpsData.hdop.toFixed(2), 10, y);
    y += 20;

    if (gpsData.fix) {
        display.drawText('Latitude: ' + gpsData.latitude.toFixed(6), 10, y);
        y += 15;
        display.drawText('Longitude: ' + gpsData.longitude.toFixed(6), 10, y);
        y += 15;
        display.drawText('Altitude: ' + gpsData.altitude.toFixed(2) + 'm', 10, y);
        y += 15;
        display.drawText('Speed: ' + gpsData.speed.toFixed(2) + ' km/h', 10, y);
        y += 15;
        display.drawText('Time: ' + new Date(gpsData.timestamp).toLocaleTimeString(), 10, y);
    }

    if (isRecording) {
        display.drawText('Recording...', 10, tftHeight - 30);
    }

    if (isNavigating) {
        display.drawText('Navigating...', 10, tftHeight - 30);
    }
}

function drawGPSMap() {
    if (!gpsData.fix) return;

    var mapWidth = tftWidth - 40;
    var mapHeight = tftHeight - 100;
    var mapX = 20;
    var mapY = 40;

    display.drawRect(mapX, mapY, mapWidth, mapHeight);

    var centerX = mapX + mapWidth / 2;
    var centerY = mapY + mapHeight / 2;

    if (gpsData.fix) {
        var x = centerX + (gpsData.longitude * mapWidth) / 360;
        var y = centerY - (gpsData.latitude * mapHeight) / 180;

        display.setColor(0x00FF00);
        display.fillCircle(x, y, 5);
        display.setColor(WILLY_PRICOLOR);
    }

    if (waypoints.length > 0) {
        display.setColor(0xFF0000);
        for (var i = 0; i < waypoints.length; i++) {
            var wp = waypoints[i];
            var wx = centerX + (wp.longitude * mapWidth) / 360;
            var wy = centerY - (wp.latitude * mapHeight) / 180;
            display.fillCircle(wx, wy, 3);
        }
        display.setColor(WILLY_PRICOLOR);
    }

    if (gpsHistory.length > 1) {
        display.setColor(0x0000FF);
        for (var i = 1; i < gpsHistory.length; i++) {
            var prev = gpsHistory[i - 1];
            var curr = gpsHistory[i];

            var px = centerX + (prev.longitude * mapWidth) / 360;
            var py = centerY - (prev.latitude * mapHeight) / 180;
            var cx = centerX + (curr.longitude * mapWidth) / 360;
            var cy = centerY - (curr.latitude * mapHeight) / 180;

            display.drawLine(px, py, cx, cy);
        }
        display.setColor(WILLY_PRICOLOR);
    }
}

function updateGPSData() {
    try {
        gpsData.latitude = -22.9068 + (Math.random() - 0.5) * 0.01;
        gpsData.longitude = -43.1729 + (Math.random() - 0.5) * 0.01;
        gpsData.altitude = Math.random() * 100;
        gpsData.speed = Math.random() * 100;
        gpsData.satellites = Math.floor(Math.random() * 12) + 1;
        gpsData.fix = gpsData.satellites > 3;
        gpsData.hdop = Math.random() * 2 + 0.5;
        gpsData.vdop = Math.random() * 2 + 0.5;
        gpsData.timestamp = new Date().toISOString();

        if (gpsData.fix) {
            gpsHistory.push({
                latitude: gpsData.latitude,
                longitude: gpsData.longitude,
                timestamp: gpsData.timestamp
            });

            if (gpsHistory.length > maxHistoryPoints) {
                gpsHistory.shift();
            }
        }
    } catch (error) {
        console.log(JSON.stringify(error));
    }
}

function addWaypoint() {
    var waypoint = {
        latitude: gpsData.latitude,
        longitude: gpsData.longitude,
        altitude: gpsData.altitude,
        timestamp: new Date().toISOString(),
        name: 'WP-' + (waypoints.length + 1)
    };

    waypoints.push(waypoint);
    console.log('Waypoint added: ' + waypoint.name);
}

function clearWaypoints() {
    waypoints = [];
    console.log('Waypoints cleared');
}

function startRecording() {
    isRecording = true;
    gpsHistory = [];
    console.log('GPS recording started');
}

function stopRecording() {
    isRecording = false;
    console.log('GPS recording stopped');
}

function saveRoute() {
    if (gpsHistory.length > 0) {
        var route = {
            name: 'Route-' + (routes.length + 1),
            waypoints: gpsHistory,
            timestamp: new Date().toISOString()
        };

        routes.push(route);
        console.log('Route saved: ' + route.name);
    }
}

function loadRoute(index) {
    if (index >= 0 && index < routes.length) {
        currentRoute = routes[index];
        console.log('Route loaded: ' + currentRoute.name);
    }
}

function startNavigation() {
    if (waypoints.length > 0) {
        isNavigating = true;
        console.log('Navigation started');
    }
}

function stopNavigation() {
    isNavigating = false;
    console.log('Navigation stopped');
}

function calculateDistance(lat1, lon1, lat2, lon2) {
    var R = 6371;
    var dLat = (lat2 - lat1) * Math.PI / 180;
    var dLon = (lon2 - lon1) * Math.PI / 180;
    var a = Math.sin(dLat / 2) * Math.sin(dLat / 2) +
        Math.cos(lat1 * Math.PI / 180) * Math.cos(lat2 * Math.PI / 180) *
        Math.sin(dLon / 2) * Math.sin(dLon / 2);
    var c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1 - a));
    return R * c;
}

function exportGPSData() {
    try {
        var csv = 'Timestamp,Latitude,Longitude,Altitude,Speed,Satellites,HDOP\n';
        for (var i = 0; i < gpsHistory.length; i++) {
            var data = gpsHistory[i];
            csv += data.timestamp + ',' + data.latitude + ',' + data.longitude + ',' +
                gpsData.altitude + ',' + gpsData.speed + ',' + gpsData.satellites + ',' +
                gpsData.hdop + '\n';
        }

        storage.write('gps_data_export.csv', csv, 'write');
        console.log('GPS data exported successfully');
    } catch (error) {
        console.log(JSON.stringify(error));
    }
}

function saveGPSConfig() {
    try {
        var config = {
            gpsData: gpsData,
            history: gpsHistory,
            waypoints: waypoints,
            routes: routes
        };

        storage.write('gps_config', JSON.stringify(config), 'write');
        console.log('GPS configuration saved');
    } catch (error) {
        console.log(JSON.stringify(error));
    }
}

function loadGPSConfig() {
    try {
        var data = storage.read('gps_config');
        if (data) {
            var config = JSON.parse(data);
            gpsData = config.gpsData || gpsData;
            gpsHistory = config.history || [];
            waypoints = config.waypoints || [];
            routes = config.routes || [];

            console.log('GPS configuration loaded');
        }
    } catch (error) {
        console.log(JSON.stringify(error));
    }
}

function main() {
    loadGPSConfig();

    var running = true;
    var updateInterval = 1000;
    var lastUpdate = 0;
    var viewMode = 'data';

    while (running) {
        var currentTime = Date.now();

        if (currentTime - lastUpdate >= updateInterval) {
            updateGPSData();
            lastUpdate = currentTime;
        }

        drawGPSController('GPS Controller');

        if (viewMode === 'data') {
            drawGPSData();
        } else if (viewMode === 'map') {
            drawGPSMap();
        }

        if (keyboard.getSelPress()) {
            var choices = [];

            if (viewMode === 'data') {
                choices = ['View Map', 'Add Waypoint', 'Clear Waypoints', 'Start Recording', 'Stop Recording', 'Save Route', 'Load Route', 'Start Navigation', 'Stop Navigation', 'Export Data', 'Save Config', 'Load Config', 'Quit'];
            } else if (viewMode === 'map') {
                choices = ['View Data', 'Add Waypoint', 'Clear Waypoints', 'Start Navigation', 'Stop Navigation', 'Quit'];
            }

            var choice = dialog.choice(choices);

            if (choice === 'Quit') {
                running = false;
            } else if (choice === 'View Map' && viewMode === 'data') {
                viewMode = 'map';
            } else if (choice === 'View Data' && viewMode === 'map') {
                viewMode = 'data';
            } else if (choice === 'Add Waypoint') {
                addWaypoint();
            } else if (choice === 'Clear Waypoints') {
                clearWaypoints();
            } else if (choice === 'Start Recording' && viewMode === 'data') {
                startRecording();
            } else if (choice === 'Stop Recording' && viewMode === 'data') {
                stopRecording();
            } else if (choice === 'Save Route' && viewMode === 'data') {
                saveRoute();
            } else if (choice === 'Load Route' && viewMode === 'data') {
                var routeIndex = parseInt(prompt('Enter route index:'));
                loadRoute(routeIndex);
            } else if (choice === 'Start Navigation') {
                startNavigation();
            } else if (choice === 'Stop Navigation') {
                stopNavigation();
            } else if (choice === 'Export Data' && viewMode === 'data') {
                exportGPSData();
            } else if (choice === 'Save Config' && viewMode === 'data') {
                saveGPSConfig();
            } else if (choice === 'Load Config' && viewMode === 'data') {
                loadGPSConfig();
            }
        }

        display.draw();
        delay(100);
    }
}

main();
