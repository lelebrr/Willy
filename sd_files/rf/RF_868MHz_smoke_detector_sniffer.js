// RF 868MHz Smoke Detector Sniffer
// Advanced sniffer for smoke detector communications
// Configure frequency and modulation before use

const RF868 = require('rf868');
const smokeSniffer = new RF868();

// Configuration
const FREQUENCY = 868.35; // MHz
const MODULATION = 'FSK';
const PULSE_WIDTH = 200; // microseconds
const CHANNELS = [868.35, 868.40, 868.45];

smokeSniffer.setFrequency(FREQUENCY);
smokeSniffer.setModulation(MODULATION);
smokeSniffer.setPulseWidth(PULSE_WIDTH);

// Smoke detector sniffing pattern
const smokePattern = [
    { duration: 2500, level: 1 },
    { duration: 500, level: 0 },
    { duration: 2500, level: 1 },
    { duration: 500, level: 0 }
];

console.log('Starting smoke detector sniffer...');
CHANNELS.forEach(channel => {
    smokeSniffer.setChannel(channel);
    smokeSniffer.receive(smokePattern, 18);
});
console.log('Smoke detector sniffing completed');
