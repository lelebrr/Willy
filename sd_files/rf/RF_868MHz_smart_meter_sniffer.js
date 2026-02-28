// RF 868MHz Smart Meter Sniffer
// Advanced sniffer for smart meter communications
// Configure frequency and modulation before use

const RF868 = require('rf868');
const smartMeterSniffer = new RF868();

// Configuration
const FREQUENCY = 868.35; // MHz
const MODULATION = 'FSK';
const PULSE_WIDTH = 200; // microseconds
const CHANNELS = [868.35, 868.40, 868.45];

smartMeterSniffer.setFrequency(FREQUENCY);
smartMeterSniffer.setModulation(MODULATION);
smartMeterSniffer.setPulseWidth(PULSE_WIDTH);

// Smart meter sniffing pattern
const smartMeterPattern = [
    { duration: 3500, level: 1 },
    { duration: 1750, level: 0 },
    { duration: 3500, level: 1 },
    { duration: 1750, level: 0 }
];

console.log('Starting smart meter sniffer...');
CHANNELS.forEach(channel => {
    smartMeterSniffer.setChannel(channel);
    smartMeterSniffer.receive(smartMeterPattern, 30);
});
console.log('Smart meter sniffing completed');
