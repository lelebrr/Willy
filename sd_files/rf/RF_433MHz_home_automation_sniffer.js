// RF 433MHz Home Automation Sniffer
// Advanced sniffer for home automation devices
// Configure frequency and modulation before use

const RF433 = require('rf433');
const homeSniffer = new RF433();

// Configuration
const FREQUENCY = 433.92; // MHz
const MODULATION = 'ASK';
const PULSE_WIDTH = 350; // microseconds
const CHANNELS = [433.92, 433.95, 433.98, 434.01];

homeSniffer.setFrequency(FREQUENCY);
homeSniffer.setModulation(MODULATION);
homeSniffer.setPulseWidth(PULSE_WIDTH);

// Home automation sniffing pattern
const homePattern = [
    { duration: 1500, level: 1 },
    { duration: 750, level: 0 },
    { duration: 1500, level: 1 },
    { duration: 750, level: 0 }
];

console.log('Starting home automation sniffer...');
CHANNELS.forEach(channel => {
    homeSniffer.setChannel(channel);
    homeSniffer.receive(homePattern, 12);
});
console.log('Home automation sniffing completed');
