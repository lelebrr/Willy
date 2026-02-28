var dialog = require('dialog');
var wifi = require('wifi');
var display = require('display');
var keyboard = require('keyboard');
var storage = require('storage');

var tftWidth = display.width();
var tftHeight = display.height();

var contacts = [];
var currentContact = null;
var currentContactIndex = -1;
var contactGroups = ['Family', 'Friends', 'Work', 'Other'];
var currentGroup = 'All';

function drawContactManager(title) {
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

function drawContactsList() {
    display.setTextSize(1);
    display.setTextAlign('left', 'top');

    var filteredContacts = currentGroup === 'All' ? contacts : contacts.filter(contact => contact.group === currentGroup);

    var y = 40;
    for (var i = 0; i < filteredContacts.length && i < 12; i++) {
        var contact = filteredContacts[i];
        var name = contact.name || 'Unknown';
        var phone = contact.phone || 'Unknown';
        var email = contact.email || 'Unknown';
        var group = contact.group || 'Other';

        display.drawText(name, 10, y);
        display.drawText(phone, 10, y + 10);
        display.drawText(email, 10, y + 20);
        display.drawText('Group: ' + group, 10, y + 30);

        y += 45;
    }
}

function createContact() {
    var name = prompt('Enter contact name:');
    if (name) {
        var contact = {
            name: name,
            phone: '',
            email: '',
            address: '',
            group: currentGroup,
            notes: '',
            createdAt: new Date().toISOString()
        };

        contacts.push(contact);
        saveContacts();
        console.log('Contact created: ' + name);
    }
}

function updateContact() {
    if (!currentContact) return;

    var name = prompt('Enter new name (leave blank to keep current):');
    if (name) currentContact.name = name;

    var phone = prompt('Enter new phone (leave blank to keep current):');
    if (phone) currentContact.phone = phone;

    var email = prompt('Enter new email (leave blank to keep current):');
    if (email) currentContact.email = email;

    var address = prompt('Enter new address (leave blank to keep current):');
    if (address) currentContact.address = address;

    var notes = prompt('Enter new notes (leave blank to keep current):');
    if (notes) currentContact.notes = notes;

    var groupIndex = contactGroups.indexOf(currentContact.group);
    var newGroupIndex = (groupIndex + 1) % contactGroups.length;
    currentContact.group = contactGroups[newGroupIndex];

    saveContacts();
    console.log('Contact updated');
}

function deleteContact() {
    if (currentContactIndex !== -1) {
        contacts.splice(currentContactIndex, 1);
        currentContact = null;
        currentContactIndex = -1;
        saveContacts();
        console.log('Contact deleted');
    }
}

function searchContacts() {
    var searchTerm = prompt('Enter search term:');
    if (searchTerm) {
        var results = contacts.filter(contact =>
            contact.name.toLowerCase().includes(searchTerm.toLowerCase()) ||
            contact.phone.includes(searchTerm) ||
            contact.email.toLowerCase().includes(searchTerm.toLowerCase())
        );

        display.setTextSize(1);
        display.setTextAlign('left', 'top');
        display.drawText('Search Results:', 10, 40);

        var y = 60;
        for (var i = 0; i < results.length && i < 10; i++) {
            var contact = results[i];
            display.drawText(contact.name + ' - ' + contact.phone, 10, y);
            y += 20;
        }
    }
}

function saveContacts() {
    try {
        storage.write('contacts_data', JSON.stringify(contacts), 'write');
        console.log('Contacts saved successfully');
    } catch (error) {
        console.log(JSON.stringify(error));
    }
}

function loadContacts() {
    try {
        var data = storage.read('contacts_data');
        if (data) {
            contacts = JSON.parse(data);
            console.log('Contacts loaded successfully');
        }
    } catch (error) {
        console.log(JSON.stringify(error));
        contacts = [];
    }
}

function exportContacts() {
    try {
        var csv = 'Name,Phone,Email,Address,Group,Notes\n';
        for (var i = 0; i < contacts.length; i++) {
            var contact = contacts[i];
            csv += contact.name + ',' + contact.phone + ',' + contact.email + ',' + contact.address + ',' + contact.group + ',' + contact.notes + '\n';
        }

        storage.write('contacts_export.csv', csv, 'write');
        console.log('Contacts exported successfully');
    } catch (error) {
        console.log(JSON.stringify(error));
    }
}

function main() {
    loadContacts();

    var running = true;

    while (running) {
        drawContactManager('Contact Manager');
        drawContactsList();

        if (keyboard.getSelPress()) {
            var choices = ['Create Contact', 'Select Contact', 'Update Contact', 'Delete Contact', 'Search Contacts', 'Export Contacts', 'Change Group', 'Quit'];
            var choice = dialog.choice(choices);

            if (choice === 'Quit') {
                running = false;
            } else if (choice === 'Create Contact') {
                createContact();
            } else if (choice === 'Select Contact') {
                var contactIndex = parseInt(prompt('Enter contact index:'));
                if (!isNaN(contactIndex) && contactIndex >= 0 && contactIndex < contacts.length) {
                    currentContact = contacts[contactIndex];
                    currentContactIndex = contactIndex;
                }
            } else if (choice === 'Update Contact') {
                updateContact();
            } else if (choice === 'Delete Contact') {
                deleteContact();
            } else if (choice === 'Search Contacts') {
                searchContacts();
            } else if (choice === 'Export Contacts') {
                exportContacts();
            } else if (choice === 'Change Group') {
                var groupIndex = contactGroups.indexOf(currentGroup);
                if (currentGroup === 'All') {
                    currentGroup = contactGroups[0];
                } else {
                    currentGroup = contactGroups[(groupIndex + 1) % contactGroups.length];
                }
            }
        }

        display.draw();
        delay(100);
    }
}

main();
