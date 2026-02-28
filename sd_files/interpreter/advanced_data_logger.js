var dialog = require('dialog');
var wifi = require('wifi');
var display = require('display');
var keyboard = require('keyboard');
var storage = require('storage');

var tftWidth = display.width();
var tftHeight = display.height();

var logData = [];
var currentLog = null;
var currentLogIndex = -1;
var logTypes = ['Temperature', 'Humidity', 'Pressure', 'Light', 'Voltage', 'Current'];
var currentType = 'Temperature';
var isLogging = false;
var logInterval = 1000;
var lastLogTime = 0;
var maxLogPoints = 1000;

function drawDataLogger(title) {
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

function createLog() {
    var name = prompt('Enter log name:');
    if (name) {
        var log = {
            name: name,
            type: currentType,
            data: [],
            startTime: new Date().toISOString(),
            endTime: null,
            interval: logInterval,
            status: 'stopped'
        };

        logData.push(log);
        saveLogs();
        console.log('Log created: ' + name);
    }
}

function startLogging() {
    if (currentLog && currentLog.status === 'stopped') {
        currentLog.status = 'running';
        currentLog.startTime = new Date().toISOString();
        isLogging = true;
        console.log('Logging started: ' + currentLog.name);
    }
}

function stopLogging() {
    if (currentLog && currentLog.status === 'running') {
        currentLog.status = 'stopped';
        currentLog.endTime = new Date().toISOString();
        isLogging = false;
        saveLogs();
        console.log('Logging stopped: ' + currentLog.name);
    }
}

function clearLog() {
    if (currentLog) {
        currentLog.data = [];
        currentLog.startTime = new Date().toISOString();
        currentLog.endTime = null;
        saveLogs();
        console.log('Log cleared: ' + currentLog.name);
    }
}

function deleteLog() {
    if (currentLogIndex !== -1) {
        logData.splice(currentLogIndex, 1);
        currentLog = null;
        currentLogIndex = -1;
        saveLogs();
        console.log('Log deleted');
    }
}

function addDataPoint() {
    if (!currentLog || currentLog.status !== 'running') return;

    var value = 0;
    switch (currentLog.type) {
        case 'Temperature':
            value = 20 + Math.random() * 20;
            break;
        case 'Humidity':
            value = 30 + Math.random() * 40;
            break;
        case 'Pressure':
            value = 1000 + Math.random() * 50;
            break;
        case 'Light':
            value = Math.random() * 1000;
            break;
        case 'Voltage':
            value = 3.0 + Math.random() * 2.0;
            break;
        case 'Current':
            value = Math.random() * 5.0;
            break;
    }

    currentLog.data.push({
        timestamp: new Date().toISOString(),
        value: value
    });

    if (currentLog.data.length > maxLogPoints) {
        currentLog.data.shift();
    }
}

function drawLogsList() {
    display.setTextSize(1);
    display.setTextAlign('left', 'top');

    var y = 40;
    for (var i = 0; i < logData.length && i < 12; i++) {
        var log = logData[i];
        var name = log.name || 'Untitled Log';
        var type = log.type || 'Unknown';
        var status = log.status || 'stopped';
        var count = log.data.length || 0;

        display.drawText(name, 10, y);
        display.drawText(type + ' - ' + status, 10, y + 10);
        display.drawText('Data points: ' + count, 10, y + 20);

        y += 35;
    }
}

function drawLogData() {
    if (!currentLog) return;

    display.setTextSize(1);
    display.setTextAlign('left', 'top');

    display.drawText('Log: ' + currentLog.name, 10, 40);
    display.drawText('Type: ' + currentLog.type, 10, 60);
    display.drawText('Status: ' + currentLog.status, 10, 80);
    display.drawText('Data points: ' + currentLog.data.length, 10, 100);

    if (currentLog.data.length > 0) {
        var latest = currentLog.data[currentLog.data.length - 1];
        display.drawText('Latest: ' + latest.value.toFixed(2), 10, 120);
        display.drawText('Time: ' + new Date(latest.timestamp).toLocaleTimeString(), 10, 140);
    }

    if (isLogging) {
        display.drawText('Logging...', 10, tftHeight - 30);
    }
}

function drawChart() {
    if (!currentLog || currentLog.data.length < 2) return;

    var chartWidth = tftWidth - 20;
    var chartHeight = 100;
    var chartX = 10;
    var chartY = tftHeight - chartHeight - 10;

    display.drawRect(chartX, chartY, chartWidth, chartHeight);

    var maxValue = Math.max(...currentLog.data.map(d => d.value));
    var minValue = Math.min(...currentLog.data.map(d => d.value));
    var valueRange = maxValue - minValue || 1;

    display.setColor(0x00FF00);
    for (var i = 1; i < currentLog.data.length; i++) {
        var x1 = chartX + (i - 1) * chartWidth / currentLog.data.length;
        var y1 = chartY + chartHeight - (currentLog.data[i - 1].value - minValue) * chartHeight / valueRange;
        var x2 = chartX + i * chartWidth / currentLog.data.length;
        var y2 = chartY + chartHeight - (currentLog.data[i].value - minValue) * chartHeight / valueRange;

        display.drawLine(x1, y1, x2, y2);
    }

    display.setColor(WILLY_PRICOLOR);
}

function exportLog() {
    if (!currentLog) return;

    try {
        var csv = 'Timestamp,Value\n';
        for (var i = 0; i < currentLog.data.length; i++) {
            var data = currentLog.data[i];
            csv += data.timestamp + ',' + data.value + '\n';
        }

        storage.write('log_export_' + currentLog.name + '.csv', csv, 'write');
        console.log('Log exported successfully');
    } catch (error) {
        console.log(JSON.stringify(error));
    }
}

function saveLogs() {
    try {
        storage.write('data_logs', JSON.stringify(logData), 'write');
        console.log('Logs saved successfully');
    } catch (error) {
        console.log(JSON.stringify(error));
    }
}

function loadLogs() {
    try {
        var data = storage.read('data_logs');
        if (data) {
            logData = JSON.parse(data);
            console.log('Logs loaded successfully');
        }
    } catch (error) {
        console.log(JSON.stringify(error));
        logData = [];
    }
}

function main() {
    loadLogs();

    var running = true;
    var viewMode = 'list';

    while (running) {
        drawDataLogger('Data Logger');

        if (viewMode === 'list') {
            drawLogsList();
        } else if (viewMode === 'data') {
            drawLogData();
            drawChart();
        }

        if (viewMode === 'data') {
            var currentTime = Date.now();
            if (currentTime - lastLogTime >= logInterval) {
                addDataPoint();
                lastLogTime = currentTime;
            }
        }

        if (keyboard.getSelPress()) {
            var choices = [];

            if (viewMode === 'list') {
                choices = ['Create Log', 'Select Log', 'Delete Log', 'Change Type', 'Quit'];
            } else if (viewMode === 'data') {
                choices = ['Start Logging', 'Stop Logging', 'Clear Log', 'Export Log', 'Back to List', 'Quit'];
            }

            var choice = dialog.choice(choices);

            if (choice === 'Quit') {
                running = false;
            } else if (choice === 'Create Log' && viewMode === 'list') {
                createLog();
            } else if (choice === 'Select Log' && viewMode === 'list') {
                var logIndex = parseInt(prompt('Enter log index:'));
                if (!isNaN(logIndex) && logIndex >= 0 && logIndex < logData.length) {
                    currentLog = logData[logIndex];
                    currentLogIndex = logIndex;
                    viewMode = 'data';
                }
            } else if (choice === 'Delete Log' && viewMode === 'list') {
                var logIndex = parseInt(prompt('Enter log index to delete:'));
                if (!isNaN(logIndex) && logIndex >= 0 && logIndex < logData.length) {
                    logData.splice(logIndex, 1);
                    saveLogs();
                }
            } else if (choice === 'Change Type' && viewMode === 'list') {
                var typeIndex = logTypes.indexOf(currentType);
                currentType = logTypes[(typeIndex + 1) % logTypes.length];
            } else if (choice === 'Start Logging' && viewMode === 'data') {
                startLogging();
            } else if (choice === 'Stop Logging' && viewMode === 'data') {
                stopLogging();
            } else if (choice === 'Clear Log' && viewMode === 'data') {
                clearLog();
            } else if (choice === 'Export Log' && viewMode === 'data') {
                exportLog();
            } else if (choice === 'Back to List' && viewMode === 'data') {
                viewMode = 'list';
            }
        }

        display.draw();
        delay(100);
    }
}

main();
