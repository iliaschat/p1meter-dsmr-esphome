document.querySelectorAll('.password-toggle').forEach(button => button.addEventListener('click', () => {
    const input = document.getElementById(button.getAttribute('aria-controls'));
    const visible = input.type === 'password';
    input.type = visible ? 'text' : 'password';
    button.setAttribute('aria-pressed', String(visible));
    button.setAttribute('aria-label', visible ? 'Hide password' : 'Show password');
    button.title = visible ? 'Hide password' : 'Show password'
}));
document.getElementById('password').addEventListener('submit', async e => {
    e.preventDefault();
    const s = document.getElementById('status');
    const p = document.getElementById('password_input').value;
    const c = document.getElementById('confirm').value;
    s.className = 'status';
    if (p !== c) {
        s.className = 'status error';
        s.textContent = 'The passwords do not match.';
        return
    }
    if (!/^[\x20-\x7e]{8,63}$/.test(p)) {
        s.className = 'status error';
        s.textContent = 'Use 8 to 63 printable ASCII characters.';
        return
    }
    s.textContent = 'Saving…';
    try {
        const r = await fetch('/api/setup-password', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/x-www-form-urlencoded'
            },
            body: new URLSearchParams({
                password: p
            })
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
    } catch (err) {
        s.className = 'status error';
        s.textContent = err.message
    }
});