const groups = {
    power: [
        ['Power Consumed', 'W'],
        ['Power Returned', 'W']
    ],
    energy: [],
    phase: [],
    quality: [
        ['Short Power Outages', 'Counter'],
        ['Long Power Outages', 'Counter'],
        ['Short Power Drops Phase 1', 'Counter'],
        ['Short Power Peaks Phase 1', 'Counter']
    ]
};
const texts = ['P1 Meter ID', 'P1 Version', 'Actual Tariff Group'];
const values = {};
let sensorData = [],
    textData = [];
const $ = id => document.getElementById(id);
const safe = s => String(s).replace(/[&<>"']/g, c => ({
    '&': '&amp;',
    '<': '&lt;',
    '>': '&gt;',
    '"': '&quot;',
    "'": '&#39;'
} [c]));

function idFor(prefix, name) {
    return prefix + '-' + String(name).replace(/[^a-z0-9]+/gi, '-').toLowerCase()
}

function available(name) {
    const item = sensorData.find(x => x.name === name);
    return item && item.has_state && Number.isFinite(Number(item.value))
}

function firstAvailable(names) {
    return names.find(available)
}

function addIfAvailable(list, name, unit, title) {
    if (available(name)) list.push([name, unit, title || name])
}

function unitFor(name) {
    if (/^Power (Consumed|Returned)/.test(name)) return 'W';
    if (/Reactive Power/.test(name)) return 'kVAr';
    if (/Apparent Power/.test(name)) return 'kVA';
    if (/Demand/.test(name)) return 'kW';
    if (/Power Factor/.test(name)) return '';
    if (/Energy/.test(name)) return 'kWh';
    if (/Gas Consumed/.test(name)) return name.includes('GJ') ? 'GJ' : 'm³';
    if (/Water|Sub-meter/.test(name)) return 'm³';
    if (/Current/.test(name)) return 'A';
    if (/Voltage/.test(name)) return 'V';
    if (/Frequency/.test(name)) return 'Hz';
    if (/Factor/.test(name)) return '';
    if (/Time|Period/.test(name)) return 's';
    return 'value'
}

function displayValue(name, raw) {
    const n = Number(raw);
    if (!Number.isFinite(n)) return String(raw);
    if (name === 'Power Consumed' || name === 'Power Returned' || /^Power (Consumed|Returned) Phase/.test(name)) return String((n * 1000).toFixed(1));
    return String(n)
}

function card(item) {
    const name = item[0],
        unit = item[1],
        title = item[2] || name;
    return `<article class="card"><h3>${safe(title)}</h3><div class="value">${safe(displayValue(name, itemValue(name)))} </div><div class="meta">${safe(unit || 'Counter')}</div></article>`
}

function itemValue(name) {
    const item = sensorData.find(x => x.name === name);
    return item ? item.value : ''
}

function info(name, value) {
    return `<div class="info"><strong>${safe(name)}</strong><span>${safe(value || 'Unavailable')}</span></div>`
}

function buildMainGroups() {
    groups.energy = [];
    groups.phase = [];
    addIfAvailable(groups.energy, 'Energy Consumed Tariff 1', 'kWh');
    addIfAvailable(groups.energy, 'Energy Consumed Tariff 2', 'kWh');
    addIfAvailable(groups.energy, 'Energy Consumed Tariff 3', 'kWh');
    addIfAvailable(groups.energy, 'Energy Consumed Tariff 4', 'kWh');
    addIfAvailable(groups.energy, 'Energy Returned Tariff 1', 'kWh');
    addIfAvailable(groups.energy, 'Energy Returned Tariff 2', 'kWh');
    addIfAvailable(groups.energy, 'Energy Returned Tariff 3', 'kWh');
    addIfAvailable(groups.energy, 'Energy Returned Tariff 4', 'kWh');
    const gas = firstAvailable(['Gas Consumed', 'Gas Consumed BE', 'Gas Consumed GJ']);
    if (gas) addIfAvailable(groups.energy, gas, unitFor(gas), 'Gas Consumed');
    for (let phase = 1; phase <= 3; phase++) {
        [
            ['Power Consumed Phase ' + phase, 'W'],
            ['Power Returned Phase ' + phase, 'W'],
            ['Current Phase ' + phase, 'A'],
            ['Voltage Phase ' + phase, 'V']
        ].forEach(x => addIfAvailable(groups.phase, x[0], x[1]));
    }
}

function renderMain() {
    buildMainGroups();
    Object.entries(groups).forEach(([group, items]) => $(group + '-cards').innerHTML = items.length ? items.map(card).join('') : '<p class="hint">No current values available.</p>');
    const infoNames = texts.filter(name => {
        const x = textData.find(i => i.name === name);
        return x && x.state
    });
    $('info-cards').innerHTML = infoNames.length ? infoNames.map(name => {
        const x = textData.find(i => i.name === name);
        return info(name, x.state)
    }).join('') : '<p class="hint">No meter metadata available.</p>'
}

function category(name) {
    if (/Tariff|Energy/.test(name)) return 'Energy and tariffs';
    if (/Phase|Current|Voltage|Power Factor|Frequency|Power /.test(name)) return 'Electricity and phases';
    if (/Gas|Water|Thermal|Sub-meter/.test(name)) return 'External meters';
    if (/Failure|Outage|Drop|Peak|Sag|Swell/.test(name)) return 'Power quality and diagnostics';
    return 'Other DSMR values'
}

