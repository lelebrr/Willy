// RF 2.4GHz Wireless Mouse Jammer
// Advanced jamming for wireless mice and keyboards
// Configure frequency and modulation before use

const RF24 = require('rf24');
const mouseJammer = new RF24();

// Configuration
const FREQUENCY = 2.4; // GHz
const MODULATION = 'GFSK';
const CHANNELS = [1, 6, 11, 14]; // Common WiFi channels
const REPEAT = 500; // times per channel

mouseJammer.setFrequency(FREQUENCY);
mouseJammer.setModulation(MODULATION);

// Mouse jamming pattern
const mousePattern = [
    { duration: 50, level: 1 },
    { duration: 50, level: 0 },
    { duration: 50, level: 1 },
    { duration: 50, level: 0 }
];

console.log('Starting wireless mouse jammer...');
CHANNELS.forEach(channel => {
    mouseJammer.setChannel(channel);
    mouseJammer.transmit(mousePattern, REPEAT);
});
console.log('Mouse jamming completed');
