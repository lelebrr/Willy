var dialog = require('dialog');
var wifi = require('wifi');
var display = require('display');
var keyboard = require('keyboard');
var storage = require('storage');

var tftWidth = display.width();
var tftHeight = display.height();

var currentUrl = 'https://example.com';
var pageContent = '';
var history = [];
var currentHistoryIndex = 0;
var bookmarks = [];
var isLoading = false;

function drawWebBrowser(title) {
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

function drawPageContent() {
    var lines = pageContent.split('\n');
    var visibleLines = [];
    var maxVisibleLines = 30;

    for (var i = 0; i < lines.length && i < maxVisibleLines; i++) {
        visibleLines.push(lines[i]);
    }

    var y = 10;
    for (var i = 0; i < visibleLines.length; i++) {
        var line = visibleLines[i];
        display.drawText(line, 10, y);
        y += 15;
    }

    if (isLoading) {
        display.drawText('Loading...', 10, tftHeight - 30);
    }
}

function drawNavigation() {
    display.setTextSize(1);
    display.setTextAlign('left', 'top');

    display.drawText('URL: ' + currentUrl, 10, tftHeight - 50);
    display.drawText('History: ' + (currentHistoryIndex + 1) + '/' + history.length, 10, tftHeight - 30);
}

function loadPage() {
    isLoading = true;

    try {
        var response = wifi.httpFetch(currentUrl, {
            method: 'GET'
        });

        pageContent = response.body || 'Page not found';
        history.push(currentUrl);
        currentHistoryIndex = history.length - 1;

        console.log('Page loaded: ' + currentUrl);
    } catch (error) {
        pageContent = 'Error loading page: ' + JSON.stringify(error);
        console.log(JSON.stringify(error));
    }

    isLoading = false;
}

function goBack() {
    if (currentHistoryIndex > 0) {
        currentHistoryIndex--;
        currentUrl = history[currentHistoryIndex];
        loadPage();
    }
}

function goForward() {
    if (currentHistoryIndex < history.length - 1) {
        currentHistoryIndex++;
        currentUrl = history[currentHistoryIndex];
        loadPage();
    }
}

function addBookmark() {
    bookmarks.push(currentUrl);
    console.log('Bookmark added: ' + currentUrl);
}

function showBookmarks() {
    var bookmarkList = bookmarks.map((url, index) => (index + 1) + '. ' + url).join('\n');
    display.drawText('Bookmarks:\n' + bookmarkList, 10, 10);
}

function main() {
    loadPage();

    var running = true;

    while (running) {
        drawWebBrowser('Web Browser');
        drawPageContent();
        drawNavigation();

        if (keyboard.getSelPress()) {
            var choices = ['Go Back', 'Go Forward', 'Add Bookmark', 'Show Bookmarks', 'New URL', 'Quit'];
            var choice = dialog.choice(choices);

            if (choice === 'Quit') {
                running = false;
            } else if (choice === 'Go Back') {
                goBack();
            } else if (choice === 'Go Forward') {
                goForward();
            } else if (choice === 'Add Bookmark') {
                addBookmark();
            } else if (choice === 'Show Bookmarks') {
                showBookmarks();
            } else if (choice === 'New URL') {
                var newUrl = prompt('Enter URL:');
                if (newUrl) {
                    currentUrl = newUrl;
                    loadPage();
                }
            }
        }

        if (keyboard.getPrevPress()) {
            goBack();
        }

        if (keyboard.getNextPress()) {
            goForward();
        }

        display.draw();
        delay(100);
    }
}

main();