function renderDetails() {
    const rows = [];
    sensorData.filter(x => x.has_state).forEach(x => rows.push({
        name: x.name,
        value: displayValue(x.name, x.value),
        unit: unitFor(x.name)
    }));
    textData.filter(x => x.state).forEach(x => rows.push({
        name: x.name,
        value: x.state,
        unit: 'text'
    }));
    const grouped = {};
    rows.forEach(x => (grouped[category(x.name)] ??= []).push(x));
    $('details-content').innerHTML = Object.entries(grouped).sort().map(([group, items]) => `<section class="details-group"><h3>${safe(group)}</h3><div class="details-grid">${items.sort((a, b) => a.name.localeCompare(b.name)).map(x => `<div class="details-card"><strong>${safe(x.name)}</strong><span>${safe(x.value)}</span><small>${safe(x.unit)}</small></div>`).join('')}</div></section>`).join('') || '<p class="hint">No sensor currently has a valid value.</p>'
}

function renderChart(id, maxId, data, unit) {
    const el = $(id),
        maxEl = $(maxId),
        numbers = (data || []).map(v => typeof v === 'number' && Number.isFinite(v) ? v : null),
        dailyMax = Math.max(0, ...numbers.filter(v => v != null));
    el.innerHTML = numbers.map((value, hour) => {
        const height = value == null || dailyMax === 0 ? 0 : Math.max(3, value / dailyMax * 100);
        const label = value == null ? 'No data' : value.toFixed(3) + ' ' + unit;
        return `<div class="bar-wrap" data-hour="${String(hour).padStart(2, '0')}" title="${String(hour).padStart(2, '0')}:00 — ${label}"><div class="bar${value == null ? ' empty' : ''}" style="height:${height}%"></div></div>`
    }).join('');
    maxEl.textContent = dailyMax ? dailyMax.toFixed(3) + ' ' + unit : 'No data'
}

function renderHistory(history) {
    const enabled = !!(history && history.enabled);
    $('history-toggle').checked = enabled;
    $('history-graphs').hidden = !enabled;
    $('history-setting-status').textContent = enabled ? 'On — writes history to flash.' : 'Off — no history is written to flash.';
    if (!enabled) return;
    renderChart('chart-consumed', 'chart-consumed-max', history.energy_consumed, 'kWh');
    renderChart('chart-returned', 'chart-returned-max', history.energy_returned, 'kWh');
    renderChart('chart-gas', 'chart-gas-max', history.gas_consumed, 'm³');
    $('history-day').value = String(history.day_offset != null ? history.day_offset : 0)
}
async function load() {
    try {
        const day = $('history-day').value;
        const response = await fetch('/api/dashboard?day=' + encodeURIComponent(day), {
            cache: 'no-store'
        });
        if (!response.ok) throw new Error('Dashboard data unavailable');
        const data = await response.json();
        sensorData = data.sensors || [];
        textData = data.text_sensors || [];
        renderMain();
        renderDetails();
        window.lastHistory = data.history || {};
        renderHistory(data.history);
        const any = sensorData.some(x => x.has_state) || textData.some(x => x.state);
        $('connection-status').textContent = any ? 'Live readings' : 'Waiting for P1 data';
        $('status-dot').className = 'status-dot ' + (any ? 'live' : '');
        $('notice').classList.remove('visible')
    } catch (error) {
        $('connection-status').textContent = 'Dashboard data unavailable';
        $('status-dot').className = 'status-dot error';
        $('notice').textContent = 'Could not read meter values. The device may be restarting or the P1 telegram is not available.';
        $('notice').classList.add('visible')
    }
}
let pendingHistoryState = null;

function openHistoryModal(enabled, history) {
    pendingHistoryState = enabled;
    $('history-modal-title').textContent = enabled ? 'Enable On-Device History?' : 'Disable On-Device History?';
    $('history-modal-text').textContent = enabled ? 'This writes history to flash every 30 minutes using alternating files. For ' + (history.device_model || 'this device') + ', the estimated additional flash-wear lifetime is approximately ' + (history.wear_min || 'a few years') + ' to ' + (history.wear_max || 'several years') + '. This is an estimate, not a guarantee, and enabling history can wear the device faster than leaving it off.' : 'This increases the device lifespan by stopping history writes. All already saved on-device history data will be deleted permanently. Confirm only if you no longer need the graphs.';
    $('history-modal').classList.add('open')
}

function closeHistoryModal() {
    $('history-modal').classList.remove('open');
    pendingHistoryState = null
}
async function saveHistoryState() {
    const button = $('history-confirm');
    button.disabled = true;
    try {
        const response = await fetch('/api/history-settings', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/x-www-form-urlencoded'
            },
            body: 'enabled=' + (pendingHistoryState ? 'true' : 'false')
        });
        if (!response.ok) throw new Error('Could not save history setting');
        closeHistoryModal();
        await load()
    } catch (error) {
        $('notice').textContent = error.message;
        $('notice').classList.add('visible')
    }
    button.disabled = false
}
$('history-toggle').addEventListener('change', () => {
    const data = window.lastHistory || {
        device_model: 'this device',
        wear_min: 'a few years',
        wear_max: 'several years'
    };
    openHistoryModal($('history-toggle').checked, data);
    $('history-toggle').checked = !!(window.lastHistory && window.lastHistory.enabled)
});
$('history-cancel').addEventListener('click', closeHistoryModal);
$('history-confirm').addEventListener('click', saveHistoryState);
$('history-modal').addEventListener('click', event => {
    if (event.target === $('history-modal')) closeHistoryModal()
});
$('details-button').addEventListener('click', () => {
    $('details-modal').classList.add('open')
});
$('details-close').addEventListener('click', () => {
    $('details-modal').classList.remove('open')
});
$('details-modal').addEventListener('click', event => {
    if (event.target === $('details-modal')) $('details-modal').classList.remove('open')
});
$('refresh').addEventListener('click', async () => {
    const button = $('refresh');
    button.disabled = true;
    await load();
    button.disabled = false
});
$('history-day').addEventListener('change', load);
load();
setInterval(load, 10000);