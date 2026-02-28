// RF 315MHz Remote Control Jammer
// Advanced jamming for remote controls
// Configure frequency and modulation before use

const RF315 = require('rf315');
const jammer = new RF315();

// Configuration
const FREQUENCY = 315.0; // MHz
const MODULATION = 'ASK';
const PULSE_WIDTH = 500; // microseconds
const REPEAT = 2000; // times

jammer.setFrequency(FREQUENCY);
jammer.setModulation(MODULATION);
jammer.setPulseWidth(PULSE_WIDTH);

// Jamming pattern for remote controls
const jamPattern = [
    { duration: 200, level: 1 },
    { duration: 100, level: 0 },
    { duration: 200, level: 1 },
    { duration: 100, level: 0 }
];

console.log('Starting remote control jammer...');
jammer.transmit(jamPattern, REPEAT);
console.log('Jamming completed');
