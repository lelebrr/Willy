// RF 868MHz Garage Door Opener
// Advanced garage door opener with rolling code emulation
// Configure frequency and codes before use

const RF868 = require('rf868');
const garageOpener = new RF868();

// Configuration
const FREQUENCY = 868.35; // MHz
const MODULATION = 'FSK';
const PULSE_WIDTH = 200; // microseconds
const ROLLING_CODE_BASE = 12345678;

garageOpener.setFrequency(FREQUENCY);
garageOpener.setModulation(MODULATION);
garageOpener.setPulseWidth(PULSE_WIDTH);

// Garage door opener pattern
const garagePattern = [
    { duration: 500, level: 1 },
    { duration: 1000, level: 0 },
    { duration: 500, level: 1 },
    { duration: 1000, level: 0 }
];

console.log('Opening garage door...');
garageOpener.transmit(garagePattern, 3);
console.log('Garage door opened');
