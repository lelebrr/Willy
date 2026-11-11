var dialog = require('dialog');
var wifi = require('wifi');
var display = require('display');
var keyboard = require('keyboard');
var storage = require('storage');

var tftWidth = display.width();
var tftHeight = display.height();

var currentFile = '';
var fileContent = '';
var cursorPos = 0;
var scrollOffset = 0;

function drawCodeEditor(title) {
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

function drawCode() {
    display.setTextSize(1);
    display.setTextAlign('left', 'top');

    var lines = fileContent.split('\n');
    var visibleLines = [];
    var maxVisibleLines = 20;

    for (var i = scrollOffset; i < lines.length && i < scrollOffset + maxVisibleLines; i++) {
        visibleLines.push(lines[i]);
    }

    var y = 40;
    for (var i = 0; i < visibleLines.length; i++) {
        var line = visibleLines[i];
        var lineNum = i + scrollOffset + 1;

        display.drawText(lineNum.toString().padStart(3, ' ') + ' ', 10, y);
        display.drawText(line, 40, y);

        y += 15;
    }

    var cursorLine = Math.floor(cursorPos / (fileContent.indexOf('\n', cursorPos) + 1));
    var cursorLineOffset = cursorPos - fileContent.lastIndexOf('\n', cursorPos - 1) - 1;
    var cursorY = 40 + (cursorLine - scrollOffset) * 15;
    var cursorX = 40 + cursorLineOffset * 6;

    display.drawRect(cursorX, cursorY, 2, 12);
}

function saveFile() {
    try {
        storage.write(currentFile, fileContent, 'write');
        console.log('File saved: ' + currentFile);
    } catch (error) {
        console.log(JSON.stringify(error));
    }
}

function loadFile() {
    var filename = prompt('Enter filename to load:');
    if (filename) {
        try {
            fileContent = storage.read(filename);
            currentFile = filename;
            cursorPos = 0;
            scrollOffset = 0;
            console.log('File loaded: ' + filename);
        } catch (error) {
            console.log(JSON.stringify(error));
            fileContent = '';
        }
    }
}

function newFile() {
    currentFile = '';
    fileContent = '';
    cursorPos = 0;
    scrollOffset = 0;
}

function main() {
    var running = true;

    while (running) {
        drawCodeEditor('Code Editor');
        drawCode();

        if (keyboard.getSelPress()) {
            var choices = ['New File', 'Load File', 'Save File', 'Quit'];
            var choice = dialog.choice(choices);

            if (choice === 'Quit') {
                running = false;
            } else if (choice === 'New File') {
                newFile();
            } else if (choice === 'Load File') {
                loadFile();
            } else if (choice === 'Save File') {
                saveFile();
            }
        }

        if (keyboard.getPrevPress()) {
            if (cursorPos > 0) {
                cursorPos--;
                if (cursorPos < scrollOffset * 80) {
                    scrollOffset--;
                }
            }
        }

        if (keyboard.getNextPress()) {
            if (cursorPos < fileContent.length) {
                cursorPos++;
                if (cursorPos > (scrollOffset + 20) * 80) {
                    scrollOffset++;
                }
            }
        }

        if (keyboard.getUpPress()) {
            var newPos = fileContent.lastIndexOf('\n', cursorPos - 1);
            if (newPos === -1) newPos = 0;
            cursorPos = newPos;
            if (cursorPos < scrollOffset * 80) {
                scrollOffset--;
            }
        }

        if (keyboard.getDownPress()) {
            var newPos = fileContent.indexOf('\n', cursorPos + 1);
            if (newPos === -1) newPos = fileContent.length;
            cursorPos = newPos + 1;
            if (cursorPos > (scrollOffset + 20) * 80) {
                scrollOffset++;
            }
        }

        display.draw();
        delay(50);
    }
}

main();
