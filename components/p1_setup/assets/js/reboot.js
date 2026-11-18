const overlay = document.getElementById('restart_overlay');

function showRestart() {
    overlay.hidden = false;
    let attempts = 0;
    const retry = async () => {
        attempts++;
        try {
            const r = await fetch('/', {
                cache: 'no-store'
            });
            if (r.ok) {
                location.reload();
                return
            }
        } catch (e) {}
        document.getElementById('retry_status').textContent = `Still waiting for the device… retry ${attempts}`;
        setTimeout(retry, 2000)
    };
    setTimeout(retry, 10000)
}
document.getElementById('reboot').addEventListener('click', async () => {
    if (!confirm('Reboot the P1 meter now?')) return;
    const s = document.getElementById('status');
    s.textContent = 'Rebooting…';
    try {
        const r = await fetch('/api/reboot', {
            method: 'POST'
        });
        if (!r.ok) throw new Error(await r.text());
        showRestart()
    } catch (e) {
        s.textContent = e.message
    }
});