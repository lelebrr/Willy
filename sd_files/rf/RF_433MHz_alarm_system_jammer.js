// RF 433MHz Alarm System Jammer
// Advanced jamming for alarm systems
// Configure frequency and modulation before use

const RF433 = require('rf433');
const alarmJammer = new RF433();

// Configuration
const FREQUENCY = 433.92; // MHz
const MODULATION = 'ASK';
const PULSE_WIDTH = 350; // microseconds
const REPEAT = 3000; // times

alarmJammer.setFrequency(FREQUENCY);
alarmJammer.setModulation(MODULATION);
alarmJammer.setPulseWidth(PULSE_WIDTH);

// Alarm system jamming pattern
const alarmPattern = [
    { duration: 150, level: 1 },
    { duration: 75, level: 0 },
    { duration: 150, level: 1 },
    { duration: 75, level: 0 }
];

console.log('Starting alarm system jammer...');
alarmJammer.transmit(alarmPattern, REPEAT);
console.log('Alarm system jamming completed');
