var dialog = require('dialog');
var wifi = require('wifi');
var display = require('display');
var keyboard = require('keyboard');
var storage = require('storage');

var tftWidth = display.width();
var tftHeight = display.height();

var motorPins = {
    motor1: { pwm: 12, dir1: 14, dir2: 16, speed: 0, direction: 0 },
    motor2: { pwm: 13, dir1: 15, dir2: 17, speed: 0, direction: 0 },
    motor3: { pwm: 18, dir1: 19, dir2: 21, speed: 0, direction: 0 },
    motor4: { pwm: 22, dir1: 23, dir2: 25, speed: 0, direction: 0 }
};

var motorSpeeds = [0, 0, 0, 0];
var motorDirections = [0, 0, 0, 0];
var maxSpeed = 255;
var acceleration = 5;
var isMoving = false;

function drawMotorController(title) {
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

function drawMotorStates() {
    display.setTextSize(1);
    display.setTextAlign('left', 'top');

    var y = 40;
    for (var i = 0; i < 4; i++) {
        var motor = motorPins['motor' + (i + 1)];
        var speed = motorSpeeds[i];
        var direction = motorDirections[i];

        display.drawText('Motor ' + (i + 1) + ':', 10, y);
        display.drawText('Speed: ' + speed + '/' + maxSpeed, 10, y + 10);
        display.drawText('Direction: ' + (direction === 1 ? 'Forward' : direction === -1 ? 'Backward' : 'Stop'), 10, y + 20);

        var barWidth = 100;
        var barHeight = 10;
        var barX = 10;
        var barY = y + 30;

        display.drawRect(barX, barY, barWidth, barHeight);
        var fillWidth = (speed / maxSpeed) * barWidth;
        display.fillRect(barX, barY, fillWidth, barHeight);

        y += 50;
    }
}

function setMotor(motorIndex, speed, direction) {
    if (motorIndex >= 0 && motorIndex < 4) {
        motorSpeeds[motorIndex] = Math.max(0, Math.min(maxSpeed, speed));
        motorDirections[motorIndex] = direction;
        updateMotor(motorIndex);
        console.log('Motor ' + (motorIndex + 1) + ' set to speed ' + motorSpeeds[motorIndex] + ', direction ' + direction);
    }
}

function updateMotor(motorIndex) {
    try {
        var motor = motorPins['motor' + (motorIndex + 1)];
        var speed = motorSpeeds[motorIndex];
        var direction = motorDirections[motorIndex];

        analogWrite(motor.pwm, speed);

        if (direction === 1) {
            digitalWrite(motor.dir1, 1);
            digitalWrite(motor.dir2, 0);
        } else if (direction === -1) {
            digitalWrite(motor.dir1, 0);
            digitalWrite(motor.dir2, 1);
        } else {
            digitalWrite(motor.dir1, 0);
            digitalWrite(motor.dir2, 0);
        }
    } catch (error) {
        console.log(JSON.stringify(error));
    }
}

function moveMotor(motorIndex, targetSpeed, targetDirection, accel) {
    if (motorIndex >= 0 && motorIndex < 4) {
        isMoving = true;

        function step() {
            var currentSpeed = motorSpeeds[motorIndex];
            var currentDirection = motorDirections[motorIndex];

            if (currentSpeed === targetSpeed && currentDirection === targetDirection) {
                isMoving = false;
                return;
            }

            if (currentDirection !== targetDirection) {
                setMotor(motorIndex, 0, 0);
                setTimeout(() => setMotor(motorIndex, targetSpeed, targetDirection), 100);
            } else {
                var newSpeed = currentSpeed;
                if (currentSpeed < targetSpeed) {
                    newSpeed = Math.min(currentSpeed + accel, targetSpeed);
                } else if (currentSpeed > targetSpeed) {
                    newSpeed = Math.max(currentSpeed - accel, targetSpeed);
                }
                setMotor(motorIndex, newSpeed, targetDirection);
            }

            setTimeout(step, 50);
        }

        step();
    }
}

function stopAllMotors() {
    for (var i = 0; i < 4; i++) {
        setMotor(i, 0, 0);
    }
    isMoving = false;
    console.log('All motors stopped');
}

function moveForward(speed) {
    for (var i = 0; i < 4; i++) {
        moveMotor(i, speed, 1, acceleration);
    }
    console.log('Moving forward at speed ' + speed);
}

function moveBackward(speed) {
    for (var i = 0; i < 4; i++) {
        moveMotor(i, speed, -1, acceleration);
    }
    console.log('Moving backward at speed ' + speed);
}

function turnLeft(speed) {
    moveMotor(0, speed, -1, acceleration);
    moveMotor(1, speed, -1, acceleration);
    moveMotor(2, speed, 1, acceleration);
    moveMotor(3, speed, 1, acceleration);
    console.log('Turning left at speed ' + speed);
}

function turnRight(speed) {
    moveMotor(0, speed, 1, acceleration);
    moveMotor(1, speed, 1, acceleration);
    moveMotor(2, speed, -1, acceleration);
    moveMotor(3, speed, -1, acceleration);
    console.log('Turning right at speed ' + speed);
}

function spinLeft(speed) {
    moveMotor(0, speed, -1, acceleration);
    moveMotor(1, speed, -1, acceleration);
    moveMotor(2, speed, -1, acceleration);
    moveMotor(3, speed, -1, acceleration);
    console.log('Spinning left at speed ' + speed);
}

function spinRight(speed) {
    moveMotor(0, speed, 1, acceleration);
    moveMotor(1, speed, 1, acceleration);
    moveMotor(2, speed, 1, acceleration);
    moveMotor(3, speed, 1, acceleration);
    console.log('Spinning right at speed ' + speed);
}

function saveMotorConfig() {
    try {
        var config = {
            maxSpeed: maxSpeed,
            acceleration: acceleration,
            speeds: motorSpeeds,
            directions: motorDirections
        };

        storage.write('motor_config', JSON.stringify(config), 'write');
        console.log('Motor configuration saved');
    } catch (error) {
        console.log(JSON.stringify(error));
    }
}

function loadMotorConfig() {
    try {
        var data = storage.read('motor_config');
        if (data) {
            var config = JSON.parse(data);
            maxSpeed = config.maxSpeed || 255;
            acceleration = config.acceleration || 5;
            motorSpeeds = config.speeds || [0, 0, 0, 0];
            motorDirections = config.directions || [0, 0, 0, 0];

            for (var i = 0; i < 4; i++) {
                updateMotor(i);
            }

            console.log('Motor configuration loaded');
        }
    } catch (error) {
        console.log(JSON.stringify(error));
    }
}

function main() {
    loadMotorConfig();

    var running = true;

    while (running) {
        drawMotorController('Motor Controller');
        drawMotorStates();

        if (isMoving) {
            display.drawText('Motors moving...', 10, tftHeight - 30);
        }

        if (keyboard.getSelPress()) {
            var choices = [
                'Move Forward', 'Move Backward', 'Turn Left', 'Turn Right',
                'Spin Left', 'Spin Right', 'Stop All',
                'Motor 1 + Speed', 'Motor 1 - Speed', 'Motor 1 Forward', 'Motor 1 Backward', 'Motor 1 Stop',
                'Motor 2 + Speed', 'Motor 2 - Speed', 'Motor 2 Forward', 'Motor 2 Backward', 'Motor 2 Stop',
                'Motor 3 + Speed', 'Motor 3 - Speed', 'Motor 3 Forward', 'Motor 3 Backward', 'Motor 3 Stop',
                'Motor 4 + Speed', 'Motor 4 - Speed', 'Motor 4 Forward', 'Motor 4 Backward', 'Motor 4 Stop',
                'Save Config', 'Load Config', 'Quit'
            ];

            var choice = dialog.choice(choices);

            if (choice === 'Quit') {
                running = false;
            } else if (choice === 'Move Forward') {
                moveForward(maxSpeed / 2);
            } else if (choice === 'Move Backward') {
                moveBackward(maxSpeed / 2);
            } else if (choice === 'Turn Left') {
                turnLeft(maxSpeed / 2);
            } else if (choice === 'Turn Right') {
                turnRight(maxSpeed / 2);
            } else if (choice === 'Spin Left') {
                spinLeft(maxSpeed / 2);
            } else if (choice === 'Spin Right') {
                spinRight(maxSpeed / 2);
            } else if (choice === 'Stop All') {
                stopAllMotors();
            } else if (choice === 'Save Config') {
                saveMotorConfig();
            } else if (choice === 'Load Config') {
                loadMotorConfig();
            } else {
                for (var i = 0; i < 4; i++) {
                    if (choice === 'Motor ' + (i + 1) + ' + Speed') {
                        moveMotor(i, Math.min(motorSpeeds[i] + 20, maxSpeed), motorDirections[i], acceleration);
                    } else if (choice === 'Motor ' + (i + 1) + ' - Speed') {
                        moveMotor(i, Math.max(motorSpeeds[i] - 20, 0), motorDirections[i], acceleration);
                    } else if (choice === 'Motor ' + (i + 1) + ' Forward') {
                        moveMotor(i, motorSpeeds[i], 1, acceleration);
                    } else if (choice === 'Motor ' + (i + 1) + ' Backward') {
                        moveMotor(i, motorSpeeds[i], -1, acceleration);
                    } else if (choice === 'Motor ' + (i + 1) + ' Stop') {
                        moveMotor(i, 0, 0, acceleration);
                    }
                }
            }
        }

        display.draw();
        delay(100);
    }
}

main();
