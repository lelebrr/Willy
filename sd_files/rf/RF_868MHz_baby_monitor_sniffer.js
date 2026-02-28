// RF 868MHz Baby Monitor Sniffer
// Advanced sniffer for baby monitor communications
// Configure frequency and modulation before use

const RF868 = require('rf868');
const babySniffer = new RF868();

// Configuration
const FREQUENCY = 868.35; // MHz
const MODULATION = 'FSK';
const PULSE_WIDTH = 200; // microseconds
const CHANNELS = [868.35, 868.40, 868.45, 868.50];

babySniffer.setFrequency(FREQUENCY);
babySniffer.setModulation(MODULATION);
babySniffer.setPulseWidth(PULSE_WIDTH);

// Baby monitor sniffing pattern
const babyPattern = [
    { duration: 3000, level: 1 },
    { duration: 1000, level: 0 },
    { duration: 3000, level: 1 },
    { duration: 1000, level: 0 }
];

console.log('Starting baby monitor sniffer...');
CHANNELS.forEach(channel => {
    babySniffer.setChannel(channel);
    babySniffer.receive(babyPattern, 20);
});
console.log('Baby monitor sniffing completed');
