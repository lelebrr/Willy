var dialog = require('dialog');
var wifi = require('wifi');
var display = require('display');
var keyboard = require('keyboard');
var storage = require('storage');

var tftWidth = display.width();
var tftHeight = display.height();

var displayModes = ['Text', 'Graphics', 'Chart', 'Image'];
var currentMode = 'Text';
var displayHistory = [];
var maxHistoryPoints = 50;
var currentColor = WILLY_PRICOLOR;
var backgroundColor = 0x000000;
var fontSize = 1;

function drawDisplayController(title) {
    display.fill(backgroundColor);
    display.drawRoundRect(
        5,
        5,
        tftWidth - 10,
        tftHeight - 10,
        5,
        currentColor
    );
    display.setTextSize(2);
    display.setTextAlign('center', 'top');
    display.setTextColor(currentColor);
    display.drawText(
        title.length > 20 ? title.substring(0, 20) : title,
        tftWidth / 2,
        5
    );
    display.setTextAlign('left', 'top');
}

function drawTextMode() {
    display.setTextSize(fontSize);
    display.setTextColor(currentColor);

    var y = 40;
    display.drawText('Display Mode: ' + currentMode, 10, y);
    y += 20 * fontSize;

    display.drawText('Font Size: ' + fontSize, 10, y);
    y += 20 * fontSize;

    display.drawText('Background: #' + backgroundColor.toString(16).padStart(6, '0'), 10, y);
    y += 20 * fontSize;

    display.drawText('Foreground: #' + currentColor.toString(16).padStart(6, '0'), 10, y);
    y += 20 * fontSize;

    var currentTime = new Date().toLocaleTimeString();
    display.setTextAlign('center', 'top');
    display.drawText('Current Time: ' + currentTime, tftWidth / 2, y);
    display.setTextAlign('left', 'top');
}

function drawGraphicsMode() {
    display.setTextColor(currentColor);

    var centerX = tftWidth / 2;
    var centerY = tftHeight / 2;

    display.drawCircle(centerX, centerY, 50);
    display.drawRect(centerX - 30, centerY - 30, 60, 60);
    display.drawLine(centerX - 40, centerY, centerX + 40, centerY);
    display.drawLine(centerX, centerY - 40, centerX, centerY + 40);

    display.setTextSize(1);
    display.setTextAlign('center', 'top');
    display.drawText('Graphics Mode', centerX, 10);
    display.setTextAlign('left', 'top');
}

function drawChartMode() {
    display.setTextColor(currentColor);

    var chartWidth = tftWidth - 40;
    var chartHeight = tftHeight - 80;
    var chartX = 20;
    var chartY = 40;

    display.drawRect(chartX, chartY, chartWidth, chartHeight);

    if (displayHistory.length > 1) {
        var colors = [WILLY_PRICOLOR, 0x00FF00, 0x0000FF, 0xFFFF00];

        for (var i = 0; i < 4 && i < displayHistory.length; i++) {
            var history = displayHistory[i];
            var color = colors[i];

            display.setColor(color);
            for (var j = 1; j < history.length; j++) {
                var x1 = chartX + (j - 1) * chartWidth / history.length;
                var y1 = chartY + chartHeight - history[j - 1] * chartHeight;
                var x2 = chartX + j * chartWidth / history.length;
                var y2 = chartY + chartHeight - history[j] * chartHeight;

                display.drawLine(x1, y1, x2, y2);
            }
        }
    }

    display.setColor(currentColor);
    display.setTextSize(1);
    display.setTextAlign('center', 'top');
    display.drawText('Chart Mode', tftWidth / 2, 10);
    display.setTextAlign('left', 'top');
}

function drawImageMode() {
    display.setTextColor(currentColor);

    var imgWidth = 100;
    var imgHeight = 100;
    var imgX = (tftWidth - imgWidth) / 2;
    var imgY = (tftHeight - imgHeight) / 2;

    for (var y = 0; y < imgHeight; y += 5) {
        for (var x = 0; x < imgWidth; x += 5) {
            var color = (x + y) % 2 === 0 ? currentColor : backgroundColor;
            display.setColor(color);
            display.fillRect(imgX + x, imgY + y, 5, 5);
        }
    }

    display.setColor(currentColor);
    display.setTextSize(1);
    display.setTextAlign('center', 'top');
    display.drawText('Image Mode', tftWidth / 2, 10);
    display.setTextAlign('left', 'top');
}

