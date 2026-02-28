// RF 433MHz Remote Control Sniffer
// Advanced sniffer for remote control signals
// Configure frequency and modulation before use

const RF433 = require('rf433');
const remoteSniffer = new RF433();

// Configuration
const FREQUENCY = 433.92; // MHz
const MODULATION = 'ASK';
const PULSE_WIDTH = 350; // microseconds
const CHANNELS = [433.92, 433.95, 433.98, 434.01];

remoteSniffer.setFrequency(FREQUENCY);
remoteSniffer.setModulation(MODULATION);
remoteSniffer.setPulseWidth(PULSE_WIDTH);

// Remote control sniffing pattern
const remotePattern = [
    { duration: 1200, level: 1 },
    { duration: 600, level: 0 },
    { duration: 1200, level: 1 },
    { duration: 600, level: 0 }
];

console.log('Starting remote control sniffer...');
CHANNELS.forEach(channel => {
    remoteSniffer.setChannel(channel);
    remoteSniffer.receive(remotePattern, 15);
});
console.log('Remote control sniffing completed');
