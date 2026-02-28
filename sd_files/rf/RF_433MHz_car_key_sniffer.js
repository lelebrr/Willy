// RF 433MHz Car Key Sniffer
// Advanced sniffer for car key fobs
// Configure frequency and modulation before use

const RF433 = require('rf433');
const carKeySniffer = new RF433();

// Configuration
const FREQUENCY = 433.92; // MHz
const MODULATION = 'ASK';
const PULSE_WIDTH = 350; // microseconds
const CHANNELS = [433.92, 433.95, 433.98];

carKeySniffer.setFrequency(FREQUENCY);
carKeySniffer.setModulation(MODULATION);
carKeySniffer.setPulseWidth(PULSE_WIDTH);

// Car key sniffing pattern
const carKeyPattern = [
    { duration: 1800, level: 1 },
    { duration: 900, level: 0 },
    { duration: 1800, level: 1 },
    { duration: 900, level: 0 }
];

console.log('Starting car key sniffer...');
CHANNELS.forEach(channel => {
    carKeySniffer.setChannel(channel);
    carKeySniffer.receive(carKeyPattern, 20);
});
console.log('Car key sniffing completed');
