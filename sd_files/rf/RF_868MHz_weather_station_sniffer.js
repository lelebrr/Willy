// RF 868MHz Weather Station Sniffer
// Advanced sniffer for weather station data
// Configure frequency and modulation before use

const RF868 = require('rf868');
const weatherSniffer = new RF868();

// Configuration
const FREQUENCY = 868.35; // MHz
const MODULATION = 'FSK';
const PULSE_WIDTH = 200; // microseconds
const CHANNELS = [868.35, 868.40, 868.45];

weatherSniffer.setFrequency(FREQUENCY);
weatherSniffer.setModulation(MODULATION);
weatherSniffer.setPulseWidth(PULSE_WIDTH);

// Weather station sniffing pattern
const weatherPattern = [
    { duration: 1000, level: 1 },
    { duration: 500, level: 0 },
    { duration: 1000, level: 1 },
    { duration: 500, level: 0 }
];

console.log('Starting weather station sniffer...');
CHANNELS.forEach(channel => {
    weatherSniffer.setChannel(channel);
    weatherSniffer.receive(weatherPattern, 10);
});
console.log('Weather station sniffing completed');
