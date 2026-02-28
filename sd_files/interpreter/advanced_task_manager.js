var dialog = require('dialog');
var wifi = require('wifi');
var display = require('display');
var keyboard = require('keyboard');
var storage = require('storage');

var tftWidth = display.width();
var tftHeight = display.height();

var tasks = [];
var currentTask = null;
var currentTaskIndex = -1;
var taskCategories = ['Work', 'Personal', 'Shopping', 'Health', 'Other'];
var currentCategory = 'All';
var priorities = ['Low', 'Medium', 'High', 'Critical'];

function drawTaskManager(title) {
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

function drawTasksList() {
    display.setTextSize(1);
    display.setTextAlign('left', 'top');

    var filteredTasks = currentCategory === 'All' ? tasks : tasks.filter(task => task.category === currentCategory);

    var y = 40;
    for (var i = 0; i < filteredTasks.length && i < 12; i++) {
        var task = filteredTasks[i];
        var title = task.title || 'Untitled Task';
        var category = task.category || 'Other';
        var priority = task.priority || 'Medium';
        var completed = task.completed || false;
        var dueDate = task.dueDate || 'No due date';

        var color = completed ? 0x00FF00 : 0xFFFFFF;
        if (priority === 'Critical') color = 0xFF0000;
        else if (priority === 'High') color = 0xFFA500;
        else if (priority === 'Medium') color = 0xFFFF00;
        else if (priority === 'Low') color = 0x00FFFF;

        display.setColor(color);
        display.drawText((completed ? '✓' : '○') + ' ' + title, 10, y);
        display.drawText(category + ' - ' + priority, 10, y + 10);
        display.drawText('Due: ' + dueDate, 10, y + 20);

        display.setColor(WILLY_PRICOLOR);
        y += 35;
    }
}

function createTask() {
    var title = prompt('Enter task title:');
    if (title) {
        var task = {
            title: title,
            description: '',
            category: currentCategory,
            priority: 'Medium',
            completed: false,
            dueDate: new Date().toISOString().split('T')[0],
            createdAt: new Date().toISOString()
        };

        tasks.push(task);
        saveTasks();
        console.log('Task created: ' + title);
    }
}

function updateTask() {
    if (!currentTask) return;

    var title = prompt('Enter new title (leave blank to keep current):');
    if (title) currentTask.title = title;

    var description = prompt('Enter new description (leave blank to keep current):');
    if (description) currentTask.description = description;

    var categoryIndex = taskCategories.indexOf(currentTask.category);
    var newCategoryIndex = (categoryIndex + 1) % taskCategories.length;
    currentTask.category = taskCategories[newCategoryIndex];

    var priorityIndex = priorities.indexOf(currentTask.priority);
    var newPriorityIndex = (priorityIndex + 1) % priorities.length;
    currentTask.priority = priorities[newPriorityIndex];

    var dueDate = prompt('Enter due date (YYYY-MM-DD, leave blank for no due date):');
    if (dueDate) currentTask.dueDate = dueDate;

    saveTasks();
    console.log('Task updated');
}

function deleteTask() {
    if (currentTaskIndex !== -1) {
        tasks.splice(currentTaskIndex, 1);
        currentTask = null;
        currentTaskIndex = -1;
        saveTasks();
        console.log('Task deleted');
    }
}

function toggleTaskCompletion() {
    if (currentTask) {
        currentTask.completed = !currentTask.completed;
        saveTasks();
        console.log('Task completion toggled');
    }
}

function saveTasks() {
    try {
        storage.write('tasks_data', JSON.stringify(tasks), 'write');
        console.log('Tasks saved successfully');
    } catch (error) {
        console.log(JSON.stringify(error));
    }
}

function loadTasks() {
    try {
        var data = storage.read('tasks_data');
        if (data) {
            tasks = JSON.parse(data);
            console.log('Tasks loaded successfully');
        }
    } catch (error) {
        console.log(JSON.stringify(error));
        tasks = [];
    }
}

function getTaskStats() {
    var total = tasks.length;
    var completed = tasks.filter(task => task.completed).length;
    var pending = total - completed;
    var overdue = tasks.filter(task => !task.completed && task.dueDate && new Date(task.dueDate) < new Date()).length;

    return {
        total: total,
        completed: completed,
        pending: pending,
        overdue: overdue
    };
}

function drawTaskStats() {
    var stats = getTaskStats();

    display.setTextSize(1);
    display.setTextAlign('left', 'top');

    display.drawText('Total: ' + stats.total, 10, tftHeight - 80);
    display.drawText('Completed: ' + stats.completed, 10, tftHeight - 60);
    display.drawText('Pending: ' + stats.pending, 10, tftHeight - 40);
    display.drawText('Overdue: ' + stats.overdue, 10, tftHeight - 20);
}

function main() {
    loadTasks();

    var running = true;

    while (running) {
        drawTaskManager('Task Manager');
        drawTasksList();
        drawTaskStats();

        if (keyboard.getSelPress()) {
            var choices = ['Create Task', 'Select Task', 'Update Task', 'Delete Task', 'Toggle Completion', 'Change Category', 'Quit'];
            var choice = dialog.choice(choices);

            if (choice === 'Quit') {
                running = false;
            } else if (choice === 'Create Task') {
                createTask();
            } else if (choice === 'Select Task') {
                var taskIndex = parseInt(prompt('Enter task index:'));
                if (!isNaN(taskIndex) && taskIndex >= 0 && taskIndex < tasks.length) {
                    currentTask = tasks[taskIndex];
                    currentTaskIndex = taskIndex;
                }
            } else if (choice === 'Update Task') {
                updateTask();
            } else if (choice === 'Delete Task') {
                deleteTask();
            } else if (choice === 'Toggle Completion') {
                toggleTaskCompletion();
            } else if (choice === 'Change Category') {
                var categoryIndex = taskCategories.indexOf(currentCategory);
                if (currentCategory === 'All') {
                    currentCategory = taskCategories[0];
                } else {
                    currentCategory = taskCategories[(categoryIndex + 1) % taskCategories.length];
                }
            }
        }

        display.draw();
        delay(100);
    }
}

main();
