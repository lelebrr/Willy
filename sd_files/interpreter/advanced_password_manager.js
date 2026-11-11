var dialog = require('dialog');
var wifi = require('wifi');
var display = require('display');
var keyboard = require('keyboard');
var storage = require('storage');

var tftWidth = display.width();
var tftHeight = display.height();

var passwords = [];
var currentPassword = null;
var currentPasswordIndex = -1;
var passwordCategories = ['Email', 'Social Media', 'Banking', 'Shopping', 'Work', 'Other'];
var currentCategory = 'All';
var masterPassword = '';
var isUnlocked = false;

function drawPasswordManager(title) {
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

function drawPasswordsList() {
    if (!isUnlocked) {
        display.setTextSize(1);
        display.setTextAlign('center', 'top');
        display.drawText('Enter master password to unlock', tftWidth / 2, tftHeight / 2);
        return;
    }

    display.setTextSize(1);
    display.setTextAlign('left', 'top');

    var filteredPasswords = currentCategory === 'All' ? passwords : passwords.filter(pwd => pwd.category === currentCategory);

    var y = 40;
    for (var i = 0; i < filteredPasswords.length && i < 12; i++) {
        var pwd = filteredPasswords[i];
        var title = pwd.title || 'Untitled';
        var username = pwd.username || 'Unknown';
        var category = pwd.category || 'Other';
        var url = pwd.url || '';

        display.drawText(title, 10, y);
        display.drawText(username, 10, y + 10);
        display.drawText(category, 10, y + 20);
        if (url) display.drawText(url, 10, y + 30);

        y += 45;
    }
}

function createPassword() {
    if (!isUnlocked) {
        var input = prompt('Enter master password:');
        if (input === masterPassword) {
            isUnlocked = true;
        } else {
            console.log('Invalid master password');
            return;
        }
    }

    var title = prompt('Enter password title:');
    if (title) {
        var password = {
            title: title,
            username: '',
            password: '',
            url: '',
            category: currentCategory,
            notes: '',
            createdAt: new Date().toISOString(),
            updatedAt: new Date().toISOString()
        };

        passwords.push(password);
        savePasswords();
        console.log('Password created: ' + title);
    }
}

function updatePassword() {
    if (!isUnlocked || !currentPassword) return;

    var title = prompt('Enter new title (leave blank to keep current):');
    if (title) currentPassword.title = title;

    var username = prompt('Enter new username (leave blank to keep current):');
    if (username) currentPassword.username = username;

    var password = prompt('Enter new password (leave blank to keep current):');
    if (password) currentPassword.password = password;

    var url = prompt('Enter new URL (leave blank to keep current):');
    if (url) currentPassword.url = url;

    var notes = prompt('Enter new notes (leave blank to keep current):');
    if (notes) currentPassword.notes = notes;

    var categoryIndex = passwordCategories.indexOf(currentPassword.category);
    var newCategoryIndex = (categoryIndex + 1) % passwordCategories.length;
    currentPassword.category = passwordCategories[newCategoryIndex];

    currentPassword.updatedAt = new Date().toISOString();
    savePasswords();
    console.log('Password updated');
}

function deletePassword() {
    if (!isUnlocked || currentPasswordIndex === -1) return;

    passwords.splice(currentPasswordIndex, 1);
    currentPassword = null;
    currentPasswordIndex = -1;
    savePasswords();
    console.log('Password deleted');
}

function generatePassword() {
    var length = parseInt(prompt('Enter password length (8-32):'));
    if (isNaN(length) || length < 8 || length > 32) {
        length = 16;
    }

    var charset = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()_+-=[]{}|;:,.<>?';
    var password = '';

    for (var i = 0; i < length; i++) {
        password += charset.charAt(Math.floor(Math.random() * charset.length));
    }

    return password;
}

function searchPasswords() {
    if (!isUnlocked) return;

    var searchTerm = prompt('Enter search term:');
    if (searchTerm) {
        var results = passwords.filter(pwd =>
            pwd.title.toLowerCase().includes(searchTerm.toLowerCase()) ||
            pwd.username.toLowerCase().includes(searchTerm.toLowerCase()) ||
            pwd.url.toLowerCase().includes(searchTerm.toLowerCase())
        );

        display.setTextSize(1);
        display.setTextAlign('left', 'top');
        display.drawText('Search Results:', 10, 40);

        var y = 60;
        for (var i = 0; i < results.length && i < 10; i++) {
            var pwd = results[i];
            display.drawText(pwd.title + ' - ' + pwd.username, 10, y);
            y += 20;
        }
    }
}

function savePasswords() {
    try {
        storage.write('passwords_data', JSON.stringify(passwords), 'write');
        console.log('Passwords saved successfully');
    } catch (error) {
        console.log(JSON.stringify(error));
    }
}

function loadPasswords() {
    try {
        var data = storage.read('passwords_data');
        if (data) {
            passwords = JSON.parse(data);
            console.log('Passwords loaded successfully');
        }
    } catch (error) {
        console.log(JSON.stringify(error));
        passwords = [];
    }
}

function exportPasswords() {
    if (!isUnlocked) return;

    try {
        var csv = 'Title,Username,Password,URL,Category,Notes,Created,Updated\n';
        for (var i = 0; i < passwords.length; i++) {
            var pwd = passwords[i];
            csv += pwd.title + ',' + pwd.username + ',' + pwd.password + ',' + pwd.url + ',' + pwd.category + ',' + pwd.notes + ',' + pwd.createdAt + ',' + pwd.updatedAt + '\n';
        }

        storage.write('passwords_export.csv', csv, 'write');
        console.log('Passwords exported successfully');
    } catch (error) {
        console.log(JSON.stringify(error));
    }
}

function main() {
    loadPasswords();

    var running = true;

    while (running) {
        drawPasswordManager('Password Manager');
        drawPasswordsList();

        if (keyboard.getSelPress()) {
            var choices = [];

            if (!isUnlocked) {
                choices = ['Unlock', 'Quit'];
            } else {
                choices = ['Create Password', 'Select Password', 'Update Password', 'Delete Password', 'Generate Password', 'Search Passwords', 'Export Passwords', 'Change Category', 'Lock', 'Quit'];
            }

            var choice = dialog.choice(choices);

            if (choice === 'Quit') {
                running = false;
            } else if (choice === 'Unlock' && !isUnlocked) {
                var input = prompt('Enter master password:');
                if (input === masterPassword) {
                    isUnlocked = true;
                } else {
                    console.log('Invalid master password');
                }
            } else if (choice === 'Create Password' && isUnlocked) {
                createPassword();
            } else if (choice === 'Select Password' && isUnlocked) {
                var pwdIndex = parseInt(prompt('Enter password index:'));
                if (!isNaN(pwdIndex) && pwdIndex >= 0 && pwdIndex < passwords.length) {
                    currentPassword = passwords[pwdIndex];
                    currentPasswordIndex = pwdIndex;
                }
            } else if (choice === 'Update Password' && isUnlocked) {
                updatePassword();
            } else if (choice === 'Delete Password' && isUnlocked) {
                deletePassword();
            } else if (choice === 'Generate Password' && isUnlocked) {
                var generatedPassword = generatePassword();
                console.log('Generated password: ' + generatedPassword);
            } else if (choice === 'Search Passwords' && isUnlocked) {
                searchPasswords();
            } else if (choice === 'Export Passwords' && isUnlocked) {
                exportPasswords();
            } else if (choice === 'Change Category' && isUnlocked) {
                var categoryIndex = passwordCategories.indexOf(currentCategory);
                if (currentCategory === 'All') {
                    currentCategory = passwordCategories[0];
                } else {
                    currentCategory = passwordCategories[(categoryIndex + 1) % passwordCategories.length];
                }
            } else if (choice === 'Lock' && isUnlocked) {
                isUnlocked = false;
            }
        }

        display.draw();
        delay(100);
    }
}

main();
