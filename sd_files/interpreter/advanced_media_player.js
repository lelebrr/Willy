var dialog = require('dialog');
var wifi = require('wifi');
var display = require('display');
var keyboard = require('keyboard');
var storage = require('storage');

var tftWidth = display.width();
var tftHeight = display.height();

var mediaFiles = [];
var currentFile = null;
var isPlaying = false;
var currentTime = 0;
var duration = 0;
var volume = 50;
var playMode = 'sequential';

function drawMediaPlayer(title) {
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

function loadMediaFiles() {
    try {
        mediaFiles = storage.list('/media');
        console.log('Media files loaded: ' + mediaFiles.length);
    } catch (error) {
        console.log(JSON.stringify(error));
        mediaFiles = [];
    }
}

function drawMediaList() {
    display.setTextSize(1);
    display.setTextAlign('left', 'top');

    var y = 40;
    for (var i = 0; i < mediaFiles.length && i < 10; i++) {
        var file = mediaFiles[i];
        var name = file.name || 'Unknown';
        var type = file.type || 'Unknown';
        var size = file.size || 0;

        display.drawText(name, 10, y);
        display.drawText(type, 10, y + 10);
        display.drawText(size + ' bytes', 10, y + 20);

        y += 35;
    }
}

function drawPlayerControls() {
    if (!currentFile) return;

    display.setTextSize(1);
    display.setTextAlign('left', 'top');

    var y = tftHeight - 100;

    display.drawText('Now Playing: ' + currentFile.name, 10, y);
    y += 20;

    display.drawText('Time: ' + currentTime + 's / ' + duration + 's', 10, y);
    y += 20;

    display.drawText('Volume: ' + volume + '%', 10, y);
    y += 20;

    display.drawText('Mode: ' + playMode, 10, y);

    var playButton = isPlaying ? 'Pause' : 'Play';
    display.drawText(playButton, tftWidth - 60, y);
}

function playFile(file) {
    try {
        currentFile = file;
        isPlaying = true;
        currentTime = 0;
        duration = Math.floor(Math.random() * 300) + 60;
        console.log('Playing: ' + file.name);
    } catch (error) {
        console.log(JSON.stringify(error));
    }
}

function pauseFile() {
    isPlaying = false;
    console.log('Paused');
}

function stopFile() {
    isPlaying = false;
    currentTime = 0;
    console.log('Stopped');
}

function nextFile() {
    if (!currentFile) return;

    var currentIndex = mediaFiles.indexOf(currentFile);
    if (currentIndex !== -1) {
        var nextIndex = (currentIndex + 1) % mediaFiles.length;
        playFile(mediaFiles[nextIndex]);
    }
}

function prevFile() {
    if (!currentFile) return;

    var currentIndex = mediaFiles.indexOf(currentFile);
    if (currentIndex !== -1) {
        var prevIndex = (currentIndex - 1 + mediaFiles.length) % mediaFiles.length;
        playFile(mediaFiles[prevIndex]);
    }
}

function main() {
    loadMediaFiles();

    var running = true;
    var lastUpdate = 0;

    while (running) {
        drawMediaPlayer('Media Player');
        drawMediaList();
        drawPlayerControls();

        if (isPlaying && Date.now() - lastUpdate >= 1000) {
            currentTime++;
            lastUpdate = Date.now();

            if (currentTime >= duration) {
                if (playMode === 'sequential') {
                    nextFile();
                } else if (playMode === 'loop') {
                    currentTime = 0;
                } else if (playMode === 'random') {
                    var randomIndex = Math.floor(Math.random() * mediaFiles.length);
                    playFile(mediaFiles[randomIndex]);
                }
            }
        }

        if (keyboard.getSelPress()) {
            var choices = ['Play', 'Pause', 'Stop', 'Next', 'Previous', 'Volume', 'Mode', 'Quit'];
            var choice = dialog.choice(choices);

            if (choice === 'Quit') {
                running = false;
            } else if (choice === 'Play') {
                if (currentFile) {
                    playFile(currentFile);
                }
            } else if (choice === 'Pause') {
                pauseFile();
            } else if (choice === 'Stop') {
                stopFile();
            } else if (choice === 'Next') {
                nextFile();
            } else if (choice === 'Previous') {
                prevFile();
            } else if (choice === 'Volume') {
                volume = (volume + 10) % 100;
            } else if (choice === 'Mode') {
                var modes = ['sequential', 'loop', 'random'];
                var currentIndex = modes.indexOf(playMode);
                playMode = modes[(currentIndex + 1) % modes.length];
            }
        }

        display.draw();
        delay(100);
    }
}

main();
