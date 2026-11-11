var dialog = require('dialog');
var wifi = require('wifi');
var display = require('display');
var keyboard = require('keyboard');
var storage = require('storage');

var tftWidth = display.width();
var tftHeight = display.height();

var terminalHistory = [];
var currentCommand = '';
var cursorPos = 0;
var scrollOffset = 0;
var maxHistoryLines = 50;

function drawTerminal(title) {
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

function drawTerminalContent() {
    var y = 10;

    for (var i = scrollOffset; i < terminalHistory.length && i < scrollOffset + 30; i++) {
        var line = terminalHistory[i];
        display.drawText(line, 10, y);
        y += 15;
    }

    var promptY = 10 + (terminalHistory.length - scrollOffset) * 15;
    display.drawText('$ ', 10, promptY);
    display.drawText(currentCommand, 30, promptY);

    var cursorX = 30 + cursorPos * 6;
    display.drawRect(cursorX, promptY, 2, 12);
}

function executeCommand(command) {
    terminalHistory.push('$ ' + command);

    try {
        var result = '';

        if (command === 'help') {
            result = 'Available commands:\nhelp - Show this help\nls - List files\ncd <dir> - Change directory\ncat <file> - Read file\npwd - Show current directory\nclear - Clear terminal\nexit - Exit terminal';
        } else if (command === 'ls') {
            var files = storage.list('/');
            result = files.join('\n');
        } else if (command.startsWith('cd ')) {
            var dir = command.substring(3);
            result = 'Directory changed to: ' + dir;
        } else if (command.startsWith('cat ')) {
            var filename = command.substring(4);
            var content = storage.read(filename);
            result = content || 'File not found: ' + filename;
        } else if (command === 'pwd') {
            result = '/';
        } else if (command === 'clear') {
            terminalHistory = [];
            scrollOffset = 0;
            return;
        } else if (command === 'exit') {
            return 'exit';
        } else {
            result = 'Unknown command: ' + command;
        }

        terminalHistory.push(result);
    } catch (error) {
        terminalHistory.push('Error: ' + JSON.stringify(error));
    }

    if (terminalHistory.length > maxHistoryLines) {
        terminalHistory.shift();
        scrollOffset = Math.max(0, scrollOffset - 1);
    }
}

function main() {
    var running = true;

    while (running) {
        drawTerminal('Terminal');
        drawTerminalContent();

        if (keyboard.getSelPress()) {
            var command = currentCommand.trim();
            if (command) {
                var result = executeCommand(command);
                if (result === 'exit') {
                    running = false;
                }
                currentCommand = '';
                cursorPos = 0;
                scrollOffset = terminalHistory.length - 30;
            }
        }

        if (keyboard.getPrevPress()) {
            if (cursorPos > 0) {
                cursorPos--;
            }
        }

        if (keyboard.getNextPress()) {
            if (cursorPos < currentCommand.length) {
                cursorPos++;
            }
        }

        if (keyboard.getUpPress()) {
            if (terminalHistory.length > 0) {
                currentCommand = terminalHistory[terminalHistory.length - 1].substring(2);
                cursorPos = currentCommand.length;
            }
        }

        if (keyboard.getDownPress()) {
            currentCommand = '';
            cursorPos = 0;
        }

        if (keyboard.getChar()) {
            var char = keyboard.getChar();
            if (char === '\n') {
                var command = currentCommand.trim();
                if (command) {
                    var result = executeCommand(command);
                    if (result === 'exit') {
                        running = false;
                    }
                    currentCommand = '';
                    cursorPos = 0;
                    scrollOffset = terminalHistory.length - 30;
                }
            } else if (char === '\b') {
                if (cursorPos > 0) {
                    currentCommand = currentCommand.substring(0, cursorPos - 1) + currentCommand.substring(cursorPos);
                    cursorPos--;
                }
            } else if (char.length === 1) {
                currentCommand = currentCommand.substring(0, cursorPos) + char + currentCommand.substring(cursorPos);
                cursorPos++;
            }
        }

        display.draw();
        delay(50);
    }
}

main();
