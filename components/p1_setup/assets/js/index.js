document.getElementById('logout').addEventListener('click', async () => {
    const button = document.getElementById('logout');
    button.disabled = true;
    try {
        const response = await fetch('/api/logout', {
            method: 'POST'
        });
        if (!response.ok) throw new Error('Could not sign out.');
        window.close();
        if (!window.closed) document.body.innerHTML = '<main style="margin:48px auto;padding:24px;max-width:520px;font-family:system-ui,sans-serif;color:#14231d"><h1>Signed out</h1><p>Your setup session has ended. You can close this tab.</p></main>'
    } catch (error) {
        button.disabled = false;
        window.alert(error.message)
    }
});