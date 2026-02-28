// RF 915MHz Pet Tracker Jammer
// Advanced jamming for pet tracking devices
// Configure frequency and modulation before use

const RF915 = require('rf915');
const petJammer = new RF915();

// Configuration
const FREQUENCY = 915.0; // MHz
const MODULATION = 'FSK';
const PULSE_WIDTH = 150; // microseconds
const REPEAT = 1500; // times

petJammer.setFrequency(FREQUENCY);
petJammer.setModulation(MODULATION);
petJammer.setPulseWidth(PULSE_WIDTH);

// Pet tracker jamming pattern
const petPattern = [
    { duration: 400, level: 1 },
    { duration: 200, level: 0 },
    { duration: 400, level: 1 },
    { duration: 200, level: 0 }
];

console.log('Starting pet tracker jammer...');
petJammer.transmit(petPattern, REPEAT);
console.log('Pet tracker jamming completed');
