var dialog = require('dialog');
var wifi = require('wifi');
var display = require('display');
var keyboard = require('keyboard');
var storage = require('storage');

var tftWidth = display.width();
var tftHeight = display.height();

var bluetoothDevices = [];
var connectedDevices = [];
var isScanning = false;
var isAdvertising = false;
var bluetoothServices = [];
var currentDevice = null;
var deviceHistory = [];
var maxHistoryPoints = 50;

function drawBluetoothController(title) {
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

function drawBluetoothDevices() {
    display.setTextSize(1);
    display.setTextAlign('left', 'top');

    var y = 40;
    display.drawText('Status: ' + (isScanning ? 'Scanning' : 'Idle'), 10, y);
    y += 20;
    display.drawText('Devices Found: ' + bluetoothDevices.length, 10, y);
    y += 20;
    display.drawText('Connected: ' + connectedDevices.length, 10, y);
    y += 20;

    for (var i = 0; i < bluetoothDevices.length && i < 8; i++) {
        var device = bluetoothDevices[i];
        var name = device.name || 'Unknown';
        var address = device.address || 'Unknown';
        var rssi = device.rssi || 0;
        var connected = connectedDevices.includes(address);

        display.drawText((connected ? '✓' : '○') + ' ' + name, 10, y);
        display.drawText(address + ' (RSSI: ' + rssi + ')', 10, y + 10);

        y += 30;
    }
}

function drawConnectedDevices() {
    display.setTextSize(1);
    display.setTextAlign('left', 'top');

    var y = 40;
    display.drawText('Connected Devices:', 10, y);
    y += 20;

    for (var i = 0; i < connectedDevices.length && i < 6; i++) {
        var address = connectedDevices[i];
        var device = bluetoothDevices.find(d => d.address === address);
        var name = device ? device.name : 'Unknown';

        display.drawText(name + ' - ' + address, 10, y);
        y += 20;
    }
}

function drawDeviceServices() {
    if (!currentDevice) return;

    display.setTextSize(1);
    display.setTextAlign('left', 'top');

    var y = 40;
    display.drawText('Device: ' + (currentDevice.name || 'Unknown'), 10, y);
    y += 20;
    display.drawText('Address: ' + currentDevice.address, 10, y);
    y += 20;
    display.drawText('Services: ' + bluetoothServices.length, 10, y);
    y += 20;

    for (var i = 0; i < bluetoothServices.length && i < 8; i++) {
        var service = bluetoothServices[i];
        display.drawText(service.uuid + ' - ' + service.name, 10, y);
        y += 15;
    }
}

function startScanning() {
    isScanning = true;
    bluetoothDevices = [];
    console.log('Starting Bluetooth scan...');

    setTimeout(() => {
        isScanning = false;
        console.log('Bluetooth scan completed');
    }, 10000);
}

function stopScanning() {
    isScanning = false;
    console.log('Bluetooth scan stopped');
}

function connectToDevice(address) {
    if (!connectedDevices.includes(address)) {
        connectedDevices.push(address);
        console.log('Connected to device: ' + address);
    }
}

function disconnectFromDevice(address) {
    var index = connectedDevices.indexOf(address);
    if (index !== -1) {
        connectedDevices.splice(index, 1);
        console.log('Disconnected from device: ' + address);
    }
}

function discoverServices(address) {
    currentDevice = bluetoothDevices.find(d => d.address === address);
    if (currentDevice) {
        bluetoothServices = [
            { uuid: '1800', name: 'Generic Access' },
            { uuid: '1801', name: 'Generic Attribute' },
            { uuid: '180A', name: 'Device Information' },
            { uuid: '180F', name: 'Battery Service' },
            { uuid: '1812', name: 'Human Interface Device' },
            { uuid: '1813', name: 'Scan Parameters' },
            { uuid: '1819', name: 'Location and Navigation' }
        ];
        console.log('Services discovered for device: ' + address);
    }
}

function sendData(address, data) {
    console.log('Sending data to ' + address + ': ' + data);
}

function sendDataToAll(data) {
    for (var i = 0; i < connectedDevices.length; i++) {
        sendData(connectedDevices[i], data);
    }
}

function startAdvertising() {
    isAdvertising = true;
    console.log('Starting Bluetooth advertising...');
}

function stopAdvertising() {
    isAdvertising = false;
    console.log('Stopping Bluetooth advertising...');
}

function saveBluetoothConfig() {
    try {
        var config = {
            devices: bluetoothDevices,
            connected: connectedDevices,
            services: bluetoothServices,
            history: deviceHistory
        };

        storage.write('bluetooth_config', JSON.stringify(config), 'write');
        console.log('Bluetooth configuration saved');
    } catch (error) {
        console.log(JSON.stringify(error));
    }
}

function loadBluetoothConfig() {
    try {
        var data = storage.read('bluetooth_config');
        if (data) {
            var config = JSON.parse(data);
            bluetoothDevices = config.devices || [];
            connectedDevices = config.connected || [];
            bluetoothServices = config.services || [];
            deviceHistory = config.history || [];

            console.log('Bluetooth configuration loaded');
        }
    } catch (error) {
        console.log(JSON.stringify(error));
    }
}

function exportDeviceList() {
    try {
        var csv = 'Name,Address,RSSI,Connected\n';
        for (var i = 0; i < bluetoothDevices.length; i++) {
            var device = bluetoothDevices[i];
            var connected = connectedDevices.includes(device.address) ? 'Yes' : 'No';
            csv += (device.name || 'Unknown') + ',' + (device.address || 'Unknown') + ',' + (device.rssi || 0) + ',' + connected + '\n';
        }

        storage.write('bluetooth_devices.csv', csv, 'write');
        console.log('Bluetooth device list exported');
    } catch (error) {
        console.log(JSON.stringify(error));
    }
}

function main() {
    loadBluetoothConfig();

    var running = true;
    var viewMode = 'devices';

    while (running) {
        drawBluetoothController('Bluetooth Controller');

        if (viewMode === 'devices') {
            drawBluetoothDevices();
        } else if (viewMode === 'connected') {
            drawConnectedDevices();
        } else if (viewMode === 'services') {
            drawDeviceServices();
        }

        if (isScanning) {
            display.drawText('Scanning...', 10, tftHeight - 30);
        }

        if (keyboard.getSelPress()) {
            var choices = [];

            if (viewMode === 'devices') {
                choices = ['Start Scan', 'Stop Scan', 'View Connected', 'View Services', 'Export List', 'Save Config', 'Load Config', 'Quit'];
            } else if (viewMode === 'connected') {
                choices = ['Back to Devices', 'Disconnect All', 'Send Message', 'Quit'];
            } else if (viewMode === 'services') {
                choices = ['Back to Devices', 'Read Characteristic', 'Write Characteristic', 'Quit'];
            }

            var choice = dialog.choice(choices);

            if (choice === 'Quit') {
                running = false;
            } else if (choice === 'Start Scan' && viewMode === 'devices') {
                startScanning();
            } else if (choice === 'Stop Scan' && viewMode === 'devices') {
                stopScanning();
            } else if (choice === 'View Connected' && viewMode === 'devices') {
                viewMode = 'connected';
            } else if (choice === 'View Services' && viewMode === 'devices') {
                if (bluetoothDevices.length > 0) {
                    discoverServices(bluetoothDevices[0].address);
                    viewMode = 'services';
                }
            } else if (choice === 'Export List' && viewMode === 'devices') {
                exportDeviceList();
            } else if (choice === 'Save Config' && viewMode === 'devices') {
                saveBluetoothConfig();
            } else if (choice === 'Load Config' && viewMode === 'devices') {
                loadBluetoothConfig();
            } else if (choice === 'Back to Devices' && (viewMode === 'connected' || viewMode === 'services')) {
                viewMode = 'devices';
            } else if (choice === 'Disconnect All' && viewMode === 'connected') {
                connectedDevices = [];
                console.log('All devices disconnected');
            } else if (choice === 'Send Message' && viewMode === 'connected') {
                var message = prompt('Enter message to send:');
                if (message) {
                    sendDataToAll(message);
                }
            } else if (choice === 'Read Characteristic' && viewMode === 'services') {
                console.log('Reading characteristic...');
            } else if (choice === 'Write Characteristic' && viewMode === 'services') {
                var value = prompt('Enter value to write:');
                if (value) {
                    console.log('Writing value: ' + value);
                }
            }
        }

        display.draw();
        delay(100);
    }
}

main();
