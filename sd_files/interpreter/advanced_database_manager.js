var dialog = require('dialog');
var wifi = require('wifi');
var display = require('display');
var keyboard = require('keyboard');
var storage = require('storage');

var tftWidth = display.width();
var tftHeight = display.height();

var databases = [];
var currentDatabase = null;
var tables = [];
var currentTable = null;
var records = [];
var currentRecord = null;

function drawDatabaseManager(title) {
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

function loadDatabases() {
    try {
        databases = storage.list('/databases');
        console.log('Databases loaded: ' + databases.length);
    } catch (error) {
        console.log(JSON.stringify(error));
        databases = [];
    }
}

function loadTables(database) {
    try {
        tables = storage.list('/databases/' + database);
        console.log('Tables loaded for ' + database + ': ' + tables.length);
    } catch (error) {
        console.log(JSON.stringify(error));
        tables = [];
    }
}

function loadRecords(database, table) {
    try {
        var data = storage.read('/databases/' + database + '/' + table);
        records = JSON.parse(data) || [];
        console.log('Records loaded for ' + database + '.' + table + ': ' + records.length);
    } catch (error) {
        console.log(JSON.stringify(error));
        records = [];
    }
}

function drawDatabases() {
    display.setTextSize(1);
    display.setTextAlign('left', 'top');

    var y = 40;
    for (var i = 0; i < databases.length && i < 10; i++) {
        var db = databases[i];
        display.drawText(db, 10, y);
        y += 20;
    }
}

function drawTables() {
    display.setTextSize(1);
    display.setTextAlign('left', 'top');

    var y = 40;
    for (var i = 0; i < tables.length && i < 10; i++) {
        var table = tables[i];
        display.drawText(table, 10, y);
        y += 20;
    }
}

function drawRecords() {
    display.setTextSize(1);
    display.setTextAlign('left', 'top');

    var y = 40;
    for (var i = 0; i < records.length && i < 10; i++) {
        var record = records[i];
        display.drawText(JSON.stringify(record), 10, y);
        y += 20;
    }
}

function createDatabase() {
    var name = prompt('Enter database name:');
    if (name) {
        try {
            storage.write('/databases/' + name, '', 'write');
            loadDatabases();
            console.log('Database created: ' + name);
        } catch (error) {
            console.log(JSON.stringify(error));
        }
    }
}

function createTable() {
    if (!currentDatabase) return;

    var name = prompt('Enter table name:');
    if (name) {
        try {
            storage.write('/databases/' + currentDatabase + '/' + name, '[]', 'write');
            loadTables(currentDatabase);
            console.log('Table created: ' + name);
        } catch (error) {
            console.log(JSON.stringify(error));
        }
    }
}

function insertRecord() {
    if (!currentDatabase || !currentTable) return;

    var data = prompt('Enter record data (JSON format):');
    if (data) {
        try {
            var record = JSON.parse(data);
            records.push(record);
            storage.write('/databases/' + currentDatabase + '/' + currentTable, JSON.stringify(records), 'write');
            console.log('Record inserted');
        } catch (error) {
            console.log(JSON.stringify(error));
        }
    }
}

function deleteRecord() {
    if (!currentDatabase || !currentTable || records.length === 0) return;

    var index = parseInt(prompt('Enter record index to delete:'));
    if (!isNaN(index) && index >= 0 && index < records.length) {
        records.splice(index, 1);
        storage.write('/databases/' + currentDatabase + '/' + currentTable, JSON.stringify(records), 'write');
        console.log('Record deleted');
    }
}

function main() {
    loadDatabases();

    var running = true;
    var viewMode = 'databases';

    while (running) {
        drawDatabaseManager('Database Manager');

        if (viewMode === 'databases') {
            drawDatabases();
        } else if (viewMode === 'tables') {
            drawTables();
        } else if (viewMode === 'records') {
            drawRecords();
        }

        if (keyboard.getSelPress()) {
            var choices = [];

            if (viewMode === 'databases') {
                choices = ['Create Database', 'Select Database', 'Quit'];
            } else if (viewMode === 'tables') {
                choices = ['Create Table', 'Select Table', 'Back', 'Quit'];
            } else if (viewMode === 'records') {
                choices = ['Insert Record', 'Delete Record', 'Back', 'Quit'];
            }

            var choice = dialog.choice(choices);

            if (choice === 'Quit') {
                running = false;
            } else if (choice === 'Create Database' && viewMode === 'databases') {
                createDatabase();
            } else if (choice === 'Select Database' && viewMode === 'databases') {
                var dbIndex = parseInt(prompt('Enter database index:'));
                if (!isNaN(dbIndex) && dbIndex >= 0 && dbIndex < databases.length) {
                    currentDatabase = databases[dbIndex];
                    loadTables(currentDatabase);
                    viewMode = 'tables';
                }
            } else if (choice === 'Create Table' && viewMode === 'tables') {
                createTable();
            } else if (choice === 'Select Table' && viewMode === 'tables') {
                var tableIndex = parseInt(prompt('Enter table index:'));
                if (!isNaN(tableIndex) && tableIndex >= 0 && tableIndex < tables.length) {
                    currentTable = tables[tableIndex];
                    loadRecords(currentDatabase, currentTable);
                    viewMode = 'records';
                }
            } else if ((choice === 'Insert Record' || choice === 'Delete Record') && viewMode === 'records') {
                if (choice === 'Insert Record') {
                    insertRecord();
                } else {
                    deleteRecord();
                }
            } else if (choice === 'Back') {
                if (viewMode === 'tables') {
                    viewMode = 'databases';
                } else if (viewMode === 'records') {
                    viewMode = 'tables';
                }
            }
        }

        display.draw();
        delay(100);
    }
}

main();
