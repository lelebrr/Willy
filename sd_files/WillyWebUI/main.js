function updateStatus() {
    fetch('/api/status')
        .then(r => r.json())
        .then(data => {
            document.getElementById('battery').textContent = `Bat: ${data.battery}%`;
            document.getElementById('heap').textContent = Math.round(data.free_heap / 1024);
            document.getElementById('psram').textContent = Math.round(data.psram_free / 1024);
            document.getElementById('cpu').textContent = data.cpu_freq;
            document.getElementById('rssi').textContent = data.wifi_rssi;
            document.getElementById('ip').textContent = data.ip;

            let uptime = data.uptime;
            let mins = Math.floor(uptime / 60);
            let secs = uptime % 60;
            document.getElementById('uptime').textContent = `Ativo: ${mins}:${secs < 10 ? '0' : ''}${secs}`;
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

function updateVersion() {
    fetch('/api/version')
        .then(r => r.json())
        .then(data => {
            document.getElementById('version').textContent = `v${data.version}`;
        })
        .catch(() => {});
}

function reboot() {
    confirm("Reiniciar o Willy?") && fetch('/api/reboot', { method: 'POST' });
}

function clearLogs() {
    fetch('/api/logs/clear', { method: 'POST' })
        .then(() => updateLogs());
}

let statusTimer = null, logTimer = null;
function startPolling() {
    if (statusTimer) return;
    updateStatus();
    updateLogs();
    updateVersion();
    statusTimer = setInterval(updateStatus, 3000);
    logTimer = setInterval(updateLogs, 5000);
}
function stopPolling() {
    clearInterval(statusTimer);
    clearInterval(logTimer);
    statusTimer = null;
    logTimer = null;
}
// Pausa o polling com a aba oculta (economiza bateria/dados)
document.addEventListener('visibilitychange', () => {
    if (document.hidden) stopPolling();
    else startPolling();
});
startPolling();
