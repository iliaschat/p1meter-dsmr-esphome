const $ = id => document.getElementById(id);
let apiPreviousState = true;
let apiPendingState = null;
let apiModalOpen = false;

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
        const d = await r.json();
        $('ssid').value = d.wifi_ssid || '';
        $('wifi_password').value = d.wifi_password || '';
        $('broker').value = d.mqtt_broker || '';
        $('port').value = d.mqtt_port || 1883;
        $('username').value = d.mqtt_username || '';
        $('mqtt_password').value = d.mqtt_password || '';
        document.getElementById("mqtt_enabled").checked = Boolean(d.mqtt_enabled);
        document.getElementById("mqtt_enabled").dispatchEvent(new Event("change"));
        document.getElementById("homewizard_enabled").checked = Boolean(d.homewizard_enabled);
        document.getElementById("api_enabled").checked = d.api_enabled !== false;
        apiPreviousState = d.api_enabled !== false;
        document.querySelector(`[name=ip_mode][value=${d.ip_mode === 'static' ? 'static' : 'dhcp'}]`).checked = true;
        $('ip').value = d.static_ip === '0.0.0.0' ? '' : d.static_ip;
        $('gateway').value = d.gateway === '0.0.0.0' ? '' : d.gateway;
        $('subnet').value = d.subnet === '0.0.0.0' ? '255.255.255.0' : d.subnet;
        $('dns').value = d.dns === '0.0.0.0' ? '' : d.dns;
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
        const r = await fetch('/api/settings', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/x-www-form-urlencoded'
            },
            body: new URLSearchParams(new FormData(e.target))
        });
        if (!r.ok) throw new Error(await r.text());
        s.textContent = 'Settings saved and applied.'
    } catch (err) {
        s.textContent = err.message
    }
});
load();
// scan();