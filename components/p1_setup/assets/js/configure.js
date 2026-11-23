const $ = id => document.getElementById(id);
let savedSettings = {};

function setMode() {
    $('static_fields').classList.toggle('hidden', document.querySelector('[name=ip_mode]:checked').value !== 'static')
}
document.querySelectorAll('[name=ip_mode]').forEach(x => x.addEventListener('change', setMode));
$('networks').addEventListener('change', () => {
    if ($('networks').value) $('ssid').value = $('networks').value
});
async function scan() {
    const b = $('scan'),
        s = $('network_status');
    b.disabled = true;
    s.textContent = 'Scanning…';
    try {
        let r, items;
        do {
            r = await fetch('/api/scan');
            if (r.status === 202) {
                await new Promise(resolve => setTimeout(resolve, 500));
                continue
            }
            if (!r.ok) throw new Error(await r.text());
            items = await r.json()
        } while (!items);
        const select = $('networks');
        select.innerHTML = '<option value="">Select a network</option>';
        for (const item of items) {
            const o = document.createElement('option');
            o.value = item.ssid;
            o.textContent = `${item.ssid} (${item.rssi} dBm)`;
            select.append(o)
        }
        s.textContent = items.length ? `Found ${items.length} network(s).` : 'No networks found.'
    } catch (e) {
        s.textContent = e.message
    } finally {
        b.disabled = false
    }
}
$('scan').addEventListener('click', scan);
async function load() {
    try {
        const r = await fetch('/api/settings', {
            cache: 'no-store'
        });
        if (!r.ok) throw new Error('Could not load saved settings.');
        savedSettings = await r.json();
        $('ssid').value = savedSettings.wifi_ssid || '';
        $('wifi_password').value = savedSettings.wifi_password || '';
        document.querySelector(`[name=ip_mode][value=${savedSettings.ip_mode === 'static' ? 'static' : 'dhcp'}]`).checked = true;
        $('ip').value = savedSettings.static_ip === '0.0.0.0' ? '' : savedSettings.static_ip;
        $('gateway').value = savedSettings.gateway === '0.0.0.0' ? '' : savedSettings.gateway;
        $('subnet').value = savedSettings.subnet === '0.0.0.0' ? '255.255.255.0' : savedSettings.subnet;
        $('dns').value = savedSettings.dns === '0.0.0.0' ? '' : savedSettings.dns;
        setMode()
    } catch (e) {
        $('status').textContent = e.message
    }
}
$('settings').addEventListener('submit', async e => {
    e.preventDefault();
    const s = $('status');
    s.textContent = 'Saving settings…';
    try {
        const body = new URLSearchParams(new FormData(e.target));
        // Preserve MQTT settings (managed from the menu) so a Wi-Fi-only save does not clear them.
        body.set('mqtt_broker', savedSettings.mqtt_broker || '');
        body.set('mqtt_port', String(savedSettings.mqtt_port || 1883));
        body.set('mqtt_username', savedSettings.mqtt_username || '');
        body.set('mqtt_password', savedSettings.mqtt_password || '');
        body.set('mqtt_enabled', savedSettings.mqtt_enabled ? 'on' : 'off');
        body.set('api_enabled', savedSettings.api_enabled ? 'on' : 'off');
        body.set('homewizard_enabled', savedSettings.homewizard_enabled ? 'on' : 'off');
        const r = await fetch('/api/settings', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/x-www-form-urlencoded'
            },
            body
        });
        if (!r.ok) throw new Error(await r.text());
        s.textContent = 'Settings saved and applied.'
    } catch (err) {
        s.textContent = err.message
    }
});
load();
// scan();
