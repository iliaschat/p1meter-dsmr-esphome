document.getElementById('reset').addEventListener('click', async () => {
    if (!confirm('Erase all saved settings and restart?')) return;
    const s = document.getElementById('status');
    s.textContent = 'Resetting…';
    try {
        const r = await fetch('/api/factory-reset', {
            method: 'POST'
        });
        if (!r.ok) throw new Error(await r.text());
        document.getElementById('restart_overlay').hidden = false;
        let n = 0;
        const retry = async () => {
            n++;
            try {
                const q = await fetch('/', {
                    cache: 'no-store'
                });
                if (q.ok) {
                    location.reload();
                    return
                }
            } catch (e) {}
            document.getElementById('retry_status').textContent = 'Still waiting… retry ' + n;
            setTimeout(retry, 2000)
        };
        setTimeout(retry, 10000)
    } catch (e) {
        s.textContent = e.message
    }
});