document.querySelectorAll('.password-toggle').forEach(button => button.addEventListener('click', () => {
    const input = document.getElementById(button.getAttribute('aria-controls'));
    const visible = input.type === 'password';
    input.type = visible ? 'text' : 'password';
    button.setAttribute('aria-pressed', String(visible));
    button.setAttribute('aria-label', visible ? 'Hide password' : 'Show password');
    button.title = visible ? 'Hide password' : 'Show password'
}));
const fieldIds = {
    wifi_ssid: 'ssid',
    wifi_password: 'wifi_password',
    mqtt_broker: 'broker',
    mqtt_port: 'port',
    mqtt_username: 'username',
    mqtt_password: 'mqtt_password',
    static_ip: 'ip',
    gateway: 'gateway',
    subnet: 'subnet',
    dns: 'dns'
};

function clearFieldError(name) {
    const input = document.getElementById(fieldIds[name]);
    const error = document.getElementById(`error-${name}`);
    if (!input || !error) return;
    error.textContent = '';
    input.removeAttribute('aria-invalid');
    input.removeAttribute('aria-describedby');
    input.style.borderColor = ''
}

function clearFieldErrors() {
    Object.keys(fieldIds).forEach(clearFieldError)
}

function showFieldError(name, message) {
    const input = document.getElementById(fieldIds[name]);
    const error = document.getElementById(`error-${name}`);
    if (!input || !error) return false;
    error.textContent = message;
    input.setAttribute('aria-invalid', 'true');
    input.setAttribute('aria-describedby', error.id);
    input.style.borderColor = '#a33b2f';
    return true
}
Object.entries(fieldIds).forEach(([name, id]) => {
    const input = document.getElementById(id);
    const error = document.createElement('div');
    error.id = `error-${name}`;
    error.className = 'field-error';
    error.setAttribute('role', 'alert');
    error.style.color = '#a33b2f';
    error.style.fontSize = '13px';
    input.closest('.field').append(error);
    input.addEventListener('input', () => clearFieldError(name));
    input.addEventListener('invalid', event => {
        event.preventDefault();
        document.getElementById('status').textContent = '';
        showFieldError(name, input.validationMessage)
    })
});

function setRequired(id, required) {
    const input = document.getElementById(id);
    input.required = required;
    const marker = document.getElementById(`required-${id}`);
    marker.hidden = !required
}
['ssid', 'wifi_password', 'broker', 'port', 'mqtt_password', 'ip', 'gateway', 'subnet'].forEach(id => {
    const marker = document.createElement('span');
    marker.id = `required-${id}`;
    marker.textContent = ' *';
    marker.style.color = '#a33b2f';
    marker.setAttribute('aria-hidden', 'true');
    document.querySelector(`label[for="${id}"]`).append(marker)
});

function updateMqttSettings() {
    const enabled = document.getElementById('mqtt_enabled').checked;
    document.getElementById('mqtt_fields').classList.toggle('hidden', !enabled);
    setRequired('broker', enabled);
    setRequired('port', enabled);
    setRequired('mqtt_password', enabled && document.getElementById('username').value.length > 0)
}

function updateRequirements() {
    ['ssid', 'wifi_password'].forEach(id => setRequired(id, true));
    updateMqttSettings();
    const staticMode = document.querySelector('[name=ip_mode]:checked').value === 'static';
    ['ip', 'gateway', 'subnet'].forEach(id => setRequired(id, staticMode));
    const password = document.getElementById('wifi_password');
    password.setCustomValidity(!password.value || /^(?:[\x20-\x7e]{8,63}|[0-9a-fA-F]{64})$(?![\s\S])/.test(password.value) ? '' : 'Use 8 to 63 printable ASCII characters or 64 hexadecimal digits.')
}
document.getElementById('mqtt_enabled').addEventListener('change', () => {
    updateMqttSettings();
    ['mqtt_broker', 'mqtt_port', 'mqtt_username', 'mqtt_password'].forEach(clearFieldError)
});
document.getElementById('username').addEventListener('input', () => {
    updateRequirements();
    clearFieldError('mqtt_password')
});
document.getElementById('wifi_password').addEventListener('input', updateRequirements);
document.querySelectorAll('[name=ip_mode]').forEach(input => input.addEventListener('change', () => {
    updateRequirements();
    ['static_ip', 'gateway', 'subnet', 'dns'].forEach(clearFieldError)
}));
const originalSetMode = setMode;
setMode = function () {
    originalSetMode();
    updateRequirements()
};
updateRequirements();
$('api_enabled').addEventListener('change', () => {
    if (apiModalOpen) return;
    apiPendingState = $('api_enabled').checked;
    $('api_enabled').checked = apiPreviousState;
    apiModalOpen = true;
    $('api_confirm').hidden = false
});
$('api_cancel').addEventListener('click', () => {
    apiPendingState = null;
    apiModalOpen = false;
    $('api_confirm').hidden = true;
    $('api_enabled').checked = apiPreviousState
});
$('api_reboot_now').addEventListener('click', () => {
    $('api_enabled').checked = apiPendingState;
    apiPreviousState = apiPendingState;
    apiPendingState = null;
    apiModalOpen = false;
    $('api_confirm').hidden = true;
    $('settings').requestSubmit()
});