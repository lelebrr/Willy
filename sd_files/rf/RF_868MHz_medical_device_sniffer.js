// RF 868MHz Medical Device Sniffer
// Advanced sniffer for medical device communications
// Configure frequency and modulation before use

const RF868 = require('rf868');
const medicalSniffer = new RF868();

// Configuration
const FREQUENCY = 868.35; // MHz
const MODULATION = 'FSK';
const PULSE_WIDTH = 200; // microseconds
const CHANNELS = [868.35, 868.40, 868.45];

medicalSniffer.setFrequency(FREQUENCY);
medicalSniffer.setModulation(MODULATION);
medicalSniffer.setPulseWidth(PULSE_WIDTH);

// Medical device sniffing pattern
const medicalPattern = [
    { duration: 4000, level: 1 },
    { duration: 1000, level: 0 },
    { duration: 4000, level: 1 },
    { duration: 1000, level: 0 }
];

console.log('Starting medical device sniffer...');
CHANNELS.forEach(channel => {
    medicalSniffer.setChannel(channel);
    medicalSniffer.receive(medicalPattern, 25);
});
console.log('Medical device sniffing completed');
