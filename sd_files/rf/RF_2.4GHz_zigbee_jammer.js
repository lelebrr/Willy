// RF 2.4GHz Zigbee Jammer
// Advanced jamming for Zigbee devices
// Configure frequency and modulation before use

const RF24 = require('rf24');
const zigbeeJammer = new RF24();

// Configuration
const FREQUENCY = 2.4; // GHz
const MODULATION = 'O-QPSK';
const CHANNELS = [11, 15, 20, 25]; // Zigbee channels
const REPEAT = 800; // times per channel

zigbeeJammer.setFrequency(FREQUENCY);
zigbeeJammer.setModulation(MODULATION);

// Zigbee jamming pattern
const zigbeePattern = [
    { duration: 40, level: 1 },
    { duration: 40, level: 0 },
    { duration: 40, level: 1 },
    { duration: 40, level: 0 }
];

console.log('Starting Zigbee jammer...');
CHANNELS.forEach(channel => {
    zigbeeJammer.setChannel(channel);
    zigbeeJammer.transmit(zigbeePattern, REPEAT);
});
console.log('Zigbee jamming completed');