function updateDisplayHistory() {
    if (!displayHistory[0]) {
        displayHistory[0] = [];
    }

    displayHistory[0].push(Math.random());

    if (displayHistory[0].length > maxHistoryPoints) {
        displayHistory[0].shift();
    }
}

function changeFontSize() {
    fontSize = fontSize >= 3 ? 1 : fontSize + 1;
    console.log('Font size changed to: ' + fontSize);
}

function changeColor() {
    var colors = [WILLY_PRICOLOR, 0x00FF00, 0x0000FF, 0xFFFF00, 0xFF00FF, 0x00FFFF, 0xFFA500, 0x800080];
    var currentIndex = colors.indexOf(currentColor);
    currentColor = colors[(currentIndex + 1) % colors.length];
    console.log('Color changed to: #' + currentColor.toString(16).padStart(6, '0'));
}

function changeBackgroundColor() {
    var colors = [0x000000, 0xFFFFFF, 0x808080, 0x404040, 0xC0C0C0];
    var currentIndex = colors.indexOf(backgroundColor);
    backgroundColor = colors[(currentIndex + 1) % colors.length];
    console.log('Background color changed to: #' + backgroundColor.toString(16).padStart(6, '0'));
}

function clearDisplay() {
    display.fill(backgroundColor);
    console.log('Display cleared');
}

function saveDisplayConfig() {
    try {
        var config = {
            mode: currentMode,
            color: currentColor,
            backgroundColor: backgroundColor,
            fontSize: fontSize,
            history: displayHistory
        };

        storage.write('display_config', JSON.stringify(config), 'write');
        console.log('Display configuration saved');
    } catch (error) {
        console.log(JSON.stringify(error));
    }
}

function loadDisplayConfig() {
    try {
        var data = storage.read('display_config');
        if (data) {
            var config = JSON.parse(data);
            currentMode = config.mode || 'Text';
            currentColor = config.color || WILLY_PRICOLOR;
            backgroundColor = config.backgroundColor || 0x000000;
            fontSize = config.fontSize || 1;
            displayHistory = config.history || [];

            console.log('Display configuration loaded');
        }
    } catch (error) {
        console.log(JSON.stringify(error));
    }
}

function main() {
    loadDisplayConfig();

    var running = true;
    var updateInterval = 1000;
    var lastUpdate = 0;

    while (running) {
        var currentTime = Date.now();

        if (currentTime - lastUpdate >= updateInterval) {
            updateDisplayHistory();
            lastUpdate = currentTime;
        }

        drawDisplayController('Display Controller');

        if (currentMode === 'Text') {
            drawTextMode();
        } else if (currentMode === 'Graphics') {
            drawGraphicsMode();
        } else if (currentMode === 'Chart') {
            drawChartMode();
        } else if (currentMode === 'Image') {
            drawImageMode();
        }

        if (keyboard.getSelPress()) {
            var choices = ['Change Mode', 'Change Color', 'Change Background', 'Change Font Size', 'Clear Display', 'Save Config', 'Load Config', 'Quit'];
            var choice = dialog.choice(choices);

            if (choice === 'Quit') {
                running = false;
            } else if (choice === 'Change Mode') {
                var modeIndex = displayModes.indexOf(currentMode);
                currentMode = displayModes[(modeIndex + 1) % displayModes.length];
            } else if (choice === 'Change Color') {
                changeColor();
            } else if (choice === 'Change Background') {
                changeBackgroundColor();
            } else if (choice === 'Change Font Size') {
                changeFontSize();
            } else if (choice === 'Clear Display') {
                clearDisplay();
            } else if (choice === 'Save Config') {
                saveDisplayConfig();
            } else if (choice === 'Load Config') {
                loadDisplayConfig();
            }
        }

        display.draw();
        delay(100);
    }
}

main();
