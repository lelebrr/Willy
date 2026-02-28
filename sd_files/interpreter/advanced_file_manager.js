var dialog = require('dialog');
var wifi = require('wifi');
var display = require('display');
var keyboard = require('keyboard');
var storage = require('storage');

var tftWidth = display.width();
var tftHeight = display.height();

var currentPath = '/';
var files = [];
var selectedFile = null;
var fileHistory = [];
var clipboard = null;
var clipboardAction = null;

function drawFileManager(title) {
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

function loadFiles() {
    try {
        files = storage.list(currentPath);
        console.log('Files loaded for path: ' + currentPath);
    } catch (error) {
        console.log(JSON.stringify(error));
        files = [];
    }
}

function drawFilesList() {
    display.setTextSize(1);
    display.setTextAlign('left', 'top');

    var y = 40;
    for (var i = 0; i < files.length && i < 15; i++) {
        var file = files[i];
        var name = file.name || 'Unknown';
        var type = file.type || 'Unknown';
        var size = file.size || 0;
        var modified = file.modified || new Date().toISOString();

        var icon = type === 'directory' ? '📁' : '📄';

        display.drawText(icon + ' ' + name, 10, y);
        display.drawText(size + ' bytes', 10, y + 10);
        display.drawText(modified.substring(0, 10), 10, y + 20);

        y += 35;
    }
}

function navigateTo(path) {
    fileHistory.push(currentPath);
    currentPath = path;
    loadFiles();
}

function goBack() {
    if (fileHistory.length > 0) {
        currentPath = fileHistory.pop();
        loadFiles();
    }
}

function createDirectory() {
    var name = prompt('Enter directory name:');
    if (name) {
        try {
            storage.createDirectory(currentPath + '/' + name);
            loadFiles();
            console.log('Directory created: ' + name);
        } catch (error) {
            console.log(JSON.stringify(error));
        }
    }
}

function createFile() {
    var name = prompt('Enter file name:');
    if (name) {
        try {
            storage.write(currentPath + '/' + name, '', 'write');
            loadFiles();
            console.log('File created: ' + name);
        } catch (error) {
            console.log(JSON.stringify(error));
        }
    }
}

function deleteFile() {
    if (!selectedFile) return;

    try {
        storage.delete(currentPath + '/' + selectedFile.name);
        loadFiles();
        selectedFile = null;
        console.log('File deleted: ' + selectedFile.name);
    } catch (error) {
        console.log(JSON.stringify(error));
    }
}

function copyFile() {
    if (!selectedFile) return;

    clipboard = selectedFile;
    clipboardAction = 'copy';
    console.log('File copied to clipboard: ' + selectedFile.name);
}

function cutFile() {
    if (!selectedFile) return;

    clipboard = selectedFile;
    clipboardAction = 'cut';
    console.log('File cut to clipboard: ' + selectedFile.name);
}

function pasteFile() {
    if (!clipboard) return;

    try {
        if (clipboardAction === 'copy') {
            var sourcePath = currentPath + '/' + clipboard.name;
            var destPath = currentPath + '/' + clipboard.name;
            storage.copy(sourcePath, destPath);
        } else if (clipboardAction === 'cut') {
            var sourcePath = currentPath + '/' + clipboard.name;
            var destPath = currentPath + '/' + clipboard.name;
            storage.move(sourcePath, destPath);
        }

        loadFiles();
        clipboard = null;
        clipboardAction = null;
        console.log('File pasted: ' + clipboard.name);
    } catch (error) {
        console.log(JSON.stringify(error));
    }
}

function renameFile() {
    if (!selectedFile) return;

    var newName = prompt('Enter new name:');
    if (newName) {
        try {
            storage.rename(currentPath + '/' + selectedFile.name, newName);
            loadFiles();
            selectedFile = null;
            console.log('File renamed: ' + newName);
        } catch (error) {
            console.log(JSON.stringify(error));
        }
    }
}

function viewFile() {
    if (!selectedFile) return;

    try {
        var content = storage.read(currentPath + '/' + selectedFile.name);
        console.log('File content: ' + content);
    } catch (error) {
        console.log(JSON.stringify(error));
    }
}

function editFile() {
    if (!selectedFile) return;

    try {
        var content = storage.read(currentPath + '/' + selectedFile.name);
        var newContent = prompt('Edit file content:', content);
        if (newContent !== null) {
            storage.write(currentPath + '/' + selectedFile.name, newContent, 'write');
            console.log('File edited: ' + selectedFile.name);
        }
    } catch (error) {
        console.log(JSON.stringify(error));
    }
}

function getFileProperties() {
    if (!selectedFile) return;

    display.setTextSize(1);
    display.setTextAlign('left', 'top');

    var y = 40;
    display.drawText('Name: ' + selectedFile.name, 10, y);
    y += 20;
    display.drawText('Type: ' + selectedFile.type, 10, y);
    y += 20;
    display.drawText('Size: ' + selectedFile.size + ' bytes', 10, y);
    y += 20;
    display.drawText('Modified: ' + selectedFile.modified, 10, y);
    y += 20;
    display.drawText('Path: ' + currentPath + '/' + selectedFile.name, 10, y);
}

function searchFiles() {
    var searchTerm = prompt('Enter search term:');
    if (searchTerm) {
        try {
            var results = storage.search(currentPath, searchTerm);
            display.setTextSize(1);
            display.setTextAlign('left', 'top');
            display.drawText('Search Results:', 10, 40);

            var y = 60;
            for (var i = 0; i < results.length && i < 10; i++) {
                var file = results[i];
                display.drawText(file.name, 10, y);
                y += 20;
            }
        } catch (error) {
            console.log(JSON.stringify(error));
        }
    }
}

function exportFiles() {
    try {
        var exportPath = prompt('Enter export path:');
        if (exportPath) {
            storage.export(currentPath, exportPath);
            console.log('Files exported to: ' + exportPath);
        }
    } catch (error) {
        console.log(JSON.stringify(error));
    }
}

function main() {
    loadFiles();

    var running = true;

    while (running) {
        drawFileManager('File Manager');
        drawFilesList();

        if (keyboard.getSelPress()) {
            var choices = ['Create Directory', 'Create File', 'Select File', 'Delete File', 'Copy File', 'Cut File', 'Paste File', 'Rename File', 'View File', 'Edit File', 'File Properties', 'Search Files', 'Export Files', 'Go Back', 'Quit'];
            var choice = dialog.choice(choices);

            if (choice === 'Quit') {
                running = false;
            } else if (choice === 'Create Directory') {
                createDirectory();
            } else if (choice === 'Create File') {
                createFile();
            } else if (choice === 'Select File') {
                var fileIndex = parseInt(prompt('Enter file index:'));
                if (!isNaN(fileIndex) && fileIndex >= 0 && fileIndex < files.length) {
                    selectedFile = files[fileIndex];
                }
            } else if (choice === 'Delete File') {
                deleteFile();
            } else if (choice === 'Copy File') {
                copyFile();
            } else if (choice === 'Cut File') {
                cutFile();
            } else if (choice === 'Paste File') {
                pasteFile();
            } else if (choice === 'Rename File') {
                renameFile();
            } else if (choice === 'View File') {
                viewFile();
            } else if (choice === 'Edit File') {
                editFile();
            } else if (choice === 'File Properties') {
                getFileProperties();
            } else if (choice === 'Search Files') {
                searchFiles();
            } else if (choice === 'Export Files') {
                exportFiles();
            } else if (choice === 'Go Back') {
                goBack();
            }
        }

        if (keyboard.getPrevPress()) {
            goBack();
        }

        display.draw();
        delay(100);
    }
}

main();
