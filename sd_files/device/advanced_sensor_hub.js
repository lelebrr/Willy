var dialog = require('dialog');
var wifi = require('wifi');
var display = require('display');
var keyboard = require('keyboard');
var storage = require('storage');

var tftWidth = display.width();
var tftHeight = display.height();

var sensors = {
    temperature: { pin: 34, value: 0, unit: '°C', min: -40, max: 125 },
    humidity: { pin: 35, value: 0, unit: '%', min: 0, max: 100 },
    pressure: { pin: 32, value: 0, unit: 'hPa', min: 300, max: 1100 },
    light: { pin: 33, value: 0, unit: 'lux', min: 0, max: 65535 },
    distance: { pin: 25, value: 0, unit: 'cm', min: 2, max: 400 },
    motion: { pin: 26, value: 0, unit: '', min: 0, max: 1 },
    gas: { pin: 27, value: 0, unit: 'ppm', min: 0, max: 1000 },
    sound: { pin: 12, value: 0, unit: 'dB', min: 30, max: 120 }
};

var sensorHistory = {};
var maxHistoryPoints = 100;
var isLogging = false;
var logInterval = 1000;
var lastLogTime = 0;

function drawSensorHub(title) {
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

function readSensors() {
    try {
        for (var sensorName in sensors) {
            var sensor = sensors[sensorName];
            sensor.value = analogRead(sensor.pin);

            if (!sensorHistory[sensorName]) {
                sensorHistory[sensorName] = [];
            }

            sensorHistory[sensorName].push({
                timestamp: Date.now(),
                value: sensor.value
            });

            if (sensorHistory[sensorName].length > maxHistoryPoints) {
                sensorHistory[sensorName].shift();
            }
        }
    } catch (error) {
        console.log(JSON.stringify(error));
    }
}

function drawSensorValues() {
    display.setTextSize(1);
    display.setTextAlign('left', 'top');

    var y = 40;
    for (var sensorName in sensors) {
        var sensor = sensors[sensorName];
        var value = sensor.value;
        var unit = sensor.unit;

        display.drawText(sensorName.charAt(0).toUpperCase() + sensorName.slice(1) + ':', 10, y);
        display.drawText(value.toFixed(2) + ' ' + unit, 10, y + 10);

        var barWidth = 100;
        var barHeight = 8;
        var barX = 10;
        var barY = y + 20;

        display.drawRect(barX, barY, barWidth, barHeight);
        var fillWidth = ((value - sensor.min) / (sensor.max - sensor.min)) * barWidth;
        display.fillRect(barX, barY, fillWidth, barHeight);

        y += 35;
    }
}

function drawSensorCharts() {
    if (Object.keys(sensorHistory).length === 0) return;

    var chartWidth = tftWidth - 20;
    var chartHeight = 60;
    var chartX = 10;
    var chartY = tftHeight - chartHeight - 10;

    display.drawRect(chartX, chartY, chartWidth, chartHeight);

    var colors = [WILLY_PRICOLOR, 0x00FF00, 0x0000FF, 0xFFFF00, 0xFF00FF, 0x00FFFF, 0xFFA500, 0x800080];
    var sensorNames = Object.keys(sensors);

    for (var i = 0; i < Math.min(4, sensorNames.length); i++) {
        var sensorName = sensorNames[i];
        var history = sensorHistory[sensorName];
        if (history.length < 2) continue;

        var color = colors[i];
        var points = [];

        for (var j = 0; j < history.length; j++) {
            var x = chartX + (j * chartWidth) / history.length;
            var y = chartY + chartHeight - ((history[j].value - sensors[sensorName].min) / (sensors[sensorName].max - sensors[sensorName].min)) * chartHeight;
            points.push({ x: x, y: y });
        }

        display.setColor(color);
        for (var j = 1; j < points.length; j++) {
            display.drawLine(points[j - 1].x, points[j - 1].y, points[j].x, points[j].y);
        }
    }

    display.setColor(WILLY_PRICOLOR);
}

function calibrateSensor(sensorName) {
    var sensor = sensors[sensorName];
    if (!sensor) return;

    console.log('Calibrating ' + sensorName + '...');

    var readings = [];
    for (var i = 0; i < 10; i++) {
        readings.push(analogRead(sensor.pin));
        delay(100);
    }

    var avg = readings.reduce((a, b) => a + b, 0) / readings.length;
    sensor.min = avg * 0.8;
    sensor.max = avg * 1.2;

    console.log(sensorName + ' calibrated. Min: ' + sensor.min + ', Max: ' + sensor.max);
}

function exportSensorData() {
    try {
        var csv = 'Timestamp,';
        for (var sensorName in sensors) {
            csv += sensorName + ',';
        }
        csv += '\n';

        var maxPoints = 0;
        for (var sensorName in sensorHistory) {
            maxPoints = Math.max(maxPoints, sensorHistory[sensorName].length);
        }

        for (var i = 0; i < maxPoints; i++) {
            csv += new Date().toISOString() + ',';
            for (var sensorName in sensors) {
                var history = sensorHistory[sensorName];
                csv += (history[i] ? history[i].value : 0) + ',';
            }
            csv += '\n';
        }

        storage.write('sensor_data_export.csv', csv, 'write');
        console.log('Sensor data exported successfully');
    } catch (error) {
        console.log(JSON.stringify(error));
    }
}

function saveSensorConfig() {
    try {
        var config = {
            sensors: sensors,
            history: sensorHistory
        };

        storage.write('sensor_config', JSON.stringify(config), 'write');
        console.log('Sensor configuration saved');
    } catch (error) {
        console.log(JSON.stringify(error));
    }
}

function loadSensorConfig() {
    try {
        var data = storage.read('sensor_config');
        if (data) {
            var config = JSON.parse(data);
            sensors = config.sensors || sensors;
            sensorHistory = config.history || {};

            console.log('Sensor configuration loaded');
        }
    } catch (error) {
        console.log(JSON.stringify(error));
    }
}

function main() {
    loadSensorConfig();

    var running = true;
    var updateInterval = 1000;
    var lastUpdate = 0;

    while (running) {
        var currentTime = Date.now();

        if (currentTime - lastUpdate >= updateInterval) {
            readSensors();
            lastUpdate = currentTime;
        }

        if (isLogging) {
            if (currentTime - lastLogTime >= logInterval) {
                saveSensorConfig();
                lastLogTime = currentTime;
            }
        }

        drawSensorHub('Sensor Hub');
        drawSensorValues();
        drawSensorCharts();

        if (isLogging) {
            display.drawText('Logging...', 10, tftHeight - 30);
        }

        if (keyboard.getSelPress()) {
            var choices = [];

            for (var sensorName in sensors) {
                choices.push('Calibrate ' + sensorName);
            }

            choices.push('Start Logging', 'Stop Logging', 'Export Data', 'Save Config', 'Load Config', 'Quit');

            var choice = dialog.choice(choices);

            if (choice === 'Quit') {
                running = false;
            } else if (choice === 'Start Logging') {
                isLogging = true;
            } else if (choice === 'Stop Logging') {
                isLogging = false;
            } else if (choice === 'Export Data') {
                exportSensorData();
            } else if (choice === 'Save Config') {
                saveSensorConfig();
            } else if (choice === 'Load Config') {
                loadSensorConfig();
            } else {
                for (var sensorName in sensors) {
                    if (choice === 'Calibrate ' + sensorName) {
                        calibrateSensor(sensorName);
                    }
                }
            }
        }

        display.draw();
        delay(100);
    }
}

main();
