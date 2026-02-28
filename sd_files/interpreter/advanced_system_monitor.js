var dialog = require('dialog');
var wifi = require('wifi');
var display = require('display');
var keyboard = require('keyboard');
var storage = require('storage');

var tftWidth = display.width();
var tftHeight = display.height();

var systemInfo = {
    cpu: 0,
    memory: 0,
    battery: 0,
    temperature: 0,
    network: 0,
    uptime: 0
};

var historyData = [];
var maxHistoryPoints = 50;

function drawSystemMonitor(title) {
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
        systemInfo.cpu = Math.random() * 100;
        systemInfo.memory = Math.random() * 100;
        systemInfo.battery = Math.random() * 100;
        systemInfo.temperature = 20 + Math.random() * 60;
        systemInfo.network = Math.random() * 100;
        systemInfo.uptime = Date.now() / 1000;

        historyData.push({
            cpu: systemInfo.cpu,
            memory: systemInfo.memory,
            battery: systemInfo.battery,
            temperature: systemInfo.temperature,
            network: systemInfo.network,
            timestamp: Date.now()
        });

        if (historyData.length > maxHistoryPoints) {
            historyData.shift();
        }
    } catch (error) {
        console.log(JSON.stringify(error));
    }
}

function drawSystemInfo() {
    display.setTextSize(1);
    display.setTextAlign('left', 'top');

    var y = 40;
    display.drawText('CPU: ' + systemInfo.cpu.toFixed(1) + '%', 10, y);
    y += 20;
    display.drawText('Memory: ' + systemInfo.memory.toFixed(1) + '%', 10, y);
    y += 20;
    display.drawText('Battery: ' + systemInfo.battery.toFixed(1) + '%', 10, y);
    y += 20;
    display.drawText('Temperature: ' + systemInfo.temperature.toFixed(1) + '°C', 10, y);
    y += 20;
    display.drawText('Network: ' + systemInfo.network.toFixed(1) + '%', 10, y);
    y += 20;
    display.drawText('Uptime: ' + Math.floor(systemInfo.uptime) + 's', 10, y);
}

function drawCharts() {
    if (historyData.length < 2) return;

    var chartWidth = tftWidth - 20;
    var chartHeight = 100;
    var chartX = 10;
    var chartY = tftHeight - chartHeight - 10;

    display.drawRect(chartX, chartY, chartWidth, chartHeight);

    var colors = [WILLY_PRICOLOR, 0x00FF00, 0x0000FF, 0xFFFF00, 0xFF00FF];
    var dataTypes = ['cpu', 'memory', 'battery', 'temperature', 'network'];

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

function saveData() {
    try {
        storage.write('system_monitor_data', JSON.stringify(historyData), 'write');
        console.log('Data saved successfully');
    } catch (error) {
        console.log(JSON.stringify(error));
    }
}

function loadData() {
    try {
        var data = storage.read('system_monitor_data');
        if (data) {
            historyData = JSON.parse(data);
            console.log('Data loaded successfully');
        }
    } catch (error) {
        console.log(JSON.stringify(error));
    }
}

function main() {
    loadData();

    var running = true;
    var updateInterval = 1000;
    var lastUpdate = 0;

    while (running) {
        var currentTime = Date.now();

        if (currentTime - lastUpdate >= updateInterval) {
            updateSystemInfo();
            lastUpdate = currentTime;
        }

        drawSystemMonitor('System Monitor');
        drawSystemInfo();
        drawCharts();

        if (keyboard.getSelPress()) {
            var choices = ['Save Data', 'Clear Data', 'Export CSV', 'Quit'];
            var choice = dialog.choice(choices);

            if (choice === 'Quit') {
                running = false;
            } else if (choice === 'Save Data') {
                saveData();
            } else if (choice === 'Clear Data') {
                historyData = [];
            } else if (choice === 'Export CSV') {
                var csv = 'Timestamp,CPU,Memory,Battery,Temperature,Network\n';
                for (var i = 0; i < historyData.length; i++) {
                    var data = historyData[i];
                    csv += data.timestamp + ',' + data.cpu + ',' + data.memory + ',' + data.battery + ',' + data.temperature + ',' + data.network + '\n';
                }
                storage.write('system_monitor_export.csv', csv, 'write');
                console.log('CSV exported successfully');
            }
        }

        if (keyboard.getPrevPress()) {
            updateInterval = Math.max(100, updateInterval - 100);
        }

        if (keyboard.getNextPress()) {
            updateInterval = Math.min(5000, updateInterval + 100);
        }

        display.draw();
        delay(50);
    }
}

main();
