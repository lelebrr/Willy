var dialog = require('dialog');
var wifi = require('wifi');
var display = require('display');
var keyboard = require('keyboard');
var storage = require('storage');

var tftWidth = display.width();
var tftHeight = display.height();

var ledPins = [2, 4, 5, 12, 14, 16, 17, 18];
var ledStates = [false, false, false, false, false, false, false, false];
var ledBrightness = [255, 255, 255, 255, 255, 255, 255, 255];
var ledPatterns = [];
var currentPattern = null;
var isRunning = false;
var patternIndex = 0;

function drawLEDController(title) {
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

function drawLEDStates() {
    display.setTextSize(1);
    display.setTextAlign('left', 'top');

    var y = 40;
    for (var i = 0; i < ledPins.length; i++) {
        var pin = ledPins[i];
        var state = ledStates[i] ? 'ON' : 'OFF';
        var brightness = ledBrightness[i];

        display.drawText('LED ' + pin + ': ' + state, 10, y);
        display.drawText('Brightness: ' + brightness, 10, y + 10);

        y += 25;
    }
}

function toggleLED(index) {
    if (index >= 0 && index < ledPins.length) {
        ledStates[index] = !ledStates[index];
        updateLED(index);
        console.log('LED ' + ledPins[index] + ' toggled to ' + (ledStates[index] ? 'ON' : 'OFF'));
    }
}

function setLED(index, state, brightness) {
    if (index >= 0 && index < ledPins.length) {
        ledStates[index] = state;
        ledBrightness[index] = brightness;
        updateLED(index);
        console.log('LED ' + ledPins[index] + ' set to ' + state + ' with brightness ' + brightness);
    }
}

function updateLED(index) {
    try {
        var pin = ledPins[index];
        var state = ledStates[index];
        var brightness = ledBrightness[index];

        if (state) {
            analogWrite(pin, brightness);
        } else {
            digitalWrite(pin, 0);
        }
    } catch (error) {
        console.log(JSON.stringify(error));
    }
}

function allLEDsOn() {
    for (var i = 0; i < ledPins.length; i++) {
        setLED(i, true, 255);
    }
}

function allLEDsOff() {
    for (var i = 0; i < ledPins.length; i++) {
        setLED(i, false, 0);
    }
}

function createPattern() {
    var name = prompt('Enter pattern name:');
    if (name) {
        var pattern = {
            name: name,
            steps: [],
            repeat: false,
            speed: 500
        };

        ledPatterns.push(pattern);
        console.log('Pattern created: ' + name);
    }
}

function addPatternStep() {
    if (!currentPattern) return;

    var step = {
        ledIndex: parseInt(prompt('Enter LED index (0-7):')),
        state: prompt('Enter state (on/off):') === 'on',
        brightness: parseInt(prompt('Enter brightness (0-255):')),
        duration: parseInt(prompt('Enter duration (ms):'))
    };

    if (!isNaN(step.ledIndex) && step.ledIndex >= 0 && step.ledIndex < ledPins.length) {
        currentPattern.steps.push(step);
        console.log('Pattern step added');
    }
}

function runPattern() {
    if (!currentPattern || currentPattern.steps.length === 0) return;

    isRunning = true;
    patternIndex = 0;

    function executeStep() {
        if (!isRunning || patternIndex >= currentPattern.steps.length) {
            if (currentPattern.repeat) {
                patternIndex = 0;
                setTimeout(executeStep, currentPattern.speed);
            } else {
                isRunning = false;
            }
            return;
        }

        var step = currentPattern.steps[patternIndex];
        setLED(step.ledIndex, step.state, step.brightness);

        patternIndex++;
        setTimeout(executeStep, step.duration);
    }

    executeStep();
}

function stopPattern() {
    isRunning = false;
    allLEDsOff();
    console.log('Pattern stopped');
}

function savePatterns() {
    try {
        storage.write('led_patterns', JSON.stringify(ledPatterns), 'write');
        console.log('Patterns saved successfully');
    } catch (error) {
        console.log(JSON.stringify(error));
    }
}

function loadPatterns() {
    try {
        var data = storage.read('led_patterns');
        if (data) {
            ledPatterns = JSON.parse(data);
            console.log('Patterns loaded successfully');
        }
    } catch (error) {
        console.log(JSON.stringify(error));
        ledPatterns = [];
    }
}

function main() {
    loadPatterns();

    var running = true;

    while (running) {
        drawLEDController('LED Controller');
        drawLEDStates();

        if (isRunning) {
            display.drawText('Pattern Running: ' + (currentPattern ? currentPattern.name : 'Unknown'), 10, tftHeight - 30);
        }

        if (keyboard.getSelPress()) {
            var choices = ['Toggle LED 0', 'Toggle LED 1', 'Toggle LED 2', 'Toggle LED 3', 'Toggle LED 4', 'Toggle LED 5', 'Toggle LED 6', 'Toggle LED 7', 'All LEDs On', 'All LEDs Off', 'Create Pattern', 'Select Pattern', 'Add Step', 'Run Pattern', 'Stop Pattern', 'Quit'];
            var choice = dialog.choice(choices);

            if (choice === 'Quit') {
                running = false;
            } else if (choice === 'Toggle LED 0') {
                toggleLED(0);
            } else if (choice === 'Toggle LED 1') {
                toggleLED(1);
            } else if (choice === 'Toggle LED 2') {
                toggleLED(2);
            } else if (choice === 'Toggle LED 3') {
                toggleLED(3);
            } else if (choice === 'Toggle LED 4') {
                toggleLED(4);
            } else if (choice === 'Toggle LED 5') {
                toggleLED(5);
            } else if (choice === 'Toggle LED 6') {
                toggleLED(6);
            } else if (choice === 'Toggle LED 7') {
                toggleLED(7);
            } else if (choice === 'All LEDs On') {
                allLEDsOn();
            } else if (choice === 'All LEDs Off') {
                allLEDsOff();
            } else if (choice === 'Create Pattern') {
                createPattern();
            } else if (choice === 'Select Pattern') {
                var patternIndex = parseInt(prompt('Enter pattern index:'));
                if (!isNaN(patternIndex) && patternIndex >= 0 && patternIndex < ledPatterns.length) {
                    currentPattern = ledPatterns[patternIndex];
                }
            } else if (choice === 'Add Step') {
                addPatternStep();
            } else if (choice === 'Run Pattern') {
                runPattern();
            } else if (choice === 'Stop Pattern') {
                stopPattern();
            }
        }

        display.draw();
        delay(100);
    }
}

main();
