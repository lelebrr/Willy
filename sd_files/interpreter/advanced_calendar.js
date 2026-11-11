var dialog = require('dialog');
var wifi = require('wifi');
var display = require('display');
var keyboard = require('keyboard');
var storage = require('storage');

var tftWidth = display.width();
var tftHeight = display.height();

var events = [];
var currentEvent = null;
var currentEventIndex = -1;
var currentDate = new Date();
var viewMode = 'month';
var eventCategories = ['Work', 'Personal', 'Meeting', 'Reminder', 'Other'];

function drawCalendar(title) {
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

function drawMonthView() {
    display.setTextSize(1);
    display.setTextAlign('left', 'top');

    var year = currentDate.getFullYear();
    var month = currentDate.getMonth();
    var firstDay = new Date(year, month, 1).getDay();
    var daysInMonth = new Date(year, month + 1, 0).getDate();

    display.drawText(year + '-' + (month + 1).toString().padStart(2, '0'), 10, 40);

    var dayNames = ['Sun', 'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat'];
    var x = 10;
    var y = 60;

    for (var i = 0; i < 7; i++) {
        display.drawText(dayNames[i], x + i * 40, y);
    }

    y += 20;
    x = 10 + firstDay * 40;

    for (var day = 1; day <= daysInMonth; day++) {
        var dayEvents = events.filter(event => {
            var eventDate = new Date(event.date);
            return eventDate.getDate() === day &&
                eventDate.getMonth() === month &&
                eventDate.getFullYear() === year;
        });

        var dayText = day.toString();
        if (dayEvents.length > 0) {
            dayText += '(' + dayEvents.length + ')';
        }

        display.drawText(dayText, x, y);

        x += 40;
        if (x > tftWidth - 50) {
            x = 10;
            y += 20;
        }
    }
}

function drawEventList() {
    display.setTextSize(1);
    display.setTextAlign('left', 'top');

    var sortedEvents = events.sort((a, b) => new Date(a.date) - new Date(b.date));

    var y = 40;
    for (var i = 0; i < sortedEvents.length && i < 15; i++) {
        var event = sortedEvents[i];
        var title = event.title || 'Untitled Event';
        var date = new Date(event.date);
        var time = event.time || 'All day';
        var category = event.category || 'Other';

        display.drawText(title, 10, y);
        display.drawText(date.toISOString().split('T')[0] + ' ' + time, 10, y + 10);
        display.drawText(category, 10, y + 20);

        y += 35;
    }
}

function createEvent() {
    var title = prompt('Enter event title:');
    if (title) {
        var event = {
            title: title,
            description: '',
            date: currentDate.toISOString().split('T')[0],
            time: '00:00',
            category: 'Personal',
            reminder: false,
            reminderTime: 15
        };

        events.push(event);
        saveEvents();
        console.log('Event created: ' + title);
    }
}

function updateEvent() {
    if (!currentEvent) return;

    var title = prompt('Enter new title (leave blank to keep current):');
    if (title) currentEvent.title = title;

    var description = prompt('Enter new description (leave blank to keep current):');
    if (description) currentEvent.description = description;

    var date = prompt('Enter new date (YYYY-MM-DD, leave blank for current):');
    if (date) currentEvent.date = date;

    var time = prompt('Enter new time (HH:MM, leave blank for current):');
    if (time) currentEvent.time = time;

    var categoryIndex = eventCategories.indexOf(currentEvent.category);
    var newCategoryIndex = (categoryIndex + 1) % eventCategories.length;
    currentEvent.category = eventCategories[newCategoryIndex];

    saveEvents();
    console.log('Event updated');
}

function deleteEvent() {
    if (currentEventIndex !== -1) {
        events.splice(currentEventIndex, 1);
        currentEvent = null;
        currentEventIndex = -1;
        saveEvents();
        console.log('Event deleted');
    }
}

function saveEvents() {
    try {
        storage.write('calendar_events', JSON.stringify(events), 'write');
        console.log('Events saved successfully');
    } catch (error) {
        console.log(JSON.stringify(error));
    }
}

function loadEvents() {
    try {
        var data = storage.read('calendar_events');
        if (data) {
            events = JSON.parse(data);
            console.log('Events loaded successfully');
        }
    } catch (error) {
        console.log(JSON.stringify(error));
        events = [];
    }
}

function navigateCalendar() {
    if (keyboard.getPrevPress()) {
        currentDate.setMonth(currentDate.getMonth() - 1);
    }

    if (keyboard.getNextPress()) {
        currentDate.setMonth(currentDate.getMonth() + 1);
    }

    if (keyboard.getUpPress()) {
        currentDate.setDate(currentDate.getDate() - 7);
    }

    if (keyboard.getDownPress()) {
        currentDate.setDate(currentDate.getDate() + 7);
    }
}

function main() {
    loadEvents();

    var running = true;

    while (running) {
        drawCalendar('Calendar');

        if (viewMode === 'month') {
            drawMonthView();
        } else if (viewMode === 'list') {
            drawEventList();
        }

        if (keyboard.getSelPress()) {
            var choices = [];

            if (viewMode === 'month') {
                choices = ['Create Event', 'View Events', 'Next Month', 'Previous Month', 'Quit'];
            } else if (viewMode === 'list') {
                choices = ['Create Event', 'Update Event', 'Delete Event', 'Back to Calendar', 'Quit'];
            }

            var choice = dialog.choice(choices);

            if (choice === 'Quit') {
                running = false;
            } else if (choice === 'Create Event') {
                createEvent();
            } else if (choice === 'View Events' && viewMode === 'month') {
                viewMode = 'list';
            } else if (choice === 'Next Month' && viewMode === 'month') {
                currentDate.setMonth(currentDate.getMonth() + 1);
            } else if (choice === 'Previous Month' && viewMode === 'month') {
                currentDate.setMonth(currentDate.getMonth() - 1);
            } else if (choice === 'Update Event' && viewMode === 'list') {
                updateEvent();
            } else if (choice === 'Delete Event' && viewMode === 'list') {
                deleteEvent();
            } else if (choice === 'Back to Calendar' && viewMode === 'list') {
                viewMode = 'month';
            }
        }

        if (viewMode === 'month') {
            navigateCalendar();
        }

        display.draw();
        delay(100);
    }
}

main();
