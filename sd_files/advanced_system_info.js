var dialog = require('dialog');
var wifi = require('wifi');
var display = require('display');
var keyboard = require('keyboard');
var storage = require('storage');

var tftWidth = display.width();
var tftHeight = display.height();

var systemInfo = {
    device: 'Willy Device',
    version: '1.0.0',
    firmware: '2.1.0',
    serial: 'WD-' + Math.floor(Math.random() * 1000000),
    mac: 'AA:BB:CC:DD:EE:FF',
    ip: '192.168.1.100',
    uptime: 0,
    memory: {
        total: 0,
        used: 0,
        free: 0
    },
    storage: {
        total: 0,
        used: 0,
        free: 0
    },
    battery: {
        level: 100,
        charging: false,
        voltage: 4.2,
        temperature: 25
    },
    sensors: {
        temperature: 25,
        humidity: 50,
        pressure: 1013,
        light: 500
    }
};

var historyData = [];
var maxHistoryPoints = 50;

function drawSystemInfo(title) {
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

function updateSystemInfo() {
    try {
        systemInfo.uptime = Date.now() / 1000;
        systemInfo.memory.used = Math.random() * 1000000;
        systemInfo.memory.free = 2000000 - systemInfo.memory.used;
        systemInfo.memory.total = 2000000;
        systemInfo.storage.used = Math.random() * 10000000;
        systemInfo.storage.free = 50000000 - systemInfo.storage.used;
        systemInfo.storage.total = 50000000;
        systemInfo.battery.level = Math.max(0, systemInfo.battery.level - Math.random() * 0.1);
        systemInfo.sensors.temperature = 20 + Math.random() * 20;
        systemInfo.sensors.humidity = 30 + Math.random() * 40;
        systemInfo.sensors.pressure = 1000 + Math.random() * 50;
        systemInfo.sensors.light = Math.random() * 1000;

        historyData.push({
            timestamp: Date.now(),
            memory: systemInfo.memory.used,
            storage: systemInfo.storage.used,
            battery: systemInfo.battery.level,
            temperature: systemInfo.sensors.temperature
        });

        if (historyData.length > maxHistoryPoints) {
            historyData.shift();
        }
    } catch (error) {
        console.log(JSON.stringify(error));
    }
}

function drawDeviceInfo() {
    display.setTextSize(1);
    display.setTextAlign('left', 'top');

    var y = 40;
    display.drawText('Device: ' + systemInfo.device, 10, y);
    y += 20;
    display.drawText('Version: ' + systemInfo.version, 10, y);
    y += 20;
    display.drawText('Firmware: ' + systemInfo.firmware, 10, y);
    y += 20;
    display.drawText('Serial: ' + systemInfo.serial, 10, y);
    y += 20;
    display.drawText('MAC: ' + systemInfo.mac, 10, y);
    y += 20;
    display.drawText('IP: ' + systemInfo.ip, 10, y);
    y += 20;
    display.drawText('Uptime: ' + Math.floor(systemInfo.uptime) + 's', 10, y);
}

function drawMemoryInfo() {
    display.setTextSize(1);
    display.setTextAlign('left', 'top');

    var y = 40;
    display.drawText('Memory:', 10, y);
    y += 20;
    display.drawText('Total: ' + systemInfo.memory.total + ' bytes', 10, y);
    y += 20;
    display.drawText('Used: ' + systemInfo.memory.used + ' bytes', 10, y);
    y += 20;
    display.drawText('Free: ' + systemInfo.memory.free + ' bytes', 10, y);
}

function drawStorageInfo() {
    display.setTextSize(1);
    display.setTextAlign('left', 'top');

    var y = 40;
    display.drawText('Storage:', 10, y);
    y += 20;
    display.drawText('Total: ' + systemInfo.storage.total + ' bytes', 10, y);
    y += 20;
    display.drawText('Used: ' + systemInfo.storage.used + ' bytes', 10, y);
    y += 20;
    display.drawText('Free: ' + systemInfo.storage.free + ' bytes', 10, y);
}

function drawBatteryInfo() {
    display.setTextSize(1);
    display.setTextAlign('left', 'top');

    var y = 40;
    display.drawText('Battery:', 10, y);
    y += 20;
    display.drawText('Level: ' + systemInfo.battery.level.toFixed(1) + '%', 10, y);
    y += 20;
    display.drawText('Charging: ' + (systemInfo.battery.charging ? 'Yes' : 'No'), 10, y);
    y += 20;
    display.drawText('Voltage: ' + systemInfo.battery.voltage + 'V', 10, y);
    y += 20;
    display.drawText('Temperature: ' + systemInfo.battery.temperature + '°C', 10, y);
}

function drawSensorInfo() {
    display.setTextSize(1);
    display.setTextAlign('left', 'top');

    var y = 40;
    display.drawText('Sensors:', 10, y);
    y += 20;
    display.drawText('Temperature: ' + systemInfo.sensors.temperature.toFixed(1) + '°C', 10, y);
    y += 20;
    display.drawText('Humidity: ' + systemInfo.sensors.humidity.toFixed(1) + '%', 10, y);
    y += 20;
    display.drawText('Pressure: ' + systemInfo.sensors.pressure.toFixed(1) + 'hPa', 10, y);
    y += 20;
    display.drawText('Light: ' + systemInfo.sensors.light.toFixed(0) + 'lux', 10, y);
}

function drawCharts() {
    if (historyData.length < 2) return;

    var chartWidth = tftWidth - 20;
    var chartHeight = 60;
    var chartX = 10;
    var chartY = tftHeight - chartHeight - 10;

    display.drawRect(chartX, chartY, chartWidth, chartHeight);

    var colors = [WILLY_PRICOLOR, 0x00FF00, 0x0000FF, 0xFFFF00];
    var dataTypes = ['memory', 'storage', 'battery', 'temperature'];

    for (var i = 0; i < dataTypes.length; i++) {
        var color = colors[i];
        var points = [];

        for (var j = 0; j < historyData.length; j++) {
            var x = chartX + (j * chartWidth) / historyData.length;
            var y = chartY + chartHeight - (historyData[j][dataTypes[i]] * chartHeight) / 100;
            points.push({ x: x, y: y });
        }

        display.setColor(color);
        for (var j = 1; j < points.length; j++) {
            display.drawLine(points[j - 1].x, points[j - 1].y, points[j].x, points[j].y);
        }
    }

    display.setColor(WILLY_PRICOLOR);
}

function saveSystemInfo() {
    try {
        storage.write('system_info_data', JSON.stringify(systemInfo), 'write');
        console.log('System info saved successfully');
    } catch (error) {
        console.log(JSON.stringify(error));
    }
}

function loadSystemInfo() {
    try {
        var data = storage.read('system_info_data');
        if (data) {
            systemInfo = JSON.parse(data);
            console.log('System info loaded successfully');
        }
    } catch (error) {
        console.log(JSON.stringify(error));
    }
}

function main() {
    loadSystemInfo();

    var running = true;
    var viewMode = 'device';
    var updateInterval = 1000;
    var lastUpdate = 0;

    while (running) {
        var currentTime = Date.now();

        if (currentTime - lastUpdate >= updateInterval) {
            updateSystemInfo();
            lastUpdate = currentTime;
        }

        drawSystemInfo('System Information');

        if (viewMode === 'device') {
            drawDeviceInfo();
        } else if (viewMode === 'memory') {
            drawMemoryInfo();
        } else if (viewMode === 'storage') {
            drawStorageInfo();
        } else if (viewMode === 'battery') {
            drawBatteryInfo();
        } else if (viewMode === 'sensors') {
            drawSensorInfo();
        }

        drawCharts();

        if (keyboard.getSelPress()) {
            var choices = ['Device Info', 'Memory', 'Storage', 'Battery', 'Sensors', 'Save Info', 'Quit'];
            var choice = dialog.choice(choices);

            if (choice === 'Quit') {
                running = false;
            } else if (choice === 'Device Info') {
                viewMode = 'device';
            } else if (choice === 'Memory') {
                viewMode = 'memory';
            } else if (choice === 'Storage') {
                viewMode = 'storage';
            } else if (choice === 'Battery') {
                viewMode = 'battery';
            } else if (choice === 'Sensors') {
                viewMode = 'sensors';
            } else if (choice === 'Save Info') {
                saveSystemInfo();
            }
        }

        display.draw();
        delay(100);
    }
}

main();
