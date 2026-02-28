var dialog = require('dialog');
var wifi = require('wifi');
var display = require('display');
var keyboard = require('keyboard');
var storage = require('storage');

var tftWidth = display.width();
var tftHeight = display.height();

var notes = [];
var currentNote = null;
var currentNoteIndex = -1;
var noteContent = '';
var cursorPos = 0;
var scrollOffset = 0;
var categories = ['Personal', 'Work', 'Ideas', 'Important'];
var currentCategory = 'Personal';

function drawNoteTaker(title) {
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

function drawNotesList() {
    display.setTextSize(1);
    display.setTextAlign('left', 'top');

    var y = 40;
    for (var i = 0; i < notes.length && i < 15; i++) {
        var note = notes[i];
        var title = note.title || 'Untitled';
        var category = note.category || 'Uncategorized';
        var date = note.date || new Date().toISOString();

        display.drawText(title, 10, y);
        display.drawText(category, 10, y + 10);
        display.drawText(date.substring(0, 10), 10, y + 20);

        y += 35;
    }
}

function drawNoteContent() {
    if (!currentNote) return;

    display.setTextSize(1);
    display.setTextAlign('left', 'top');

    var lines = noteContent.split('\n');
    var visibleLines = [];
    var maxVisibleLines = 20;

    for (var i = scrollOffset; i < lines.length && i < scrollOffset + maxVisibleLines; i++) {
        visibleLines.push(lines[i]);
    }

    var y = 40;
    for (var i = 0; i < visibleLines.length; i++) {
        var line = visibleLines[i];
        display.drawText(line, 10, y);
        y += 15;
    }

    var cursorLine = Math.floor(cursorPos / (noteContent.indexOf('\n', cursorPos) + 1));
    var cursorLineOffset = cursorPos - noteContent.lastIndexOf('\n', cursorPos - 1) - 1;
    var cursorY = 40 + (cursorLine - scrollOffset) * 15;
    var cursorX = 10 + cursorLineOffset * 6;

    display.drawRect(cursorX, cursorY, 2, 12);
}

function createNote() {
    var title = prompt('Enter note title:');
    if (title) {
        var note = {
            title: title,
            content: '',
            category: currentCategory,
            date: new Date().toISOString()
        };

        notes.push(note);
        currentNote = note;
        currentNoteIndex = notes.length - 1;
        noteContent = '';
        cursorPos = 0;
        scrollOffset = 0;

        saveNotes();
        console.log('Note created: ' + title);
    }
}

function deleteNote() {
    if (currentNoteIndex !== -1) {
        notes.splice(currentNoteIndex, 1);
        currentNote = null;
        currentNoteIndex = -1;
        noteContent = '';

        saveNotes();
        console.log('Note deleted');
    }
}

function saveNotes() {
    try {
        storage.write('notes_data', JSON.stringify(notes), 'write');
        console.log('Notes saved successfully');
    } catch (error) {
        console.log(JSON.stringify(error));
    }
}

function loadNotes() {
    try {
        var data = storage.read('notes_data');
        if (data) {
            notes = JSON.parse(data);
            console.log('Notes loaded successfully');
        }
    } catch (error) {
        console.log(JSON.stringify(error));
        notes = [];
    }
}

function filterNotesByCategory() {
    var filteredNotes = notes.filter(note => note.category === currentCategory);
    return filteredNotes;
}

function main() {
    loadNotes();

    var running = true;
    var viewMode = 'list';

    while (running) {
        drawNoteTaker('Note Taker');

        if (viewMode === 'list') {
            var filteredNotes = filterNotesByCategory();
            display.setTextSize(1);
            display.setTextAlign('left', 'top');
            display.drawText('Category: ' + currentCategory, 10, 40);

            var y = 60;
            for (var i = 0; i < filteredNotes.length && i < 12; i++) {
                var note = filteredNotes[i];
                var title = note.title || 'Untitled';
                var date = note.date || new Date().toISOString();

                display.drawText(title, 10, y);
                display.drawText(date.substring(0, 10), 10, y + 10);

                y += 25;
            }
        } else if (viewMode === 'content') {
            drawNoteContent();
        }

        if (keyboard.getSelPress()) {
            var choices = [];

            if (viewMode === 'list') {
                choices = ['Create Note', 'Select Note', 'Delete Note', 'Change Category', 'Quit'];
            } else if (viewMode === 'content') {
                choices = ['Save Note', 'Back to List', 'Delete Note', 'Quit'];
            }

            var choice = dialog.choice(choices);

            if (choice === 'Quit') {
                running = false;
            } else if (choice === 'Create Note' && viewMode === 'list') {
                createNote();
                viewMode = 'content';
            } else if (choice === 'Select Note' && viewMode === 'list') {
                var noteIndex = parseInt(prompt('Enter note index:'));
                if (!isNaN(noteIndex) && noteIndex >= 0 && noteIndex < filteredNotes.length) {
                    currentNote = filteredNotes[noteIndex];
                    currentNoteIndex = notes.indexOf(currentNote);
                    noteContent = currentNote.content || '';
                    cursorPos = 0;
                    scrollOffset = 0;
                    viewMode = 'content';
                }
            } else if (choice === 'Delete Note' && viewMode === 'list') {
                var noteIndex = parseInt(prompt('Enter note index to delete:'));
                if (!isNaN(noteIndex) && noteIndex >= 0 && noteIndex < filteredNotes.length) {
                    var noteToDelete = filteredNotes[noteIndex];
                    var globalIndex = notes.indexOf(noteToDelete);
                    if (globalIndex !== -1) {
                        notes.splice(globalIndex, 1);
                        saveNotes();
                    }
                }
            } else if (choice === 'Change Category' && viewMode === 'list') {
                var categoryIndex = categories.indexOf(currentCategory);
                currentCategory = categories[(categoryIndex + 1) % categories.length];
            } else if (choice === 'Save Note' && viewMode === 'content') {
                currentNote.content = noteContent;
                saveNotes();
                console.log('Note saved');
            } else if (choice === 'Back to List' && viewMode === 'content') {
                viewMode = 'list';
            } else if (choice === 'Delete Note' && viewMode === 'content') {
                if (currentNoteIndex !== -1) {
                    notes.splice(currentNoteIndex, 1);
                    currentNote = null;
                    currentNoteIndex = -1;
                    noteContent = '';
                    saveNotes();
                    viewMode = 'list';
                }
            }
        }

        if (viewMode === 'content' && keyboard.getChar()) {
            var char = keyboard.getChar();
            if (char === '\n') {
                noteContent = noteContent.substring(0, cursorPos) + '\n' + noteContent.substring(cursorPos);
                cursorPos++;
            } else if (char === '\b') {
                if (cursorPos > 0) {
                    noteContent = noteContent.substring(0, cursorPos - 1) + noteContent.substring(cursorPos);
                    cursorPos--;
                }
            } else if (char.length === 1) {
                noteContent = noteContent.substring(0, cursorPos) + char + noteContent.substring(cursorPos);
                cursorPos++;
            }
        }

        if (viewMode === 'content' && keyboard.getPrevPress()) {
            if (cursorPos > 0) {
                cursorPos--;
                if (cursorPos < scrollOffset * 80) {
                    scrollOffset--;
                }
            }
        }

        if (viewMode === 'content' && keyboard.getNextPress()) {
            if (cursorPos < noteContent.length) {
                cursorPos++;
                if (cursorPos > (scrollOffset + 20) * 80) {
                    scrollOffset++;
                }
            }
        }

        display.draw();
        delay(50);
    }
}

main();
