var dialog = require('dialog');
var wifi = require('wifi');
var display = require('display');
var keyboard = require('keyboard');
var storage = require('storage');

var tftWidth = display.width();
var tftHeight = display.height();

var servoPins = [13, 14, 15, 16, 17, 18];
var servoAngles = [90, 90, 90, 90, 90, 90];
var servoMin = 0;
var servoMax = 180;
var servoSpeed = 10;
var isMoving = false;
var currentServo = 0;

function drawServoController(title) {
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

function drawServoStates() {
    display.setTextSize(1);
    display.setTextAlign('left', 'top');

    var y = 40;
    for (var i = 0; i < servoPins.length; i++) {
        var pin = servoPins[i];
        var angle = servoAngles[i];

        display.drawText('Servo ' + pin + ': ' + angle + '°', 10, y);

        var barWidth = 100;
        var barHeight = 10;
        var barX = 10;
        var barY = y + 15;

        display.drawRect(barX, barY, barWidth, barHeight);
        var fillWidth = (angle / 180) * barWidth;
        display.fillRect(barX, barY, fillWidth, barHeight);

        y += 35;
    }
}

function setServo(index, angle) {
    if (index >= 0 && index < servoPins.length) {
        servoAngles[index] = Math.max(servoMin, Math.min(servoMax, angle));
        updateServo(index);
        console.log('Servo ' + servoPins[index] + ' set to ' + servoAngles[index] + '°');
    }
}

function updateServo(index) {
    try {
        var pin = servoPins[index];
        var angle = servoAngles[index];
        var pulseWidth = map(angle, 0, 180, 500, 2500);

        servoWrite(pin, pulseWidth);
    } catch (error) {
        console.log(JSON.stringify(error));
    }
}

function moveServoTo(index, targetAngle, speed) {
    if (index >= 0 && index < servoPins.length) {
        isMoving = true;
        currentServo = index;

        function step() {
            var currentAngle = servoAngles[index];
            var diff = targetAngle - currentAngle;
            var stepSize = Math.sign(diff) * Math.min(Math.abs(diff), speed);

            if (Math.abs(diff) <= speed) {
                setServo(index, targetAngle);
                isMoving = false;
                return;
            }

            setServo(index, currentAngle + stepSize);
            setTimeout(step, 50);
        }

        step();
    }
}

function sweepServo(index, minAngle, maxAngle, speed) {
    if (index >= 0 && index < servoPins.length) {
        isMoving = true;
        currentServo = index;
        var direction = 1;
        var currentAngle = minAngle;

        function sweep() {
            if (!isMoving) return;

            currentAngle += direction * speed;

            if (currentAngle >= maxAngle) {
                currentAngle = maxAngle;
                direction = -1;
            } else if (currentAngle <= minAngle) {
                currentAngle = minAngle;
                direction = 1;
            }

            setServo(index, currentAngle);
            setTimeout(sweep, 50);
        }

        sweep();
    }
}

function stopAllServos() {
    isMoving = false;
    for (var i = 0; i < servoPins.length; i++) {
        setServo(i, 90);
    }
    console.log('All servos stopped');
}

function saveServoConfig() {
    try {
        var config = {
            minAngle: servoMin,
            maxAngle: servoMax,
            speed: servoSpeed,
            angles: servoAngles
        };

        storage.write('servo_config', JSON.stringify(config), 'write');
        console.log('Servo configuration saved');
    } catch (error) {
        console.log(JSON.stringify(error));
    }
}

function loadServoConfig() {
    try {
        var data = storage.read('servo_config');
        if (data) {
            var config = JSON.parse(data);
            servoMin = config.minAngle || 0;
            servoMax = config.maxAngle || 180;
            servoSpeed = config.speed || 10;
            servoAngles = config.angles || [90, 90, 90, 90, 90, 90];

            for (var i = 0; i < servoAngles.length; i++) {
                updateServo(i);
            }

            console.log('Servo configuration loaded');
        }
    } catch (error) {
        console.log(JSON.stringify(error));
    }
}

function main() {
    loadServoConfig();

    var running = true;

    while (running) {
        drawServoController('Servo Controller');
        drawServoStates();

        if (isMoving) {
            display.drawText('Servo ' + servoPins[currentServo] + ' moving...', 10, tftHeight - 30);
        }

        if (keyboard.getSelPress()) {
            var choices = [];

            for (var i = 0; i < servoPins.length; i++) {
                choices.push('Servo ' + i + ' +10°');
                choices.push('Servo ' + i + ' -10°');
                choices.push('Servo ' + i + ' +1°');
                choices.push('Servo ' + i + ' -1°');
                choices.push('Servo ' + i + ' Sweep');
            }

            choices.push('All Servos Home', 'Stop All', 'Save Config', 'Load Config', 'Quit');

            var choice = dialog.choice(choices);

            if (choice === 'Quit') {
                running = false;
            } else if (choice === 'All Servos Home') {
                for (var i = 0; i < servoPins.length; i++) {
                    setServo(i, 90);
                }
            } else if (choice === 'Stop All') {
                stopAllServos();
            } else if (choice === 'Save Config') {
                saveServoConfig();
            } else if (choice === 'Load Config') {
                loadServoConfig();
            } else {
                for (var i = 0; i < servoPins.length; i++) {
                    if (choice === 'Servo ' + i + ' +10°') {
                        moveServoTo(i, servoAngles[i] + 10, servoSpeed);
                    } else if (choice === 'Servo ' + i + ' -10°') {
                        moveServoTo(i, servoAngles[i] - 10, servoSpeed);
                    } else if (choice === 'Servo ' + i + ' +1°') {
                        moveServoTo(i, servoAngles[i] + 1, servoSpeed);
                    } else if (choice === 'Servo ' + i + ' -1°') {
                        moveServoTo(i, servoAngles[i] - 1, servoSpeed);
                    } else if (choice === 'Servo ' + i + ' Sweep') {
                        sweepServo(i, 0, 180, 2);
                    }
                }
            }
        }

        display.draw();
        delay(100);
    }
}

main();
