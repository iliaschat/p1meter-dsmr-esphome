function showRestart() {
    document.getElementById('restart_overlay').hidden = false;
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
document.getElementById('upload').addEventListener('submit', async e => {
    e.preventDefault();
    const s = document.getElementById('status');
    const form = e.target;
    const button = form.querySelector('button[type="submit"]') || form.querySelector('button');
    s.textContent = 'Uploading... do not disconnect power.';
    if (button) button.disabled = true;
    try {
        const r = await fetch('/update', {
            method: 'POST',
            body: new FormData(e.target)
        });
        const t = await r.text();
        if (!r.ok) throw new Error(t);
        s.textContent = t || 'Update successful. The device is restarting.';
        showRestart()
    } catch (err) {
        // The device may reboot before the browser receives the final HTTP
        // response. In that case fetch reports a network error even though
        // the OTA image was accepted and the device is restarting.
        if (err instanceof TypeError || err.message === 'Failed to fetch') {
            s.textContent = 'Update uploaded. The device is restarting.';
            showRestart();
            return
        }
        if (button) button.disabled = false;
        s.textContent = err.message || 'Firmware update failed.'
    }
});