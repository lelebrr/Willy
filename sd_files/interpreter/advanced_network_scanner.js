var dialog = require('dialog');
var wifi = require('wifi');
var display = require('display');
var keyboard = require('keyboard');
var storage = require('storage');

var tftWidth = display.width();
var tftHeight = display.height();

var scanResults = [];
var currentScan = null;
var isScanning = false;

function drawNetworkScanner(title) {
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

function startNetworkScan() {
    if (isScanning) return;

    isScanning = true;
    scanResults = [];

    try {
        currentScan = wifi.scan();
        console.log('Network scan started');
    } catch (error) {
        console.log(JSON.stringify(error));
        isScanning = false;
    }
}

function updateScanResults() {
    if (!isScanning || !currentScan) return;

    try {
        var results = currentScan.getResults();
        if (results && results.length > 0) {
            scanResults = results;
            isScanning = false;
            currentScan = null;
            console.log('Network scan completed');
        }
    } catch (error) {
        console.log(JSON.stringify(error));
    }
}

function drawScanResults() {
    display.setTextSize(1);
    display.setTextAlign('left', 'top');

    var y = 40;
    for (var i = 0; i < scanResults.length && i < 10; i++) {
        var network = scanResults[i];
        var ssid = network.ssid || 'Unknown';
        var bssid = network.bssid || 'Unknown';
        var signal = network.signal || 0;
        var security = network.security || 'Open';

        display.drawText(ssid, 10, y);
        display.drawText(bssid, 10, y + 10);
        display.drawText(signal + 'dBm', 10, y + 20);
        display.drawText(security, 10, y + 30);

        y += 50;
    }
}

function saveScanResults() {
    try {
        storage.write('network_scan_results', JSON.stringify(scanResults), 'write');
        console.log('Scan results saved successfully');
    } catch (error) {
        console.log(JSON.stringify(error));
    }
}

function loadScanResults() {
    try {
        var data = storage.read('network_scan_results');
        if (data) {
            scanResults = JSON.parse(data);
            console.log('Scan results loaded successfully');
        }
    } catch (error) {
        console.log(JSON.stringify(error));
    }
}

function main() {
    loadScanResults();

    var running = true;

    while (running) {
        updateScanResults();

        drawNetworkScanner('Network Scanner');

        if (isScanning) {
            display.drawText('Scanning...', 10, 40);
        } else {
            drawScanResults();
        }

        if (keyboard.getSelPress()) {
            var choices = ['Start Scan', 'Save Results', 'Load Results', 'Clear Results', 'Quit'];
            var choice = dialog.choice(choices);

            if (choice === 'Quit') {
                running = false;
            } else if (choice === 'Start Scan') {
                startNetworkScan();
            } else if (choice === 'Save Results') {
                saveScanResults();
            } else if (choice === 'Load Results') {
                loadScanResults();
            } else if (choice === 'Clear Results') {
                scanResults = [];
            }
        }

        display.draw();
        delay(100);
    }
}

main();
