var dialog = require('dialog');
var wifi = require('wifi');
var display = require('display');
var keyboard = require('keyboard');
var storage = require('storage');

var tftWidth = display.width();
var tftHeight = display.height();

var timers = [];
var currentTimer = null;
var currentTimerIndex = -1;
var timerTypes = ['Countdown', 'Stopwatch', 'Interval'];
var currentType = 'Countdown';
var isRunning = false;
var startTime = 0;
var elapsedTime = 0;
var targetTime = 0;

function drawTimer(title) {
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

function drawTimersList() {
    display.setTextSize(1);
    display.setTextAlign('left', 'top');

    var y = 40;
    for (var i = 0; i < timers.length && i < 12; i++) {
        var timer = timers[i];
        var name = timer.name || 'Untitled Timer';
        var type = timer.type || 'Countdown';
        var duration = timer.duration || 0;
        var remaining = timer.remaining || 0;

        display.drawText(name, 10, y);
        display.drawText(type + ' - ' + duration + 's', 10, y + 10);
        display.drawText('Remaining: ' + remaining + 's', 10, y + 20);

        y += 35;
    }
}

function drawActiveTimer() {
    if (!currentTimer) return;

    display.setTextSize(3);
    display.setTextAlign('center', 'top');

    var currentTime = isRunning ? Date.now() - startTime : elapsedTime;
    var remaining = currentTimer.type === 'Countdown' ? Math.max(0, currentTimer.duration - currentTime) : currentTime;

    var timeText = '';
    if (currentTimer.type === 'Countdown') {
        var minutes = Math.floor(remaining / 60);
        var seconds = Math.floor(remaining % 60);
        timeText = minutes.toString().padStart(2, '0') + ':' + seconds.toString().padStart(2, '0');
    } else if (currentTimer.type === 'Stopwatch') {
        var minutes = Math.floor(currentTime / 60000);
        var seconds = Math.floor((currentTime % 60000) / 1000);
        var milliseconds = Math.floor((currentTime % 1000) / 10);
        timeText = minutes.toString().padStart(2, '0') + ':' + seconds.toString().padStart(2, '0') + '.' + milliseconds.toString().padStart(2, '0');
    } else if (currentTimer.type === 'Interval') {
        var intervals = Math.floor(currentTime / currentTimer.interval);
        var remaining = currentTime % currentTimer.interval;
        timeText = intervals + ' intervals (' + (remaining / 1000).toFixed(1) + 's)';
    }

    display.drawText(timeText, tftWidth / 2, 40);

    display.setTextSize(1);
    display.setTextAlign('left', 'top');
    display.drawText('Type: ' + currentTimer.type, 10, tftHeight - 60);
    display.drawText('Status: ' + (isRunning ? 'Running' : 'Stopped'), 10, tftHeight - 40);
    display.drawText('Name: ' + currentTimer.name, 10, tftHeight - 20);
}

function createTimer() {
    var name = prompt('Enter timer name:');
    if (name) {
        var duration = parseInt(prompt('Enter duration in seconds:'));
        if (!isNaN(duration) && duration > 0) {
            var timer = {
                name: name,
                type: currentType,
                duration: duration,
                interval: duration,
                remaining: duration,
                createdAt: new Date().toISOString()
            };

            timers.push(timer);
            saveTimers();
            console.log('Timer created: ' + name);
        }
    }
}

function startTimer() {
    if (currentTimer && !isRunning) {
        isRunning = true;
        startTime = Date.now() - elapsedTime;
        console.log('Timer started: ' + currentTimer.name);
    }
}

function stopTimer() {
    if (currentTimer && isRunning) {
        isRunning = false;
        elapsedTime = Date.now() - startTime;
        currentTimer.remaining = currentTimer.type === 'Countdown' ?
            Math.max(0, currentTimer.duration - elapsedTime) : elapsedTime;
        saveTimers();
        console.log('Timer stopped: ' + currentTimer.name);
    }
}

function resetTimer() {
    if (currentTimer) {
        isRunning = false;
        elapsedTime = 0;
        currentTimer.remaining = currentTimer.duration;
        saveTimers();
        console.log('Timer reset: ' + currentTimer.name);
    }
}

function deleteTimer() {
    if (currentTimerIndex !== -1) {
        timers.splice(currentTimerIndex, 1);
        currentTimer = null;
        currentTimerIndex = -1;
        saveTimers();
        console.log('Timer deleted');
    }
}

function saveTimers() {
    try {
        storage.write('timers_data', JSON.stringify(timers), 'write');
        console.log('Timers saved successfully');
    } catch (error) {
        console.log(JSON.stringify(error));
    }
}

function loadTimers() {
    try {
        var data = storage.read('timers_data');
        if (data) {
            timers = JSON.parse(data);
            console.log('Timers loaded successfully');
        }
    } catch (error) {
        console.log(JSON.stringify(error));
        timers = [];
    }
}

function updateTimer() {
    if (currentTimer && isRunning) {
        elapsedTime = Date.now() - startTime;
        currentTimer.remaining = currentTimer.type === 'Countdown' ?
            Math.max(0, currentTimer.duration - elapsedTime) : elapsedTime;

        if (currentTimer.type === 'Countdown' && currentTimer.remaining <= 0) {
            isRunning = false;
            console.log('Timer completed: ' + currentTimer.name);
        }
    }
}

function main() {
    loadTimers();

    var running = true;
    var viewMode = 'list';

    while (running) {
        drawTimer('Timer');

        if (viewMode === 'list') {
            drawTimersList();
        } else if (viewMode === 'active') {
            drawActiveTimer();
        }

        if (viewMode === 'active') {
            updateTimer();
        }

        if (keyboard.getSelPress()) {
            var choices = [];

            if (viewMode === 'list') {
                choices = ['Create Timer', 'Select Timer', 'Delete Timer', 'Change Type', 'Quit'];
            } else if (viewMode === 'active') {
                choices = ['Start', 'Stop', 'Reset', 'Back to List', 'Delete Timer', 'Quit'];
            }

            var choice = dialog.choice(choices);

            if (choice === 'Quit') {
                running = false;
            } else if (choice === 'Create Timer' && viewMode === 'list') {
                createTimer();
            } else if (choice === 'Select Timer' && viewMode === 'list') {
                var timerIndex = parseInt(prompt('Enter timer index:'));
                if (!isNaN(timerIndex) && timerIndex >= 0 && timerIndex < timers.length) {
                    currentTimer = timers[timerIndex];
                    currentTimerIndex = timerIndex;
                    elapsedTime = 0;
                    isRunning = false;
                    viewMode = 'active';
                }
            } else if (choice === 'Delete Timer' && viewMode === 'list') {
                var timerIndex = parseInt(prompt('Enter timer index to delete:'));
                if (!isNaN(timerIndex) && timerIndex >= 0 && timerIndex < timers.length) {
                    timers.splice(timerIndex, 1);
                    saveTimers();
                }
            } else if (choice === 'Change Type' && viewMode === 'list') {
                var typeIndex = timerTypes.indexOf(currentType);
                currentType = timerTypes[(typeIndex + 1) % timerTypes.length];
            } else if (choice === 'Start' && viewMode === 'active') {
                startTimer();
            } else if (choice === 'Stop' && viewMode === 'active') {
                stopTimer();
            } else if (choice === 'Reset' && viewMode === 'active') {
                resetTimer();
            } else if (choice === 'Back to List' && viewMode === 'active') {
                viewMode = 'list';
            } else if (choice === 'Delete Timer' && viewMode === 'active') {
                deleteTimer();
                viewMode = 'list';
            }
        }

        display.draw();
        delay(100);
    }
}

main();
