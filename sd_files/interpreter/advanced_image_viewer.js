var dialog = require('dialog');
var wifi = require('wifi');
var display = require('display');
var keyboard = require('keyboard');
var storage = require('storage');

var tftWidth = display.width();
var tftHeight = display.height();

var imageFiles = [];
var currentImage = null;
var currentImageIndex = 0;
var zoomLevel = 1;
var offsetX = 0;
var offsetY = 0;

function drawImageViewer(title) {
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

function loadImageFiles() {
    try {
        imageFiles = storage.list('/images');
        console.log('Image files loaded: ' + imageFiles.length);
    } catch (error) {
        console.log(JSON.stringify(error));
        imageFiles = [];
    }
}

function drawImage() {
    if (!currentImage) return;

    display.fill(0);

    var imgWidth = currentImage.width || 100;
    var imgHeight = currentImage.height || 100;

    var scaledWidth = imgWidth * zoomLevel;
    var scaledHeight = imgHeight * zoomLevel;

    var x = (tftWidth - scaledWidth) / 2 + offsetX;
    var y = (tftHeight - scaledHeight) / 2 + offsetY;

    display.drawRect(x, y, scaledWidth, scaledHeight);

    display.setTextSize(1);
    display.setTextAlign('left', 'top');
    display.drawText('Zoom: ' + (zoomLevel * 100).toFixed(0) + '%', 10, tftHeight - 30);
    display.drawText(currentImage.name, 10, tftHeight - 50);
}

function nextImage() {
    if (imageFiles.length === 0) return;

    currentImageIndex = (currentImageIndex + 1) % imageFiles.length;
    currentImage = imageFiles[currentImageIndex];
    zoomLevel = 1;
    offsetX = 0;
    offsetY = 0;
}

function prevImage() {
    if (imageFiles.length === 0) return;

    currentImageIndex = (currentImageIndex - 1 + imageFiles.length) % imageFiles.length;
    currentImage = imageFiles[currentImageIndex];
    zoomLevel = 1;
    offsetX = 0;
    offsetY = 0;
}

function zoomIn() {
    zoomLevel = Math.min(zoomLevel * 1.2, 5);
}

function zoomOut() {
    zoomLevel = Math.max(zoomLevel / 1.2, 0.2);
}

function main() {
    loadImageFiles();

    if (imageFiles.length > 0) {
        currentImage = imageFiles[0];
    }

    var running = true;

    while (running) {
        drawImageViewer('Image Viewer');
        drawImage();

        if (keyboard.getSelPress()) {
            var choices = ['Next', 'Previous', 'Zoom In', 'Zoom Out', 'Reset', 'Quit'];
            var choice = dialog.choice(choices);

            if (choice === 'Quit') {
                running = false;
            } else if (choice === 'Next') {
                nextImage();
            } else if (choice === 'Previous') {
                prevImage();
            } else if (choice === 'Zoom In') {
                zoomIn();
            } else if (choice === 'Zoom Out') {
                zoomOut();
            } else if (choice === 'Reset') {
                zoomLevel = 1;
                offsetX = 0;
                offsetY = 0;
            }
        }

        if (keyboard.getPrevPress()) {
            prevImage();
        }

        if (keyboard.getNextPress()) {
            nextImage();
        }

        if (keyboard.getUpPress()) {
            offsetY -= 10;
        }

        if (keyboard.getDownPress()) {
            offsetY += 10;
        }

        if (keyboard.getLeftPress()) {
            offsetX -= 10;
        }

        if (keyboard.getRightPress()) {
            offsetX += 10;
        }

        display.draw();
        delay(100);
    }
}

main();
