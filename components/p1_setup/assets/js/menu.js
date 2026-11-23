const $ = id => document.getElementById(id);
let settings = {};
let historyEnabled = false;
let pendingApi = null;
let pendingHomewizard = null;

function isEnabled(value) { return value === true || value === 'true' || value === 1 || value === '1'; }

function indicator(el, on) {
  el.classList.toggle('on', on);
  el.classList.toggle('off', !on);
}

async function loadSettings() {
  try {
    const r = await fetch('/api/settings', { cache: 'no-store' });
    if (!r.ok) throw new Error('Could not load settings');
    settings = await r.json();
  } catch (e) {
    console.warn(e);
  }
  try {
    const d = await fetch('/api/dashboard', { cache: 'no-store' });
    const j = await d.json();
    historyEnabled = !!(j.history && j.history.enabled);
  } catch (e) {}
  updateIndicators();
  updateHistoryLabel();
}

function updateIndicators() {
  const apiOn = isEnabled(settings.api_enabled);
  const mqttOn = isEnabled(settings.mqtt_enabled);
  const homewizardOn = isEnabled(settings.homewizard_enabled);
  indicator($('api-indicator'), apiOn);
  indicator($('mqtt-indicator'), mqttOn);
  indicator($('homewizard-indicator'), homewizardOn);
  $('api-label').textContent = apiOn ? 'Disable ESPHome API' : 'Enable ESPHome API';
  $('mqtt-label').textContent = mqttOn ? 'Disable MQTT' : 'Enable MQTT';
  $('homewizard-label').textContent = homewizardOn ? 'Disable HomeWizzard' : 'Enable HomeWizzard';
}

function updateHistoryLabel() {
  $('history-label').textContent = historyEnabled ? 'Disable History' : 'Enable History';
}

async function postSettings(extra) {
  const body = new URLSearchParams({
    wifi_ssid: settings.wifi_ssid || '',
    wifi_password: settings.wifi_password || '',
    mqtt_broker: settings.mqtt_broker || '',
    mqtt_port: String(settings.mqtt_port || 1883),
    mqtt_username: settings.mqtt_username || '',
    mqtt_password: settings.mqtt_password || '',
    ip_mode: settings.ip_mode || 'dhcp',
    static_ip: settings.static_ip || '0.0.0.0',
    gateway: settings.gateway || '0.0.0.0',
    subnet: settings.subnet || '255.255.255.0',
    dns: settings.dns || '0.0.0.0',
    mqtt_enabled: settings.mqtt_enabled ? 'on' : 'off',
    homewizard_enabled: settings.homewizard_enabled ? 'on' : 'off',
    api_enabled: settings.api_enabled ? 'on' : 'off',
    ...extra
  });
  const r = await fetch('/api/settings', {
    method: 'POST',
    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
    body
  });
  if (!r.ok) throw new Error(await r.text());
  await loadSettings();
}

function openModal(id) { $(id).classList.add('open'); }
function closeModal(id) { $(id).classList.remove('open'); }
document.querySelectorAll('.modal').forEach(modal => {
  modal.addEventListener('click', ev => { if (ev.target === modal) closeModal(modal.id); });
});

// Communication submenu toggle
const commToggle = $('menu-communication');
const submenu = $('submenu-communication');
commToggle.addEventListener('click', ev => {
  if (submenu.contains(ev.target)) return; // clicking a submenu item must not toggle the submenu
  const expanded = commToggle.getAttribute('aria-expanded') === 'true';
  commToggle.setAttribute('aria-expanded', String(!expanded));
  submenu.hidden = expanded;
});

// Configure WiFi -> open configure page inside a modal
$('menu-configure-wifi').addEventListener('click', () => {
  $('configure-iframe').src = '/configure';
  openModal('configure-modal');
});
$('configure-modal-close').addEventListener('click', () => {
  closeModal('configure-modal');
  $('configure-iframe').src = 'about:blank';
});

// Reboot
$('menu-reboot').addEventListener('click', () => openModal('reboot-modal'));
$('menu-ota').addEventListener('click', () => {
  $('ota-iframe').src = '/ota';
  openModal('ota-modal');
});
$('ota-modal-close').addEventListener('click', () => {
  closeModal('ota-modal');
  $('ota-iframe').src = 'about:blank';
});
$('modal-reboot').addEventListener('click', async () => {
  if (!confirm('Reboot the P1 meter now?')) return;
  try {
    await fetch('/api/reboot', { method: 'POST' });
    const overlay = $('modal-reboot-overlay');
    overlay.hidden = false;
    const retry = async () => {
      try {
        const q = await fetch('/', { cache: 'no-store' });
        if (q.ok) { location.reload(); return }
      } catch (e) {}
      setTimeout(retry, 2000);
    };
    setTimeout(retry, 10000);
  } catch (e) { $('modal-reboot-status').textContent = e.message; }
});

// Factory reset
$('menu-factory-reset').addEventListener('click', () => openModal('factory-reset-modal'));
$('modal-factory-reset').addEventListener('click', async () => {
  if (!confirm('Erase all saved settings and restart?')) return;
  try {
    await fetch('/api/factory-reset', { method: 'POST' });
    const overlay = $('modal-factory-reset-overlay');
    overlay.hidden = false;
    const retry = async () => {
      try {
        const q = await fetch('/', { cache: 'no-store' });
        if (q.ok) { location.reload(); return }
      } catch (e) {}
      setTimeout(retry, 2000);
    };
    setTimeout(retry, 10000);
  } catch (e) { $('modal-factory-reset-status').textContent = e.message; }
});

