// RF 915MHz GPS Jammer
// Advanced jamming for GPS signals
// Configure frequency and modulation before use

const RF915 = require('rf915');
const gpsJammer = new RF915();

// Configuration
const FREQUENCY = 915.0; // MHz
const MODULATION = 'FSK';
const PULSE_WIDTH = 150; // microseconds
const REPEAT = 2000; // times

gpsJammer.setFrequency(FREQUENCY);
gpsJammer.setModulation(MODULATION);
gpsJammer.setPulseWidth(PULSE_WIDTH);

// GPS jamming pattern
const gpsPattern = [
    { duration: 300, level: 1 },
    { duration: 150, level: 0 },
    { duration: 300, level: 1 },
    { duration: 150, level: 0 }
];

console.log('Starting GPS jammer...');
gpsJammer.transmit(gpsPattern, REPEAT);
console.log('GPS jamming completed');
