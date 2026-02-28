// RF 433MHz Door Lock Sniffer
// Advanced sniffer for door lock communications
// Configure frequency and modulation before use

const RF433 = require('rf433');
const lockSniffer = new RF433();

// Configuration
const FREQUENCY = 433.92; // MHz
const MODULATION = 'ASK';
const PULSE_WIDTH = 350; // microseconds
const CHANNELS = [433.92, 433.95, 433.98];

lockSniffer.setFrequency(FREQUENCY);
lockSniffer.setModulation(MODULATION);
lockSniffer.setPulseWidth(PULSE_WIDTH);

// Door lock sniffing pattern
const lockPattern = [
    { duration: 2000, level: 1 },
    { duration: 1000, level: 0 },
    { duration: 2000, level: 1 },
    { duration: 1000, level: 0 }
];

console.log('Starting door lock sniffer...');
CHANNELS.forEach(channel => {
    lockSniffer.setChannel(channel);
    lockSniffer.receive(lockPattern, 15);
});
console.log('Door lock sniffing completed');