// Change Password
$('menu-password').addEventListener('click', () => openModal('password-modal'));
$('modal-password-cancel').addEventListener('click', () => closeModal('password-modal'));
$('modal-password-save').addEventListener('click', async () => {
  const p = $('modal-password-input').value;
  const c = $('modal-confirm').value;
  const s = $('modal-password-status');
  s.className = 'status';
  if (p !== c) { s.className = 'status error'; s.textContent = 'The passwords do not match.'; return }
  if (!/^[\x20-\x7e]{8,63}$/.test(p)) { s.className = 'status error'; s.textContent = 'Use 8 to 63 printable ASCII characters.'; return }
  s.textContent = 'Saving…';
  try {
    await fetch('/api/setup-password', { method: 'POST', headers: { 'Content-Type': 'application/x-www-form-urlencoded' }, body: new URLSearchParams({ password: p }) });
    const overlay = $('modal-password-overlay');
    overlay.hidden = false;
    const retry = async () => {
      try {
        const q = await fetch('/', { cache: 'no-store' });
        if (q.ok) { location.reload(); return }
      } catch (e) {}
      setTimeout(retry, 2000);
    };
    setTimeout(retry, 10000);
  } catch (e) { s.className = 'status error'; s.textContent = e.message; }
});

// ESPHome API toggle
$('menu-api').addEventListener('click', async () => {
  pendingApi = !isEnabled(settings.api_enabled);
  try {
    await postSettings({ api_enabled: pendingApi ? 'on' : 'off' });
  } catch (e) { alert('Could not change ESPHome Native API: ' + e.message); }
});

// HomeWizzard toggle
$('menu-homewizard').addEventListener('click', async () => {
  pendingHomewizard = !isEnabled(settings.homewizard_enabled);
  try {
    await postSettings({ homewizard_enabled: pendingHomewizard ? 'on' : 'off' });
  } catch (e) { alert('Could not change HomeWizzard: ' + e.message); }
});

// MQTT -> open configure-mqtt modal
let mqttModalOpen = false;
$('menu-mqtt').addEventListener('click', () => {
  $('mqtt-broker').value = settings.mqtt_broker || '';
  $('mqtt-port').value = settings.mqtt_port || 1883;
  $('mqtt-username').value = settings.mqtt_username || '';
  $('mqtt-password').value = settings.mqtt_password || '';
  $('mqtt-enabled').checked = isEnabled(settings.mqtt_enabled);
  $('mqtt-status').textContent = '';
  openModal('configure-mqtt-modal');
  mqttModalOpen = true;
});
$('mqtt-cancel').addEventListener('click', () => { closeModal('configure-mqtt-modal'); mqttModalOpen = false; });
$('mqtt-save').addEventListener('click', async () => {
  const s = $('mqtt-status');
  const enabled = $('mqtt-enabled').checked;
  const broker = $('mqtt-broker').value.trim();
  if (enabled && !broker) { s.textContent = 'Enter an MQTT broker host when enabling MQTT.'; return }
  try {
    await postSettings({
      mqtt_enabled: enabled ? 'on' : 'off',
      mqtt_broker: broker,
      mqtt_port: $('mqtt-port').value || 1883,
      mqtt_username: $('mqtt-username').value,
      mqtt_password: $('mqtt-password').value
    });
    closeModal('configure-mqtt-modal');
    mqttModalOpen = false;
  } catch (e) { s.textContent = e.message; }
});

// Enable / Disable History
$('menu-history').addEventListener('click', () => {
  $('history-modal-title').textContent = historyEnabled ? 'Disable On-Device History?' : 'Enable On-Device History?';
  $('history-modal-text').textContent = historyEnabled
    ? 'This increases the device lifespan by stopping history writes. All already saved on-device history data will be deleted permanently. Confirm only if you no longer need the graphs.'
    : 'This writes history to flash every 30 minutes using alternating files. This can reduce the device flash lifespan over time.';
  openModal('history-modal');
});
$('history-cancel').addEventListener('click', () => closeModal('history-modal'));
$('history-confirm').addEventListener('click', async () => {
  try {
    const response = await fetch('/api/history-settings', {
      method: 'POST',
      headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
      body: 'enabled=' + !historyEnabled
    });
    if (!response.ok) throw new Error(await response.text());
    closeModal('history-modal');
    await loadSettings();
  } catch (e) { alert('Could not change history setting: ' + e.message); }
});

// Logout
$('logout').addEventListener('click', async () => {
  const b = $('logout'); b.disabled = true;
  try {
    const r = await fetch('/api/logout', { method: 'POST' });
    if (!r.ok) throw new Error('Could not sign out.');
    window.close();
    if (!window.closed) document.body.innerHTML = '<main style="margin:48px auto;padding:24px;max-width:520px;font-family:system-ui,sans-serif;color:#14231d"><h1>Signed out</h1><p>Your setup session has ended. You can close this tab.</p></main>';
  } catch (e) { b.disabled = false; window.alert(e.message); }
});

loadSettings();
