function updateStatus() {
    fetch('/api/status')
        .then(r => r.json())
        .then(data => {
            document.getElementById('battery').textContent = `Bat: ${data.battery}%`;
            document.getElementById('heap').textContent = Math.round(data.free_heap / 1024);
            document.getElementById('rssi').textContent = data.wifi_rssi;
            document.getElementById('ip').textContent = data.ip;

            let uptime = data.uptime;
            let mins = Math.floor(uptime / 60);
            let secs = uptime % 60;
            document.getElementById('uptime').textContent = `Uptime: ${mins}:${secs < 10 ? '0' : ''}${secs}`;
        });
}

function updateLogs() {
    fetch('/api/logs')
        .then(r => r.text())
        .then(text => {
            const area = document.getElementById('log-window');
            area.textContent = text;
            area.scrollTop = area.scrollHeight;
        });
}

function reboot() {
    confirm("Reboot Willy?") && fetch('/api/reboot', { method: 'POST' });
}

setInterval(updateStatus, 3000);
setInterval(updateLogs, 5000);
updateStatus();
updateLogs();
