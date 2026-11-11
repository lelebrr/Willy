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
var clipboard = '';
var searchMode = false;
var searchText = '';

function drawTextEditor(title) {
    display.fill(0);
    display.drawRoundRect(
        5,
        5,
        tftWidth - 10,
        tftHeight - 10,
        5,
        WILLY_PRICOLOR
    );
    display.setTextSize(1);
    display.setTextAlign('left', 'top');
}

function drawTextContent() {
    var lines = fileContent.split('\n');
    var visibleLines = [];
    var maxVisibleLines = 25;

    for (var i = scrollOffset; i < lines.length && i < scrollOffset + maxVisibleLines; i++) {
        visibleLines.push(lines[i]);
    }

    var y = 10;
    for (var i = 0; i < visibleLines.length; i++) {
        var line = visibleLines[i];
        var lineNum = i + scrollOffset + 1;

        display.drawText(lineNum.toString().padStart(3, ' ') + ' ', 10, y);
        display.drawText(line, 40, y);

        y += 15;
    }

    var cursorLine = Math.floor(cursorPos / (fileContent.indexOf('\n', cursorPos) + 1));
    var cursorLineOffset = cursorPos - fileContent.lastIndexOf('\n', cursorPos - 1) - 1;
    var cursorY = 10 + (cursorLine - scrollOffset) * 15;
    var cursorX = 40 + cursorLineOffset * 6;

    display.drawRect(cursorX, cursorY, 2, 12);

    if (searchMode) {
        display.drawText('Search: ' + searchText, 10, tftHeight - 30);
    }
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

function copyText() {
    var start = fileContent.lastIndexOf('\n', cursorPos - 1) + 1;
    var end = fileContent.indexOf('\n', cursorPos);
    if (end === -1) end = fileContent.length;

    clipboard = fileContent.substring(start, end);
    console.log('Text copied to clipboard');
}

function pasteText() {
    if (clipboard) {
        fileContent = fileContent.substring(0, cursorPos) + clipboard + fileContent.substring(cursorPos);
        cursorPos += clipboard.length;
        console.log('Text pasted from clipboard');
    }
}

function cutText() {
    copyText();
    var start = fileContent.lastIndexOf('\n', cursorPos - 1) + 1;
    var end = fileContent.indexOf('\n', cursorPos);
    if (end === -1) end = fileContent.length;

    fileContent = fileContent.substring(0, start) + fileContent.substring(end);
    cursorPos = start;
    console.log('Text cut to clipboard');
}

function findText() {
    searchMode = true;
    searchText = '';
}

function nextMatch() {
    if (!searchText) return;

    var pos = fileContent.indexOf(searchText, cursorPos + 1);
    if (pos !== -1) {
        cursorPos = pos;
        var line = Math.floor(cursorPos / (fileContent.indexOf('\n', cursorPos) + 1));
        if (line < scrollOffset || line >= scrollOffset + 25) {
            scrollOffset = line - 12;
        }
    }
}

function main() {
    var running = true;

    while (running) {
        drawTextEditor('Text Editor');
        drawTextContent();

        if (keyboard.getSelPress()) {
            var choices = ['New', 'Load', 'Save', 'Copy', 'Paste', 'Cut', 'Find', 'Next', 'Quit'];
            var choice = dialog.choice(choices);

            if (choice === 'Quit') {
                running = false;
            } else if (choice === 'New') {
                newFile();
            } else if (choice === 'Load') {
                loadFile();
            } else if (choice === 'Save') {
                saveFile();
            } else if (choice === 'Copy') {
                copyText();
            } else if (choice === 'Paste') {
                pasteText();
            } else if (choice === 'Cut') {
                cutText();
            } else if (choice === 'Find') {
                findText();
            } else if (choice === 'Next') {
                nextMatch();
            }
        }

        if (searchMode && keyboard.getChar()) {
            var char = keyboard.getChar();
            if (char === '\n') {
                searchMode = false;
                nextMatch();
            } else if (char === '\b') {
                searchText = searchText.substring(0, searchText.length - 1);
            } else if (char.length === 1) {
                searchText += char;
            }
        }

        if (!searchMode && keyboard.getChar()) {
            var char = keyboard.getChar();
            if (char === '\n') {
                fileContent = fileContent.substring(0, cursorPos) + '\n' + fileContent.substring(cursorPos);
                cursorPos++;
            } else if (char === '\b') {
                if (cursorPos > 0) {
                    fileContent = fileContent.substring(0, cursorPos - 1) + fileContent.substring(cursorPos);
                    cursorPos--;
                }
            } else if (char.length === 1) {
                fileContent = fileContent.substring(0, cursorPos) + char + fileContent.substring(cursorPos);
                cursorPos++;
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
                if (cursorPos > (scrollOffset + 25) * 80) {
                    scrollOffset++;
                }
            }
        }

        display.draw();
        delay(50);
    }
}

main();
