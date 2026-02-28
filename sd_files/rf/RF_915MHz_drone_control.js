// RF 915MHz Drone Control
// Advanced drone control with frequency hopping
// Configure frequency and protocols before use

const RF915 = require('rf915');
const droneController = new RF915();

// Configuration
const FREQUENCY = 915.0; // MHz
const MODULATION = 'FSK';
const PULSE_WIDTH = 150; // microseconds
const HOPPING_CHANNELS = [915.0, 915.2, 915.4, 915.6];

droneController.setFrequency(FREQUENCY);
droneController.setModulation(MODULATION);
droneController.setPulseWidth(PULSE_WIDTH);

// Drone control pattern
const dronePattern = [
    { duration: 300, level: 1 },
    { duration: 700, level: 0 },
    { duration: 300, level: 1 },
    { duration: 700, level: 0 }
];

console.log('Starting drone control...');
droneController.transmit(dronePattern, 5);
console.log('Drone control completed');
