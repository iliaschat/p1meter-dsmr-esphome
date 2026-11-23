#include "web_assets.h"

namespace esphome::p1_setup::web_pages {

const char CONFIGURE_MQTT_PAGE[] PROGMEM = R"HTML(<!doctype html>
<html lang="en">

<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width,initial-scale=1">
    <title>MQTT Settings - P1 Meter</title>
    <!-- CSS: configure-mqtt -->
</head>

<body><a href="/">&larr; Home</a>
    <h1>MQTT Settings</h1>
    <p>Enter your MQTT broker details to enable MQTT, or leave the broker empty and uncheck "Enable MQTT" to disable it.</p>
    <form id="mqtt">
        <div class="field">
            <label for="mqtt-broker">Broker host</label>
            <input id="mqtt-broker" name="mqtt_broker" maxlength="64" placeholder="broker.example.com">
        </div>
        <div class="field">
            <label for="mqtt-port">Broker port</label>
            <input id="mqtt-port" name="mqtt_port" type="number" min="1" max="65535" value="1883">
        </div>
        <div class="field">
            <label for="mqtt-username">Username</label>
            <input id="mqtt-username" name="mqtt_username" maxlength="32" autocomplete="username">
        </div>
        <div class="field">
            <label for="mqtt-password">Password</label>
            <div class="password-field">
                <input id="mqtt-password" name="mqtt_password" type="password" maxlength="64" autocomplete="new-password">
                <button class="password-toggle" type="button" aria-controls="mqtt-password" aria-label="Show password" aria-pressed="false" title="Show password">
                    <svg class="eye-on" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">
                        <path d="M2 12s3.5-7 10-7 10 7 10 7-3.5 7-10 7S2 12 2 12Z" />
                        <circle cx="12" cy="12" r="3" />
                    </svg>
                    <svg class="eye-off" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">
                        <path d="M2 12s3.5-7 10-7 10 7 10 7-3.5 7-10 7S2 12 2 12Z" />
                        <path d="M4 4l16 16" />
                    </svg>
                </button>
            </div>
        </div>
        <div class="field">
            <label>
                <input id="mqtt-enabled" type="checkbox" style="width:auto;padding:0"> Enable MQTT
            </label>
        </div>
        <p class="status" id="mqtt-status" role="status"></p>
        <div class="actions">
            <button class="secondary" id="mqtt-cancel" type="button">Cancel</button>
            <button id="mqtt-save" type="button">Save</button>
        </div>
    </form>
    <!-- JS: configure-mqtt -->
    
</body></html>)HTML";

const char CONFIGURE_PAGE[] PROGMEM = R"HTML(<!doctype html>
<html lang="en">

<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width,initial-scale=1">
    <title>Configure P1 Meter</title>
    <style>:root{font-family:system-ui,sans-serif;color:#14231d;background:#f3f6f2}*{box-sizing:border-box}body{margin:0 auto;padding:22px;max-width:720px}a{color:#27734f}h1{font-size:25px;margin:16px 0 6px}h2{font-size:17px;margin:24px 0 8px}p,.hint{color:#52645b;line-height:1.5}.field{display:grid;gap:6px;margin:14px 0}label{font-weight:600}input,select{width:100%;padding:11px 12px;border:1px solid #bccbc1;border-radius:6px;background:#fff;font:inherit;color:#14231d}button{padding:11px 14px;border:1px solid #27734f;border-radius:6px;background:#27734f;color:#fff;font:inherit;font-weight:650;cursor:pointer}button.secondary{background:#fff;color:#27734f}.row{display:flex;gap:10px;align-items:center}.row>*{flex:1}.modes{display:flex;gap:22px;margin:12px 0}.modes label{font-weight:500}.status{min-height:24px;margin-top:12px;color:#27734f}.hidden{display:none}.actions{display:flex;gap:10px;flex-wrap:wrap;margin-top:22px}.overlay{position:fixed;inset:0;display:grid;place-items:center;background:rgba(243,246,242,.96);z-index:5;text-align:center;padding:24px}.overlay[hidden]{display:none}.panel{max-width:430px;background:#fff;border:1px solid #d3ded6;border-radius:12px;padding:28px;box-shadow:0 8px 30px #14231d22}.confirm-actions{display:flex;gap:10px;justify-content:center;margin-top:20px}.confirm-actions button{min-width:110px}.spinner{width:28px;height:28px;margin:0 auto 16px;border:3px solid #c8d8cd;border-top-color:#27734f;border-radius:50%;animation:spin 1s linear infinite}@keyframes spin{to{transform:rotate(360deg)}}@media(max-width:520px){body{padding:16px}.row{align-items:stretch;flex-direction:column}}</style>
     <style>.password-field{position:relative}.password-field input{padding-right:52px}.password-toggle{position:absolute;right:4px;top:50%;transform:translateY(-50%);width:40px;height:40px;padding:8px;display:grid;place-items:center;border:0;background:transparent;color:#52645b}.password-toggle:hover{color:#27734f}.password-toggle:focus-visible{outline:2px solid #27734f}.password-toggle svg{width:20px;height:20px}.password-toggle .eye-off{display:none}.password-toggle[aria-pressed="true"] .eye-off{display:block}.password-toggle[aria-pressed="true"] .eye-on{display:none}</style>
</head>

<body><a href="/">&larr; Home</a>
    <h1>Configure device</h1>
    <p>Configure network, Wi-Fi, and IP address settings.</p>
    <form id="settings">
        <div class="field">
            <label for="networks">Nearby Wi-Fi networks</label>
            <div class="row">
                <select id="networks">
                    <option value="">Select a network</option>
                </select>
                <button class="secondary" type="button" id="scan">Refresh scan</button>
            </div>
            <div id="network_status" role="status"></div>
        </div>
        <div class="field">
            <label for="ssid">Wi-Fi SSID</label>
            <input id="ssid" name="wifi_ssid" maxlength="32" required>
        </div>
        <div class="field">
            <label for="wifi_password">Wi-Fi password</label>
            <div class="password-field">
                <input id="wifi_password" name="wifi_password" type="password" maxlength="64" autocomplete="new-password">
                <button class="password-toggle" type="button" aria-controls="wifi_password" aria-label="Show password" aria-pressed="false" title="Show password"><svg class="eye-on" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">
                        <path d="M2 12s3.5-7 10-7 10 7 10 7-3.5 7-10 7S2 12 2 12Z" />
                        <circle cx="12" cy="12" r="3" />
                    </svg>
                    <svg class="eye-off" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">
                        <path d="M2 12s3.5-7 10-7 10 7 10 7-3.5 7-10 7S2 12 2 12Z" />
                        <path d="M4 4l16 16" />
                    </svg>
                </button>
            </div>
        </div>
        <h2>IP address</h2>
        <div class="modes">
            <label>
                <input type="radio" name="ip_mode" value="dhcp" checked> DHCP
            </label>
            <label>
                <input type="radio" name="ip_mode" value="static"> Static
            </label>
        </div>
        <section id="static_fields" class="hidden">
            <div class="field">
                <label for="ip">Device IP address</label>
                <input id="ip" name="static_ip" inputmode="decimal" placeholder="192.168.1.50">
            </div>
            <div class="field">
                <label for="gateway">Gateway</label>
                <input id="gateway" name="gateway" inputmode="decimal" placeholder="192.168.1.1">
            </div>
            <div class="field">
                <label for="subnet">Subnet mask</label>
                <input id="subnet" name="subnet" inputmode="decimal" value="255.255.255.0">
            </div>
            <div class="field">
                <label for="dns">DNS server (optional)</label>
                <input id="dns" name="dns" inputmode="decimal">
            </div>
        </section>
        <div class="actions">
            <button type="submit">Save settings</button>
        </div>
        <div id="status" class="status" role="status"></div>
    </form>
    <script>const $ =id =>document.getElementById(id);let savedSettings ={};function setMode() {$('static_fields').classList.toggle('hidden',document.querySelector('[name=ip_mode]:checked').value!=='static')} document.querySelectorAll('[name=ip_mode]').forEach(x =>x.addEventListener('change',setMode));$('networks').addEventListener('change',() =>{if ($('networks').value) $('ssid').value =$('networks').value});async function scan() {const b =$('scan'),s =$('network_status');b.disabled =true;s.textContent ='Scanning…';try {let r,items;do {r =await fetch('/api/scan');if (r.status===202) {await new Promise(resolve =>setTimeout(resolve,500));continue} if (!r.ok) throw new Error(await r.text());items =await r.json()} while (!items);const select =$('networks');select.innerHTML ='<option value="">Select a network</option>';for (const item of items) {const o =document.createElement('option');o.value =item.ssid;o.textContent =`${item.ssid} (${item.rssi} dBm)`;select.append(o)} s.textContent =items.length ?`Found ${items.length} network(s).` :'No networks found.'} catch (e) {s.textContent =e.message} finally {b.disabled =false}} $('scan').addEventListener('click',scan);async function load() {try {const r =await fetch('/api/settings',{cache:'no-store'});if (!r.ok) throw new Error('Could not load saved settings.');savedSettings =await r.json();$('ssid').value =savedSettings.wifi_ssid||'';$('wifi_password').value =savedSettings.wifi_password||'';document.querySelector(`[name=ip_mode][value=${savedSettings.ip_mode==='static' ? 'static' : 'dhcp'}]`).checked =true;$('ip').value =savedSettings.static_ip==='0.0.0.0' ?'' :savedSettings.static_ip;$('gateway').value =savedSettings.gateway==='0.0.0.0' ?'' :savedSettings.gateway;$('subnet').value =savedSettings.subnet==='0.0.0.0' ?'255.255.255.0' :savedSettings.subnet;$('dns').value =savedSettings.dns==='0.0.0.0' ?'' :savedSettings.dns;setMode()} catch (e) {$('status').textContent =e.message}} $('settings').addEventListener('submit',async e =>{e.preventDefault();const s =$('status');s.textContent ='Saving settings…';try {const body =new URLSearchParams(new FormData(e.target));body.set('mqtt_broker',savedSettings.mqtt_broker||'');body.set('mqtt_port',String(savedSettings.mqtt_port||1883));body.set('mqtt_username',savedSettings.mqtt_username||'');body.set('mqtt_password',savedSettings.mqtt_password||'');body.set('mqtt_enabled',savedSettings.mqtt_enabled ?'on' :'off');body.set('api_enabled',savedSettings.api_enabled ?'on' :'off');body.set('homewizard_enabled',savedSettings.homewizard_enabled ?'on' :'off');const r =await fetch('/api/settings',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body});if (!r.ok) throw new Error(await r.text());s.textContent ='Settings saved and applied.'} catch (err) {s.textContent =err.message}});load();</script>
    <script>document.querySelectorAll('.password-toggle').forEach(button =>button.addEventListener('click',() =>{const input =document.getElementById(button.getAttribute('aria-controls'));const visible =input.type==='password';input.type =visible ?'text' :'password';button.setAttribute('aria-pressed',String(visible));button.setAttribute('aria-label',visible ?'Hide password' :'Show password');button.title =visible ?'Hide password' :'Show password'}));const fieldIds ={wifi_ssid:'ssid',wifi_password:'wifi_password',static_ip:'ip',gateway:'gateway',subnet:'subnet',dns:'dns'};function clearFieldError(name) {const input =document.getElementById(fieldIds[name]);const error =document.getElementById(`error-${name}`);if (!input||!error) return;error.textContent ='';input.removeAttribute('aria-invalid');input.removeAttribute('aria-describedby');input.style.borderColor =''} function clearFieldErrors() {Object.keys(fieldIds).forEach(clearFieldError)} function showFieldError(name,message) {const input =document.getElementById(fieldIds[name]);const error =document.getElementById(`error-${name}`);if (!input||!error) return false;error.textContent =message;input.setAttribute('aria-invalid','true');input.setAttribute('aria-describedby',error.id);input.style.borderColor ='#a33b2f';return true} Object.entries(fieldIds).forEach(([name,id]) =>{const input =document.getElementById(id);const error =document.createElement('div');error.id =`error-${name}`;error.className ='field-error';error.setAttribute('role','alert');error.style.color ='#a33b2f';error.style.fontSize ='13px';input.closest('.field').append(error);input.addEventListener('input',() =>clearFieldError(name));input.addEventListener('invalid',event =>{event.preventDefault();document.getElementById('status').textContent ='';showFieldError(name,input.validationMessage)})});function setRequired(id,required) {const input =document.getElementById(id);input.required =required;const marker =document.getElementById(`required-${id}`);marker.hidden =!required} ['ssid','wifi_password'].forEach(id =>setRequired(id,true));const staticMode =() =>document.querySelector('[name=ip_mode]:checked').value==='static';['ip','gateway','subnet'].forEach(id =>setRequired(id,staticMode()));const password =document.getElementById('wifi_password');password.setCustomValidity(!password.value||/^(?:[-~]{8,63}|[0-9a-fA-F]{64})$(?![\s\S])/.test(password.value) ?'' :'Use 8 to 63 printable ASCII characters or 64 hexadecimal digits.');document.querySelectorAll('[name=ip_mode]').forEach(input =>input.addEventListener('change',() =>{['ip','gateway','subnet','dns'].forEach(clearFieldError);['ip','gateway','subnet'].forEach(id =>setRequired(id,staticMode()));updateRequirements();}));document.getElementById('wifi_password').addEventListener('input',() =>{password.setCustomValidity(!password.value||/^(?:[-~]{8,63}|[0-9a-fA-F]{64})$(?![\s\S])/.test(password.value) ?'' :'Use 8 to 63 printable ASCII characters or 64 hexadecimal digits.');});function updateRequirements() {['ssid','wifi_password'].forEach(id =>setRequired(id,true));['ip','gateway','subnet'].forEach(id =>setRequired(id,staticMode()));} const markers =['ssid','wifi_password','ip','gateway','subnet'];markers.forEach(id =>{const marker =document.createElement('span');marker.id =`required-${id}`;marker.textContent =' *';marker.style.color ='#a33b2f';marker.setAttribute('aria-hidden','true');document.querySelector(`label[for="${id}"]`).append(marker)});updateRequirements();</script>
    
</body></html>)HTML";

const char DASHBOARD_PAGE[] PROGMEM = R"HTML(<!doctype html>
<html lang="en">

<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width,initial-scale=1">
    <title>P1 Meter Dashboard</title>
    <style>:root{font-family:system-ui,sans-serif;color:#14231d;background:#f3f6f2}*{box-sizing:border-box}body{margin:0 auto;padding:22px;max-width:980px}a{color:#27734f}h1{font-size:26px;margin:16px 0 6px}h2{font-size:18px;margin:28px 0 10px}.intro,.hint{color:#52645b;line-height:1.5}.topbar{display:flex;justify-content:space-between;align-items:flex-start;gap:16px}.status{display:flex;align-items:center;gap:8px;color:#52645b;font-size:14px}.status-dot{width:10px;height:10px;border-radius:50%;background:#d08a22}.status-dot.live{background:#27734f}.status-dot.error{background:#a33b2f}.cards{display:grid;grid-template-columns:repeat(3,minmax(0,1fr));gap:12px}.card{padding:16px;background:#fff;border:1px solid #d5dfd8;border-radius:8px;min-height:100px}.card h3{font-size:14px;font-weight:600;color:#52645b;margin:0 0 12px}.value{font-size:25px;font-weight:700;letter-spacing:-.02em;overflow-wrap:anywhere}.value.unavailable{font-size:18px;color:#8a9890;font-weight:500}.meta{margin-top:7px;color:#62736a;font-size:13px}.info-grid{display:grid;grid-template-columns:repeat(3,minmax(0,1fr));gap:12px}.info{padding:14px 16px;background:#e9f0ea;border-radius:8px}.info strong{display:block;font-size:13px;color:#52645b;margin-bottom:6px}.info span{font-weight:650;overflow-wrap:anywhere}.actions{display:flex;gap:10px;flex-wrap:wrap;margin-top:24px}button,select{padding:10px 14px;border:1px solid #27734f;border-radius:6px;background:#27734f;color:#fff;font:inherit;font-weight:650;cursor:pointer}select{background:#fff;color:#27734f}button:disabled{opacity:.6;cursor:wait}.notice{display:none;margin-top:14px;padding:12px 14px;border-radius:6px;background:#fff4df;color:#74551f}.notice.visible{display:block}.history-setting{display:flex;align-items:center;gap:12px;padding:14px 16px;background:#fff;border:1px solid #d5dfd8;border-radius:8px}.history-setting input{width:20px;height:20px;accent-color:#27734f}.history-setting label{font-weight:700}.history-setting small{display:block;color:#52645b;font-weight:400;margin-top:3px}.history-controls{display:flex;align-items:center;gap:10px;flex-wrap:wrap}.history-controls label{font-size:14px;color:#52645b;font-weight:650}.charts{display:grid;grid-template-columns:repeat(3,minmax(0,1fr));gap:12px}.chart{padding:14px;background:#fff;border:1px solid #d5dfd8;border-radius:8px}.chart h3{font-size:14px;color:#52645b;margin:0 0 12px}.bars{height:155px;display:flex;align-items:flex-end;gap:2px;border-bottom:1px solid #cbd7ce;padding:0 2px}.bar-wrap{height:100%;flex:1;display:flex;align-items:flex-end;justify-content:center;position:relative}.bar{width:100%;min-height:2px;border-radius:3px 3px 0 0;background:#27734f}.bar.empty{background:#dce5df}.bar-wrap:hover .bar{background:#d08a22}.bar-wrap::after{content:attr(data-hour);position:absolute;bottom:-18px;font-size:9px;color:#718178}.chart-footer{display:flex;justify-content:space-between;margin-top:25px;color:#718178;font-size:12px}.chart-value{font-weight:650;color:#52645b}.history-empty{color:#718178;font-size:13px;margin-top:8px}.modal{display:none;position:fixed;inset:0;background:#14231dcc;align-items:center;justify-content:center;padding:18px;z-index:5}.modal.open{display:flex}.modal-card{width:min(760px,100%);max-height:calc(100vh - 36px);overflow:auto;background:#fff;border-radius:10px;padding:22px;box-shadow:0 12px 40px #0005}.modal-card h2{margin:0 0 10px}.modal-card p{color:#52645b;line-height:1.5}.modal-actions{display:flex;justify-content:flex-end;gap:10px;margin-top:18px}.secondary{background:#fff;color:#27734f}.details-grid{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:12px}.details-group{margin-top:18px}.details-group h3{font-size:15px;color:#27734f;border-bottom:1px solid #d5dfd8;padding-bottom:7px}.details-card{padding:12px;background:#f3f6f2;border-radius:7px}.details-card strong{display:block;font-size:13px;color:#52645b}.details-card span{font-weight:650;display:block;margin-top:5px;overflow-wrap:anywhere}.details-card small{color:#718178}@media(max-width:720px){.cards{grid-template-columns:repeat(2,minmax(0,1fr))}.info-grid,.charts,.details-grid{grid-template-columns:1fr 1fr}}@media(max-width:480px){body{padding:16px}.cards,.info-grid,.charts,.details-grid{grid-template-columns:1fr}.topbar{display:block}}.menu{border-bottom:1px solid #d5dfd8;background:#fff;position:sticky;top:0;z-index:4}.menu-list{display:flex;flex-wrap:wrap;gap:4px;list-style:none;margin:0;padding:0 12px}.menu-item{display:inline-flex;align-items:center;gap:8px;padding:8px 12px;border:1px solid #d5dfd8;border-radius:6px;background:#fff;color:#14231d;text-decoration:none;font-size:14px;font-weight:650;cursor:pointer;white-space:nowrap}.menu-item svg.menu-icon{width:16px;height:16px;flex:none;color:#27734f}.menu-item.menu-toggle{padding-right:6px}.menu-arrow{width:14px;height:14px;flex:none;color:#27734f;margin-left:2px}.menu-item:hover{border-color:#27734f;background:#f8fbf8}.menu-group{position:relative}.submenu{position:absolute;left:0;right:0;top:100%;margin-top:4px;padding:8px;list-style:none;margin:0;background:#fff;border:1px solid #d5dfd8;border-radius:6px;box-shadow:0 8px 24px #0003;z-index:5;display:flex;flex-direction:column;gap:2px;min-width:220px}.submenu[hidden]{display:none}.submenu .menu-item{border:0;background:transparent;padding:8px 10px;text-align:left;font-size:14px;font-weight:500;justify-content:space-between;width:100%}.submenu .menu-item .status-indicator{margin-right:8px}.submenu .menu-item:hover{background:#f3f6f2;border-color:transparent}.status-indicator{display:inline-block;width:8px;height:8px;border-radius:50%;background:#a33b2f;flex:none}.status-indicator.on{background:#27734f}.status-indicator.off{background:#a33b2f}.topbar-bottom .status{margin-left:0}</style>
</head>

<body><a href="/">&larr; Home</a>
    <header class="topbar">
        <div>
            <div class="status">
                <span class="status-dot" id="status-dot"></span>
                <span id="connection-status">Connecting to meter…</span>
            </div>
            <h1>Meter dashboard</h1>
            <p class="intro">Live readings from the P1 port. Empty or unsupported DSMR fields are hidden automatically.</p>
        </div>
        <div class="actions">
            <button class="secondary" id="refresh" type="button">Refresh</button>
            <button class="secondary" id="details-button" type="button">More sensor details</button>
        </div>
    </header>
    <div class="notice" id="notice" role="status"></div>
    <section id="history-graphs" hidden>
        <h2>Hourly usage</h2>
        <div class="history-controls">
            <label for="history-day">Show data for</label>
            <select id="history-day">
                <option value="0">Today</option>
                <option value="1">Yesterday</option>
                <option value="2">Two days ago</option>
            </select>
        </div>
        <p class="hint">Each bar is the energy or gas used during that hour. Bar heights are scaled to the highest value in the selected day.</p>
        <section class="charts">
            <article class="chart">
                <h3>Energy Consumed</h3>
                <div class="bars" id="chart-consumed"></div>
                <div class="chart-footer">
                    <span>00:00</span>
                    <span class="chart-value" id="chart-consumed-max">—</span>
                    <span>23:00</span>
                </div>
            </article>
            <article class="chart">
                <h3>Energy Returned</h3>
                <div class="bars" id="chart-returned"></div>
                <div class="chart-footer">
                    <span>00:00</span>
                    <span class="chart-value" id="chart-returned-max">—</span>
                    <span>23:00</span>
                </div>
            </article>
            <article class="chart">
                <h3>Gas Consumed</h3>
                <div class="bars" id="chart-gas"></div>
                <div class="chart-footer">
                    <span>00:00</span>
                    <span class="chart-value" id="chart-gas-max">—</span>
                    <span>23:00</span>
                </div>
            </article>
        </section>
    </section>
    <div class="modal" id="history-modal" role="dialog" aria-modal="true" aria-labelledby="history-modal-title">
        <div class="modal-card">
            <h2 id="history-modal-title">Confirm On-Device History</h2>
            <p id="history-modal-text"></p>
            <div class="modal-actions">
                <button class="secondary" id="history-cancel" type="button">Cancel</button>
                <button id="history-confirm" type="button">Confirm</button>
            </div>
        </div>
    </div>
    <div class="modal" id="details-modal" role="dialog" aria-modal="true" aria-labelledby="details-title">
        <div class="modal-card">
            <h2 id="details-title">Available sensor details</h2>
            <p>Only sensors with a valid current value are shown. Regional alternatives and diagnostic fields remain available here without filling the main dashboard.</p>
            <div id="details-content"></div>
            <div class="modal-actions">
                <button class="secondary" id="details-close" type="button">Close</button>
            </div>
        </div>
    </div>
    <h2>Power now</h2>
    <section class="cards" id="power-cards"></section>
    <h2>Energy totals</h2>
    <section class="cards" id="energy-cards"></section>
    <h2>Phase readings</h2>
    <section class="cards" id="phase-cards"></section>
    <h2>Meter information</h2>
    <section class="info-grid" id="info-cards"></section>
    <h2>Power quality</h2>
    <section class="cards" id="quality-cards"></section>
    <script>const groups ={power:[['Power Consumed','W'],['Power Returned','W']],energy:[],phase:[],quality:[['Short Power Outages','Counter'],['Long Power Outages','Counter'],['Short Power Drops Phase 1','Counter'],['Short Power Peaks Phase 1','Counter']]};const texts =['P1 Meter ID','P1 Version','Actual Tariff Group'];const values ={};let sensorData =[],textData =[];const $ =id =>document.getElementById(id);const safe =s =>String(s).replace(/[&<>"']/g, c => ({'&': '&amp;',
 '<': '&lt;',
 '>': '&gt;',
 '"': '&quot;',
 "'": '&#39;'} [c]));

function idFor(prefix, name) {return prefix + '-' + String(name).replace(/[^a-z0-9]+/gi, '-').toLowerCase()}

function available(name) {const item = sensorData.find(x => x.name===name);
 return item&&item.has_state&&Number.isFinite(Number(item.value))}

function firstAvailable(names) {return names.find(available)}

function addIfAvailable(list, name, unit, title) {if (available(name)) list.push([name, unit, title||name])}

function unitFor(name) {if (/^Power (Consumed|Returned)/.test(name)) return 'W';
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
 return 'value'}

function displayValue(name, raw) {const n = Number(raw);
 if (!Number.isFinite(n)) return String(raw);
 if (name==='Power Consumed'||name==='Power Returned'||/^Power (Consumed|Returned) Phase/.test(name)) return String((n * 1000).toFixed(1));
 return String(n)}

function card(item) {const name = item[0], unit = item[1], title = item[2]||name;
 return `<article class="card"><h3>${safe(title)}</h3><div class="value">${safe(displayValue(name, itemValue(name)))} </div><div class="meta">${safe(unit||'Counter')}</div></article>`}

function itemValue(name) {const item = sensorData.find(x => x.name===name);
 return item ? item.value : ''}

function info(name, value) {return `<div class="info"><strong>${safe(name)}</strong><span>${safe(value||'Unavailable')}</span></div>`}

function buildMainGroups() {groups.energy = [];
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
 for (let phase = 1; phase<=3; phase++) {[['Power Consumed Phase ' + phase, 'W'],
 ['Power Returned Phase ' + phase, 'W'],
 ['Current Phase ' + phase, 'A'],
 ['Voltage Phase ' + phase, 'V']].forEach(x => addIfAvailable(groups.phase, x[0], x[1]));}}

function renderMain() {buildMainGroups();
 Object.entries(groups).forEach(([group, items]) => $(group + '-cards').innerHTML = items.length ? items.map(card).join('') : '<p class="hint">No current values available.</p>');
 const infoNames = texts.filter(name => {const x = textData.find(i => i.name===name);
 return x&&x.state});
 $('info-cards').innerHTML = infoNames.length ? infoNames.map(name => {const x = textData.find(i => i.name===name);
 return info(name, x.state)}).join('') : '<p class="hint">No meter metadata available.</p>'}

function category(name) {if (/Tariff|Energy/.test(name)) return 'Energy and tariffs';
 if (/Phase|Current|Voltage|Power Factor|Frequency|Power /.test(name)) return 'Electricity and phases';
 if (/Gas|Water|Thermal|Sub-meter/.test(name)) return 'External meters';
 if (/Failure|Outage|Drop|Peak|Sag|Swell/.test(name)) return 'Power quality and diagnostics';
 return 'Other DSMR values'}

function renderDetails() {const rows = [];
 sensorData.filter(x => x.has_state).forEach(x => rows.push({name: x.name,
 value: displayValue(x.name, x.value),
 unit: unitFor(x.name)}));
 textData.filter(x => x.state).forEach(x => rows.push({name: x.name,
 value: x.state,
 unit: 'text'}));
 const grouped = {};
 rows.forEach(x => (grouped[category(x.name)] ??= []).push(x));
 $('details-content').innerHTML = Object.entries(grouped).sort().map(([group, items]) => `<section class="details-group"><h3>${safe(group)}</h3><div class="details-grid">${items.sort((a, b) => a.name.localeCompare(b.name)).map(x => `<div class="details-card"><strong>${safe(x.name)}</strong><span>${safe(x.value)}</span><small>${safe(x.unit)}</small></div>`).join('')}</div></section>`).join('')||'<p class="hint">No sensor currently has a valid value.</p>'}

function renderChart(id, maxId, data, unit) {const el = $(id), maxEl = $(maxId),
 numbers = (data||[]).map(v => typeof v==='number'&&Number.isFinite(v) ? v : null),
 dailyMax = Math.max(0, ...numbers.filter(v => v!=null));
 el.innerHTML = numbers.map((value, hour) => {const height = value==null||dailyMax===0 ? 0 : Math.max(3, value / dailyMax * 100);
 const label = value==null ? 'No data' : value.toFixed(3) + ' ' + unit;
 return `<div class="bar-wrap" data-hour="${String(hour).padStart(2,'0')}" title="${String(hour).padStart(2,'0')}:00 — ${label}"><div class="bar${value==null ?' empty' :''}" style="height:${height}%"></div></div>`}).join('');
 maxEl.textContent = dailyMax ? dailyMax.toFixed(3) + ' ' + unit : 'No data'}

function renderHistory(history) {const enabled = !!(history&&history.enabled);
 $('history-graphs').hidden = !enabled;
 if (!enabled) return;
 renderChart('chart-consumed', 'chart-consumed-max', history.energy_consumed, 'kWh');
 renderChart('chart-returned', 'chart-returned-max', history.energy_returned, 'kWh');
 renderChart('chart-gas', 'chart-gas-max', history.gas_consumed, 'm³');
 $('history-day').value = String(history.day_offset!=null ? history.day_offset : 0)}
async function load() {try {const day = $('history-day').value;
 const response = await fetch('/api/dashboard?day=' + encodeURIComponent(day), {cache: 'no-store'});
 if (!response.ok) throw new Error('Dashboard data unavailable');
 const data = await response.json();
 sensorData = data.sensors||[];
 textData = data.text_sensors||[];
 renderMain();
 renderDetails();
 window.lastHistory = data.history||{};
 renderHistory(data.history);
 const any = sensorData.some(x => x.has_state)||textData.some(x => x.state);
 $('connection-status').textContent = any ? 'Live readings' : 'Waiting for P1 data';
 $('status-dot').className = 'status-dot ' + (any ? 'live' : '');
 $('notice').classList.remove('visible')} catch (error) {$('connection-status').textContent = 'Dashboard data unavailable';
 $('status-dot').className = 'status-dot error';
 $('notice').textContent = 'Could not read meter values. The device may be restarting or the P1 telegram is not available.';
 $('notice').classList.add('visible')}}
$('details-button').addEventListener('click', () => {$('details-modal').classList.add('open')});
$('details-close').addEventListener('click', () => {$('details-modal').classList.remove('open')});
$('details-modal').addEventListener('click', event => {if (event.target===$('details-modal')) $('details-modal').classList.remove('open')});
$('refresh').addEventListener('click', async () => {const button = $('refresh');
 button.disabled = true;
 await load();
 button.disabled = false});
$('history-day').addEventListener('change', load);
load();
setInterval(load, 10000);</script>
    
</body></html>)HTML";

const char FACTORY_RESET_PAGE[] PROGMEM = R"HTML(<!doctype html>
<html lang="en">
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width,initial-scale=1">
    <title>Factory Reset</title>
    <style>:root{font-family:system-ui,sans-serif;color:#14231d;background:#f3f6f2}body{margin:0 auto;padding:22px;max-width:620px}a{color:#27734f}h1{font-size:25px}p{color:#52645b;line-height:1.5}button{padding:11px 15px;border:0;border-radius:6px;background:#a33b2f;color:white;font:inherit;font-weight:650}.status{margin-top:14px}.overlay{position:fixed;inset:0;background:#f3f6f2ee;display:grid;place-items:center}.overlay[hidden]{display:none}.panel{padding:28px;text-align:center;background:white;border-radius:12px;box-shadow:0 8px 30px #14231d22}</style>
</head>
<body><a href="/">&larr; Home</a>
    <h1>Factory Reset</h1>
    <p>This erases saved Wi-Fi, MQTT, and network settings, then restarts the device in setup mode.</p>
    <button id="reset">Factory Reset</button>
    <p class="status" id="status" role="status"></p>
    <div class="overlay" id="restart_overlay" hidden>
        <div class="panel">
            <h2>Device is restarting</h2>
            <p>The meter is temporarily unavailable while it restarts. This page will retry automatically.</p>
            <p id="retry_status">Waiting for the device…</p>
        </div>
    </div>
    <script>document.getElementById('reset').addEventListener('click',async () =>{if (!confirm('Erase all saved settings and restart?')) return;const s =document.getElementById('status');s.textContent ='Resetting…';try {const r =await fetch('/api/factory-reset',{method:'POST'});if (!r.ok) throw new Error(await r.text());document.getElementById('restart_overlay').hidden =false;let n =0;const retry =async () =>{n++;try {const q =await fetch('/',{cache:'no-store'});if (q.ok) {location.reload();return}} catch (e) {} document.getElementById('retry_status').textContent ='Still waiting… retry ' +n;setTimeout(retry,2000)};setTimeout(retry,10000)} catch (e) {s.textContent =e.message}});</script>
     
</body></html>)HTML";

const char INDEX_PAGE[] PROGMEM = R"HTML(<!doctype html>
<html lang="en">

<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width,initial-scale=1">
    <title>P1 Meter</title>
    <style>:root{font-family:system-ui,sans-serif;color:#14231d;background:#f3f6f2}*{box-sizing:border-box}body{margin:0 auto;padding:22px;max-width:980px}a{color:#27734f}h1{font-size:26px;margin:16px 0 6px}h2{font-size:18px;margin:28px 0 10px}.intro,.hint{color:#52645b;line-height:1.5}.topbar{display:flex;justify-content:space-between;align-items:flex-start;gap:16px}.status{display:flex;align-items:center;gap:8px;color:#52645b;font-size:14px}.status-dot{width:10px;height:10px;border-radius:50%;background:#d08a22}.status-dot.live{background:#27734f}.status-dot.error{background:#a33b2f}.cards{display:grid;grid-template-columns:repeat(3,minmax(0,1fr));gap:12px}.card{padding:16px;background:#fff;border:1px solid #d5dfd8;border-radius:8px;min-height:100px}.card h3{font-size:14px;font-weight:600;color:#52645b;margin:0 0 12px}.value{font-size:25px;font-weight:700;letter-spacing:-.02em;overflow-wrap:anywhere}.value.unavailable{font-size:18px;color:#8a9890;font-weight:500}.meta{margin-top:7px;color:#62736a;font-size:13px}.info-grid{display:grid;grid-template-columns:repeat(3,minmax(0,1fr));gap:12px}.info{padding:14px 16px;background:#e9f0ea;border-radius:8px}.info strong{display:block;font-size:13px;color:#52645b;margin-bottom:6px}.info span{font-weight:650;overflow-wrap:anywhere}.actions{display:flex;gap:10px;flex-wrap:wrap;margin-top:24px}button,select{padding:10px 14px;border:1px solid #27734f;border-radius:6px;background:#27734f;color:#fff;font:inherit;font-weight:650;cursor:pointer}select{background:#fff;color:#27734f}button:disabled{opacity:.6;cursor:wait}.notice{display:none;margin-top:14px;padding:12px 14px;border-radius:6px;background:#fff4df;color:#74551f}.notice.visible{display:block}.history-setting{display:flex;align-items:center;gap:12px;padding:14px 16px;background:#fff;border:1px solid #d5dfd8;border-radius:8px}.history-setting input{width:20px;height:20px;accent-color:#27734f}.history-setting label{font-weight:700}.history-setting small{display:block;color:#52645b;font-weight:400;margin-top:3px}.history-controls{display:flex;align-items:center;gap:10px;flex-wrap:wrap}.history-controls label{font-size:14px;color:#52645b;font-weight:650}.charts{display:grid;grid-template-columns:repeat(3,minmax(0,1fr));gap:12px}.chart{padding:14px;background:#fff;border:1px solid #d5dfd8;border-radius:8px}.chart h3{font-size:14px;color:#52645b;margin:0 0 12px}.bars{height:155px;display:flex;align-items:flex-end;gap:2px;border-bottom:1px solid #cbd7ce;padding:0 2px}.bar-wrap{height:100%;flex:1;display:flex;align-items:flex-end;justify-content:center;position:relative}.bar{width:100%;min-height:2px;border-radius:3px 3px 0 0;background:#27734f}.bar.empty{background:#dce5df}.bar-wrap:hover .bar{background:#d08a22}.bar-wrap::after{content:attr(data-hour);position:absolute;bottom:-18px;font-size:9px;color:#718178}.chart-footer{display:flex;justify-content:space-between;margin-top:25px;color:#718178;font-size:12px}.chart-value{font-weight:650;color:#52645b}.history-empty{color:#718178;font-size:13px;margin-top:8px}.modal{display:none;position:fixed;inset:0;background:#14231dcc;align-items:center;justify-content:center;padding:18px;z-index:5}.modal.open{display:flex}.modal-card{width:min(760px,100%);max-height:calc(100vh - 36px);overflow:auto;background:#fff;border-radius:10px;padding:22px;box-shadow:0 12px 40px #0005}.modal-card h2{margin:0 0 10px}.modal-card p{color:#52645b;line-height:1.5}.modal-actions{display:flex;justify-content:flex-end;gap:10px;margin-top:18px}.secondary{background:#fff;color:#27734f}.details-grid{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:12px}.details-group{margin-top:18px}.details-group h3{font-size:15px;color:#27734f;border-bottom:1px solid #d5dfd8;padding-bottom:7px}.details-card{padding:12px;background:#f3f6f2;border-radius:7px}.details-card strong{display:block;font-size:13px;color:#52645b}.details-card span{font-weight:650;display:block;margin-top:5px;overflow-wrap:anywhere}.details-card small{color:#718178}@media(max-width:720px){.cards{grid-template-columns:repeat(2,minmax(0,1fr))}.info-grid,.charts,.details-grid{grid-template-columns:1fr 1fr}}@media(max-width:480px){body{padding:16px}.cards,.info-grid,.charts,.details-grid{grid-template-columns:1fr}.topbar{display:block}}.menu{border-bottom:1px solid #d5dfd8;background:#fff;position:sticky;top:0;z-index:4}.menu-list{display:flex;flex-wrap:wrap;gap:4px;list-style:none;margin:0;padding:0 12px}.menu-item{display:inline-flex;align-items:center;gap:8px;padding:8px 12px;border:1px solid #d5dfd8;border-radius:6px;background:#fff;color:#14231d;text-decoration:none;font-size:14px;font-weight:650;cursor:pointer;white-space:nowrap}.menu-item svg.menu-icon{width:16px;height:16px;flex:none;color:#27734f}.menu-item.menu-toggle{padding-right:6px}.menu-arrow{width:14px;height:14px;flex:none;color:#27734f;margin-left:2px}.menu-item:hover{border-color:#27734f;background:#f8fbf8}.menu-group{position:relative}.submenu{position:absolute;left:0;right:0;top:100%;margin-top:4px;padding:8px;list-style:none;margin:0;background:#fff;border:1px solid #d5dfd8;border-radius:6px;box-shadow:0 8px 24px #0003;z-index:5;display:flex;flex-direction:column;gap:2px;min-width:220px}.submenu[hidden]{display:none}.submenu .menu-item{border:0;background:transparent;padding:8px 10px;text-align:left;font-size:14px;font-weight:500;justify-content:space-between;width:100%}.submenu .menu-item .status-indicator{margin-right:8px}.submenu .menu-item:hover{background:#f3f6f2;border-color:transparent}.status-indicator{display:inline-block;width:8px;height:8px;border-radius:50%;background:#a33b2f;flex:none}.status-indicator.on{background:#27734f}.status-indicator.off{background:#a33b2f}.topbar-bottom .status{margin-left:0}</style>
</head>

<body>
    <header class="topbar">
        <div>
            <div class="eyebrow">ESPHome · P1 Meter</div>
            <h1>Meter dashboard</h1>
        </div>
        <div class="actions">
            <button class="secondary" id="refresh" type="button">Refresh</button>
            <!-- <button class="secondary" id="details-button" type="button">More sensor details</button> -->
            <button class="logout" id="logout" type="button" aria-label="Sign out" title="Sign out">
            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">
                <path d="M9 21H5a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h4" />
                <polyline points="16 17 21 12 16 7" />
                <line x1="21" y1="12" x2="9" y2="12" />
            </svg>
            </button>
        </div>
    </header>

    <nav class="menu" id="main-menu">
        <ul class="menu-list">
            <li>
                <button class="menu-item" id="menu-configure-wifi" type="button">
                    <svg class="menu-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">
                        <circle cx="12" cy="12" r="3" />
                        <path d="M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 0 1 0 2.83 2 2 0 0 1-2.83 0l-.06-.06a1.65 1.65 0 0 0-1.82-.33 1.65 1.65 0 0 0-1 1.51V21a2 2 0 0 1-2 2 2 2 0 0 1-2-2v-.09A1.65 1.65 0 0 0 9 14a1.65 1.65 0 0 0-1.82-.33l-.06.06a2 2 0 0 1-2.83 0 2 2 0 0 1 0-2.83l.06-.06a1.65 1.65 0 0 0 .33-1.82 1.65 1.65 0 0 0-1.51-1H3a2 2 0 0 1-2-2 2 2 0 0 1 2-2h0.09A1.65 1.65 0 0 0 3 10a1.65 1.65 0 0 0 1.82.33l.06-.06a2 2 0 0 1 2.83 0 2 2 0 0 1 0 2.83l-.06.06a1.65 1.65 0 0 0 1.82.33h.09A1.65 1.65 0 0 0 10 10a1.65 1.65 0 0 0 1-1.51l-.06-.06a2 2 0 0 1 2.83 0 2 2 0 0 1 0 2.83l.06.06a1.65 1.65 0 0 0 1.82-.33 1.65 1.65 0 0 0 1-1.51V3a2 2 0 0 1 2-2 2 2 0 0 1 2 2v0.09A1.65 1.65 0 0 0 21 10a1.65 1.65 0 0 0-1.51 1H21a2 2 0 0 1 2 2 2 2 0 0 1-2 2h-0.09A1.65 1.65 0 0 0 19 14a1.65 1.65 0 0 0-1.51-1v.09A1.65 1.65 0 0 0 19.4 15Z" />
                    </svg>
                    <span>Configure</span>
                </button>
            </li>
            <li>
                <a class="menu-item" id="menu-ota" href="#">
                    <svg class="menu-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">
                        <path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4" />
                        <polyline points="17 8 12 3 7 8" />
                        <line x1="12" y1="3" x2="12" y2="15" />
                    </svg>
                    <span>OTA Update</span>
                </a>
            </li>
            <li>
                <a class="menu-item" id="menu-reboot" type="button">
                    <svg class="menu-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">
                        <polyline points="1 4 1 10 7 10" />
                        <polyline points="23 20 23 14 17 14" />
                        <path d="M20.49 9A9 9 0 0 0 5.64 5.64L1 10m22 4l-4.64 4.36A9 9 0 0 1 3.51 15" />
                    </svg>
                    <span>Reboot</span>
                </a>
            </li>
            <li>
                <a class="menu-item" id="menu-factory-reset" type="button">
                    <svg class="menu-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">
                        <path d="M19 21H5a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h4l2-4h6a2 2 0 0 1 2 2v14a2 2 0 0 1-2 2Z" />
                        <line x1="10" y1="11" x2="10" y2="17" />
                        <line x1="14" y1="11" x2="14" y2="17" />
                    </svg>
                    <span>Factory Reset</span>
                </a>
            </li>
            <li class="menu-group">
                <button class="menu-item menu-toggle" id="menu-communication" type="button" aria-expanded="false">
                    <svg class="menu-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">
                        <path d="M1 21h22L12 1 1 21" />
                        <polyline points="22 12 18 12 15 21 9 21 6 12 2 12" />
                    </svg>
                    <span>Communication</span>
                    <svg class="menu-arrow" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">
                        <polyline points="6 9 12 15 18 9" />
                    </svg>
                </button>
                <ul class="submenu" id="submenu-communication">
                    <li>
                        <button class="menu-item" id="menu-api" type="button">
                            <span class="status-indicator" id="api-indicator"></span>
                            <span id="api-label">Enable ESPHome API</span>
                        </button>
                    </li>
                    <li>
                        <button class="menu-item" id="menu-mqtt" type="button">
                            <span class="status-indicator" id="mqtt-indicator"></span>
                            <span id="mqtt-label">Enable MQTT</span>
                        </button>
                    </li>
                    <li>
                        <button class="menu-item" id="menu-homewizard" type="button">
                            <span class="status-indicator" id="homewizard-indicator"></span>
                            <span id="homewizard-label">Enable HomeWizzard</span>
                        </button>
                    </li>
                </ul>
            </li>
            <li>
                <a class="menu-item" id="menu-password" type="button">
                    <svg class="menu-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">
                        <path d="M21 8a8 8 0 0 0-8-8H5a8 8 0 0 0-8 8v20a8 8 0 0 0 8 8h8a8 8 0 0 0 8-8v-2" />
                        <line x1="17" y1="11" x2="17" y2="13" />
                        <line x1="12" y1="6.5" x2="17" y2="11.5" />
                    </svg>
                    <span>Change Password</span>
                </a>
            </li>
            <li>
                <button class="menu-item" id="menu-history" type="button">
                    <svg class="menu-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">
                        <path d="M13 2V6H21" />
                        <path d="M3 22V4L12 13l9-9v18" />
                    </svg>
                    <span id="history-label">Enable History</span>
                </button>
            </li>
            <li>
                <a class="menu-item" id="details-button" type="button">
                    <svg class="menu-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">
                        <path d="M19 21H5a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h4l2-4h6a2 2 0 0 1 2 2v14a2 2 0 0 1-2 2Z" />
                        <line x1="10" y1="11" x2="10" y2="17" />
                        <line x1="14" y1="11" x2="14" y2="17" />
                    </svg>
                    <span>Factory Reset</span>
                </a>
            </li>
        </ul>
    </nav>

    <div class="notice" id="notice" role="status"></div>

    <div class="topbar-bottom">
        <div class="status">
            <span class="status-dot" id="status-dot"></span>
            <span id="connection-status">Connecting to meter…</span>
        </div>
    </div>

    <h2>Power now</h2>
    <section class="cards" id="power-cards">

    </section>
    <h2>Energy totals</h2>
    <section class="cards" id="energy-cards"></section>
    <h2>Phase readings</h2>
    <section class="cards" id="phase-cards"></section>
    <h2>Meter information</h2>
    <section class="info-grid" id="info-cards"></section>
    <h2>Power quality</h2>
    <section class="cards" id="quality-cards"></section>

    <section id="history-graphs" hidden>
        <h2>Hourly usage</h2>
        <div class="history-controls">
            <label for="history-day">Show data for</label>
            <select id="history-day">
                <option value="0">Today</option>
                <option value="1">Yesterday</option>
                <option value="2">Two days ago</option>
            </select>
        </div>
        <p class="hint">Each bar is the energy or gas used during that hour. Bar heights are scaled to the highest value in the selected day.</p>
        <section class="charts">
            <article class="chart">
                <h3>Energy Consumed</h3>
                <div class="bars" id="chart-consumed"></div>
                <div class="chart-footer">
                    <span>00:00</span>
                    <span class="chart-value" id="chart-consumed-max">—</span>
                    <span>23:00</span>
                </div>
            </article>
            <article class="chart">
                <h3>Energy Returned</h3>
                <div class="bars" id="chart-returned"></div>
                <div class="chart-footer">
                    <span>00:00</span>
                    <span class="chart-value" id="chart-returned-max">—</span>
                    <span>23:00</span>
                </div>
            </article>
            <article class="chart">
                <h3>Gas Consumed</h3>
                <div class="bars" id="chart-gas"></div>
                <div class="chart-footer">
                    <span>00:00</span>
                    <span class="chart-value" id="chart-gas-max">—</span>
                    <span>23:00</span>
                </div>
            </article>
        </section>
    </section>

    <div class="modal" id="reboot-modal" role="dialog" aria-modal="true" aria-labelledby="reboot-modal-title">
        <div class="modal-card">
            <h2 id="reboot-modal-title">Reboot device</h2>
            <p>Restart the meter now.</p>
            <button id="modal-reboot" type="button">Reboot</button>
            <p class="status" id="modal-reboot-status" role="status"></p>
            <div class="overlay" id="modal-reboot-overlay" hidden>
                <div class="panel">
                    <div class="spinner" aria-hidden="true"></div>
                    <h2>Device is restarting</h2>
                    <p>The meter is temporarily unavailable while it restarts. This page will refresh automatically in about 10 seconds.</p>
                    <p id="modal-reboot-retry">Waiting for the device…</p>
                </div>
            </div>
        </div>
    </div>

    <div class="modal" id="factory-reset-modal" role="dialog" aria-modal="true" aria-labelledby="factory-modal-title">
        <div class="modal-card">
            <h2 id="factory-modal-title">Factory Reset</h2>
            <p>This erases saved Wi-Fi, MQTT, and network settings, then restarts the device in setup mode.</p>
            <button id="modal-factory-reset" type="button">Factory Reset</button>
            <p class="status" id="modal-factory-reset-status" role="status"></p>
            <div class="overlay" id="modal-factory-reset-overlay" hidden>
                <div class="panel">
                    <h2>Device is restarting</h2>
                    <p>The meter is temporarily unavailable while it restarts. This page will retry automatically.</p>
                    <p id="modal-factory-reset-retry">Waiting for the device…</p>
                </div>
            </div>
        </div>
    </div>

    <div class="modal" id="password-modal" role="dialog" aria-modal="true" aria-labelledby="password-modal-title">
        <div class="modal-card">
            <h2 id="password-modal-title">Setup password</h2>
            <p>This password protects the P1-Meter-Setup Wi-Fi network and this web interface. Use 8 to 63 printable ASCII characters. Saving restarts the device; reconnect to P1-Meter-Setup with the new password. Web sign-in username: p1meter.</p>
            <form id="modal-password">
                <div class="field">
                    <label for="modal-password-input">New setup password</label>
                    <div class="password-field">
                        <input id="modal-password-input" name="password" type="password" minlength="8" maxlength="63" required autocomplete="new-password">
                        <button class="password-toggle" type="button" aria-controls="modal-password-input" aria-label="Show password" aria-pressed="false" title="Show password">
                            <svg class="eye-on" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">
                                <path d="M2 12s3.5-7 10-7 10 7 10 7-3.5 7-10 7S2 12 2 12Z" />
                                <circle cx="12" cy="12" r="3" />
                            </svg>
                            <svg class="eye-off" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">
                                <path d="M2 12s3.5-7 10-7 10 7 10 7-3.5 7-10 7S2 12 2 12Z" />
                                <path d="M4 4l16 16" />
                            </svg>
                        </button>
                    </div>
                </div>
                <div class="field">
                    <label for="modal-confirm">Confirm password</label>
                    <div class="password-field">
                        <input id="modal-confirm" type="password" minlength="8" maxlength="63" required autocomplete="new-password">
                        <button class="password-toggle" type="button" aria-controls="modal-confirm" aria-label="Show password" aria-pressed="false" title="Show password">
                            <svg class="eye-on" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">
                                <path d="M2 12s3.5-7 10-7 10 7 10 7-3.5 7-10 7S2 12 2 12Z" />
                                <circle cx="12" cy="12" r="3" />
                            </svg>
                            <svg class="eye-off" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">
                                <path d="M2 12s3.5-7 10-7 10 7 10 7-3.5 7-10 7S2 12 2 12Z" />
                                <path d="M4 4l16 16" />
                            </svg>
                        </button>
                    </div>
                </div>
                <p class="status" id="modal-password-status" role="status"></p>
                <div class="modal-actions">
                    <button class="secondary" id="modal-password-cancel" type="button">Cancel</button>
                    <button id="modal-password-save" type="button">Save</button>
                </div>
                <div class="overlay" id="modal-password-overlay" hidden>
                    <div class="panel">
                        <div class="spinner" aria-hidden="true"></div>
                        <h2>Device is restarting</h2>
                        <p>The meter is temporarily unavailable while it restarts. This page will refresh automatically in about 10 seconds.</p>
                        <p id="modal-password-retry">Waiting for the device…</p>
                    </div>
                </div>
        </div>
    </div>

    <div class="modal" id="configure-mqtt-modal" role="dialog" aria-modal="true" aria-labelledby="mqtt-modal-title">
        <div class="modal-card">
            <h2 id="mqtt-modal-title">MQTT Settings</h2>
            <p>Configure the MQTT broker and enable or disable MQTT.</p>
            <form id="mqtt-form">
                <div class="field">
                    <label for="mqtt-broker">Broker host</label>
                    <input id="mqtt-broker" name="mqtt_broker" maxlength="64">
                </div>
                <div class="field">
                    <label for="mqtt-port">Broker port</label>
                    <input id="mqtt-port" name="mqtt_port" type="number" min="1" max="65535" value="1883">
                </div>
                <div class="field">
                    <label for="mqtt-username">Username</label>
                    <input id="mqtt-username" name="mqtt_username" maxlength="32" autocomplete="username">
                </div>
                <div class="field">
                    <label for="mqtt-password">Password</label>
                    <div class="password-field">
                        <input id="mqtt-password" name="mqtt_password" type="password" maxlength="64" autocomplete="new-password">
                        <button class="password-toggle" type="button" aria-controls="mqtt-password" aria-label="Show password" aria-pressed="false" title="Show password">
                            <svg class="eye-on" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">
                                <path d="M2 12s3.5-7 10-7 10 7 10 7-3.5 7-10 7S2 12 2 12Z" />
                                <circle cx="12" cy="12" r="3" />
                            </svg>
                            <svg class="eye-off" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">
                                <path d="M2 12s3.5-7 10-7 10 7 10 7-3.5 7-10 7S2 12 2 12Z" />
                                <path d="M4 4l16 16" />
                            </svg>
                        </button>
                    </div>
                </div>
                <div class="field">
                    <label>
                        <input id="mqtt-enabled" type="checkbox" style="width:auto;padding:0"> Enable MQTT
                    </label>
                    </div>
                <p class="status" id="mqtt-status" role="status"></p>
                <div class="modal-actions">
                    <button class="secondary" id="mqtt-cancel" type="button">Cancel</button>
                    <button id="mqtt-save" type="button">Save</button>
                </div>
            </form>
        </div>
    </div>

    <div class="modal" id="ota-modal" role="dialog" aria-modal="true" aria-labelledby="ota-modal-title">
        <div class="modal-card">
            <h2 id="ota-modal-title">OTA Update</h2>
            <iframe id="ota-iframe" src="about:blank" title="OTA Update" style="width:100%;height:520px;border:none;border-radius:8px;background:#fff;"></iframe>
            <div class="modal-actions">
                <button class="secondary" id="ota-modal-close" type="button">Close</button>
            </div>
        </div>
    </div>

    <div class="modal" id="configure-modal" role="dialog" aria-modal="true" aria-labelledby="configure-modal-title">
        <div class="modal-card">
            <h2 id="configure-modal-title">Configure</h2>
            <p class="hint">Configure network, Wi-Fi, and IP address settings.</p>
            <iframe id="configure-iframe" src="" style="width:100%;height:520px;border:none;border-radius:8px;background:#fff;"></iframe>
            <div class="modal-actions">
                <button class="secondary" id="configure-modal-close" type="button">Close</button>
            </div>
        </div>
    </div>

    <div class="modal" id="history-modal" role="dialog" aria-modal="true" aria-labelledby="history-modal-title">
        <div class="modal-card">
            <h2 id="history-modal-title">Confirm On-Device History</h2>
            <p id="history-modal-text"></p>
            <div class="modal-actions">
                <button class="secondary" id="history-cancel" type="button">Cancel</button>
                <button id="history-confirm" type="button">Confirm</button>
            </div>
        </div>
    </div>

    <div class="modal" id="details-modal" role="dialog" aria-modal="true" aria-labelledby="details-title">
        <div class="modal-card">
            <h2 id="details-title">Available sensor details</h2>
            <p>Only sensors with a valid current value are shown. Regional alternatives and diagnostic fields remain available here without filling the main dashboard.</p>
            <div id="details-content"></div>
            <div class="modal-actions">
                <button class="secondary" id="details-close" type="button">Close</button>
            </div>
        </div>
    </div>

    <script>const groups ={power:[['Power Consumed','W'],['Power Returned','W']],energy:[],phase:[],quality:[['Short Power Outages','Counter'],['Long Power Outages','Counter'],['Short Power Drops Phase 1','Counter'],['Short Power Peaks Phase 1','Counter']]};const texts =['P1 Meter ID','P1 Version','Actual Tariff Group'];const values ={};let sensorData =[],textData =[];const $ =id =>document.getElementById(id);const safe =s =>String(s).replace(/[&<>"']/g, c => ({'&': '&amp;',
 '<': '&lt;',
 '>': '&gt;',
 '"': '&quot;',
 "'": '&#39;'} [c]));

function idFor(prefix, name) {return prefix + '-' + String(name).replace(/[^a-z0-9]+/gi, '-').toLowerCase()}

function available(name) {const item = sensorData.find(x => x.name===name);
 return item&&item.has_state&&Number.isFinite(Number(item.value))}

function firstAvailable(names) {return names.find(available)}

function addIfAvailable(list, name, unit, title) {if (available(name)) list.push([name, unit, title||name])}

function unitFor(name) {if (/^Power (Consumed|Returned)/.test(name)) return 'W';
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
 return 'value'}

function displayValue(name, raw) {const n = Number(raw);
 if (!Number.isFinite(n)) return String(raw);
 if (name==='Power Consumed'||name==='Power Returned'||/^Power (Consumed|Returned) Phase/.test(name)) return String((n * 1000).toFixed(1));
 return String(n)}

function card(item) {const name = item[0], unit = item[1], title = item[2]||name;
 return `<article class="card"><h3>${safe(title)}</h3><div class="value">${safe(displayValue(name, itemValue(name)))} </div><div class="meta">${safe(unit||'Counter')}</div></article>`}

function itemValue(name) {const item = sensorData.find(x => x.name===name);
 return item ? item.value : ''}

function info(name, value) {return `<div class="info"><strong>${safe(name)}</strong><span>${safe(value||'Unavailable')}</span></div>`}

function buildMainGroups() {groups.energy = [];
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
 for (let phase = 1; phase<=3; phase++) {[['Power Consumed Phase ' + phase, 'W'],
 ['Power Returned Phase ' + phase, 'W'],
 ['Current Phase ' + phase, 'A'],
 ['Voltage Phase ' + phase, 'V']].forEach(x => addIfAvailable(groups.phase, x[0], x[1]));}}

function renderMain() {buildMainGroups();
 Object.entries(groups).forEach(([group, items]) => $(group + '-cards').innerHTML = items.length ? items.map(card).join('') : '<p class="hint">No current values available.</p>');
 const infoNames = texts.filter(name => {const x = textData.find(i => i.name===name);
 return x&&x.state});
 $('info-cards').innerHTML = infoNames.length ? infoNames.map(name => {const x = textData.find(i => i.name===name);
 return info(name, x.state)}).join('') : '<p class="hint">No meter metadata available.</p>'}

function category(name) {if (/Tariff|Energy/.test(name)) return 'Energy and tariffs';
 if (/Phase|Current|Voltage|Power Factor|Frequency|Power /.test(name)) return 'Electricity and phases';
 if (/Gas|Water|Thermal|Sub-meter/.test(name)) return 'External meters';
 if (/Failure|Outage|Drop|Peak|Sag|Swell/.test(name)) return 'Power quality and diagnostics';
 return 'Other DSMR values'}

function renderDetails() {const rows = [];
 sensorData.filter(x => x.has_state).forEach(x => rows.push({name: x.name,
 value: displayValue(x.name, x.value),
 unit: unitFor(x.name)}));
 textData.filter(x => x.state).forEach(x => rows.push({name: x.name,
 value: x.state,
 unit: 'text'}));
 const grouped = {};
 rows.forEach(x => (grouped[category(x.name)] ??= []).push(x));
 $('details-content').innerHTML = Object.entries(grouped).sort().map(([group, items]) => `<section class="details-group"><h3>${safe(group)}</h3><div class="details-grid">${items.sort((a, b) => a.name.localeCompare(b.name)).map(x => `<div class="details-card"><strong>${safe(x.name)}</strong><span>${safe(x.value)}</span><small>${safe(x.unit)}</small></div>`).join('')}</div></section>`).join('')||'<p class="hint">No sensor currently has a valid value.</p>'}

function renderChart(id, maxId, data, unit) {const el = $(id), maxEl = $(maxId),
 numbers = (data||[]).map(v => typeof v==='number'&&Number.isFinite(v) ? v : null),
 dailyMax = Math.max(0, ...numbers.filter(v => v!=null));
 el.innerHTML = numbers.map((value, hour) => {const height = value==null||dailyMax===0 ? 0 : Math.max(3, value / dailyMax * 100);
 const label = value==null ? 'No data' : value.toFixed(3) + ' ' + unit;
 return `<div class="bar-wrap" data-hour="${String(hour).padStart(2,'0')}" title="${String(hour).padStart(2,'0')}:00 — ${label}"><div class="bar${value==null ?' empty' :''}" style="height:${height}%"></div></div>`}).join('');
 maxEl.textContent = dailyMax ? dailyMax.toFixed(3) + ' ' + unit : 'No data'}

function renderHistory(history) {const enabled = !!(history&&history.enabled);
 $('history-graphs').hidden = !enabled;
 if (!enabled) return;
 renderChart('chart-consumed', 'chart-consumed-max', history.energy_consumed, 'kWh');
 renderChart('chart-returned', 'chart-returned-max', history.energy_returned, 'kWh');
 renderChart('chart-gas', 'chart-gas-max', history.gas_consumed, 'm³');
 $('history-day').value = String(history.day_offset!=null ? history.day_offset : 0)}
async function load() {try {const day = $('history-day').value;
 const response = await fetch('/api/dashboard?day=' + encodeURIComponent(day), {cache: 'no-store'});
 if (!response.ok) throw new Error('Dashboard data unavailable');
 const data = await response.json();
 sensorData = data.sensors||[];
 textData = data.text_sensors||[];
 renderMain();
 renderDetails();
 window.lastHistory = data.history||{};
 renderHistory(data.history);
 const any = sensorData.some(x => x.has_state)||textData.some(x => x.state);
 $('connection-status').textContent = any ? 'Live readings' : 'Waiting for P1 data';
 $('status-dot').className = 'status-dot ' + (any ? 'live' : '');
 $('notice').classList.remove('visible')} catch (error) {$('connection-status').textContent = 'Dashboard data unavailable';
 $('status-dot').className = 'status-dot error';
 $('notice').textContent = 'Could not read meter values. The device may be restarting or the P1 telegram is not available.';
 $('notice').classList.add('visible')}}
$('details-button').addEventListener('click', () => {$('details-modal').classList.add('open')});
$('details-close').addEventListener('click', () => {$('details-modal').classList.remove('open')});
$('details-modal').addEventListener('click', event => {if (event.target===$('details-modal')) $('details-modal').classList.remove('open')});
$('refresh').addEventListener('click', async () => {const button = $('refresh');
 button.disabled = true;
 await load();
 button.disabled = false});
$('history-day').addEventListener('change', load);
load();
setInterval(load, 10000);</script>
    <script>const $ =id =>document.getElementById(id);let settings ={};let historyEnabled =false;let pendingApi =null;let pendingHomewizard =null;function isEnabled(value) {return value===true||value==='true'||value===1||value==='1';} function indicator(el,on) {el.classList.toggle('on',on);el.classList.toggle('off',!on);} async function loadSettings() {try {const r =await fetch('/api/settings',{cache:'no-store'});if (!r.ok) throw new Error('Could not load settings');settings =await r.json();} catch (e) {console.warn(e);} try {const d =await fetch('/api/dashboard',{cache:'no-store'});const j =await d.json();historyEnabled =!!(j.history&&j.history.enabled);} catch (e) {} updateIndicators();updateHistoryLabel();} function updateIndicators() {const apiOn =isEnabled(settings.api_enabled);const mqttOn =isEnabled(settings.mqtt_enabled);const homewizardOn =isEnabled(settings.homewizard_enabled);indicator($('api-indicator'),apiOn);indicator($('mqtt-indicator'),mqttOn);indicator($('homewizard-indicator'),homewizardOn);$('api-label').textContent =apiOn ?'Disable ESPHome API' :'Enable ESPHome API';$('mqtt-label').textContent =mqttOn ?'Disable MQTT' :'Enable MQTT';$('homewizard-label').textContent =homewizardOn ?'Disable HomeWizzard' :'Enable HomeWizzard';} function updateHistoryLabel() {$('history-label').textContent =historyEnabled ?'Disable History' :'Enable History';} async function postSettings(extra) {const body =new URLSearchParams({wifi_ssid:settings.wifi_ssid||'',wifi_password:settings.wifi_password||'',mqtt_broker:settings.mqtt_broker||'',mqtt_port:String(settings.mqtt_port||1883),mqtt_username:settings.mqtt_username||'',mqtt_password:settings.mqtt_password||'',ip_mode:settings.ip_mode||'dhcp',static_ip:settings.static_ip||'0.0.0.0',gateway:settings.gateway||'0.0.0.0',subnet:settings.subnet||'255.255.255.0',dns:settings.dns||'0.0.0.0',mqtt_enabled:settings.mqtt_enabled ?'on' :'off',homewizard_enabled:settings.homewizard_enabled ?'on' :'off',api_enabled:settings.api_enabled ?'on' :'off',...extra});const r =await fetch('/api/settings',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body});if (!r.ok) throw new Error(await r.text());await loadSettings();} function openModal(id) {$(id).classList.add('open');} function closeModal(id) {$(id).classList.remove('open');} document.querySelectorAll('.modal').forEach(modal =>{modal.addEventListener('click',ev =>{if (ev.target===modal) closeModal(modal.id);});});const commToggle =$('menu-communication');const submenu =$('submenu-communication');commToggle.addEventListener('click',ev =>{if (submenu.contains(ev.target)) return;const expanded =commToggle.getAttribute('aria-expanded')==='true';commToggle.setAttribute('aria-expanded',String(!expanded));submenu.hidden =expanded;});$('menu-configure-wifi').addEventListener('click',() =>{$('configure-iframe').src ='/configure';openModal('configure-modal');});$('configure-modal-close').addEventListener('click',() =>{closeModal('configure-modal');$('configure-iframe').src ='about:blank';});$('menu-reboot').addEventListener('click',() =>openModal('reboot-modal'));$('menu-ota').addEventListener('click',() =>{$('ota-iframe').src ='/ota';openModal('ota-modal');});$('ota-modal-close').addEventListener('click',() =>{closeModal('ota-modal');$('ota-iframe').src ='about:blank';});$('modal-reboot').addEventListener('click',async () =>{if (!confirm('Reboot the P1 meter now?')) return;try {await fetch('/api/reboot',{method:'POST'});const overlay =$('modal-reboot-overlay');overlay.hidden =false;const retry =async () =>{try {const q =await fetch('/',{cache:'no-store'});if (q.ok) {location.reload();return}} catch (e) {} setTimeout(retry,2000);};setTimeout(retry,10000);} catch (e) {$('modal-reboot-status').textContent =e.message;}});$('menu-factory-reset').addEventListener('click',() =>openModal('factory-reset-modal'));$('modal-factory-reset').addEventListener('click',async () =>{if (!confirm('Erase all saved settings and restart?')) return;try {await fetch('/api/factory-reset',{method:'POST'});const overlay =$('modal-factory-reset-overlay');overlay.hidden =false;const retry =async () =>{try {const q =await fetch('/',{cache:'no-store'});if (q.ok) {location.reload();return}} catch (e) {} setTimeout(retry,2000);};setTimeout(retry,10000);} catch (e) {$('modal-factory-reset-status').textContent =e.message;}});$('menu-password').addEventListener('click',() =>openModal('password-modal'));$('modal-password-cancel').addEventListener('click',() =>closeModal('password-modal'));$('modal-password-save').addEventListener('click',async () =>{const p =$('modal-password-input').value;const c =$('modal-confirm').value;const s =$('modal-password-status');s.className ='status';if (p!==c) {s.className ='status error';s.textContent ='The passwords do not match.';return} if (!/^[\x20-\x7e]{8,63}$/.test(p)) {s.className ='status error';s.textContent ='Use 8 to 63 printable ASCII characters.';return} s.textContent ='Saving…';try {await fetch('/api/setup-password',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:new URLSearchParams({password:p})});const overlay =$('modal-password-overlay');overlay.hidden =false;const retry =async () =>{try {const q =await fetch('/',{cache:'no-store'});if (q.ok) {location.reload();return}} catch (e) {} setTimeout(retry,2000);};setTimeout(retry,10000);} catch (e) {s.className ='status error';s.textContent =e.message;}});$('menu-api').addEventListener('click',async () =>{pendingApi =!isEnabled(settings.api_enabled);try {await postSettings({api_enabled:pendingApi ?'on' :'off'});} catch (e) {alert('Could not change ESPHome Native API: ' +e.message);}});$('menu-homewizard').addEventListener('click',async () =>{pendingHomewizard =!isEnabled(settings.homewizard_enabled);try {await postSettings({homewizard_enabled:pendingHomewizard ?'on' :'off'});} catch (e) {alert('Could not change HomeWizzard: ' +e.message);}});let mqttModalOpen =false;$('menu-mqtt').addEventListener('click',() =>{$('mqtt-broker').value =settings.mqtt_broker||'';$('mqtt-port').value =settings.mqtt_port||1883;$('mqtt-username').value =settings.mqtt_username||'';$('mqtt-password').value =settings.mqtt_password||'';$('mqtt-enabled').checked =isEnabled(settings.mqtt_enabled);$('mqtt-status').textContent ='';openModal('configure-mqtt-modal');mqttModalOpen =true;});$('mqtt-cancel').addEventListener('click',() =>{closeModal('configure-mqtt-modal');mqttModalOpen =false;});$('mqtt-save').addEventListener('click',async () =>{const s =$('mqtt-status');const enabled =$('mqtt-enabled').checked;const broker =$('mqtt-broker').value.trim();if (enabled&&!broker) {s.textContent ='Enter an MQTT broker host when enabling MQTT.';return} try {await postSettings({mqtt_enabled:enabled ?'on' :'off',mqtt_broker:broker,mqtt_port:$('mqtt-port').value||1883,mqtt_username:$('mqtt-username').value,mqtt_password:$('mqtt-password').value});closeModal('configure-mqtt-modal');mqttModalOpen =false;} catch (e) {s.textContent =e.message;}});$('menu-history').addEventListener('click',() =>{$('history-modal-title').textContent =historyEnabled ?'Disable On-Device History?' :'Enable On-Device History?';$('history-modal-text').textContent =historyEnabled ?'This increases the device lifespan by stopping history writes. All already saved on-device history data will be deleted permanently. Confirm only if you no longer need the graphs.' :'This writes history to flash every 30 minutes using alternating files. This can reduce the device flash lifespan over time.';openModal('history-modal');});$('history-cancel').addEventListener('click',() =>closeModal('history-modal'));$('history-confirm').addEventListener('click',async () =>{try {const response =await fetch('/api/history-settings',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'enabled=' +!historyEnabled});if (!response.ok) throw new Error(await response.text());closeModal('history-modal');await loadSettings();} catch (e) {alert('Could not change history setting: ' +e.message);}});$('logout').addEventListener('click',async () =>{const b =$('logout');b.disabled =true;try {const r =await fetch('/api/logout',{method:'POST'});if (!r.ok) throw new Error('Could not sign out.');window.close();if (!window.closed) document.body.innerHTML ='<main style="margin:48px auto;padding:24px;max-width:520px;font-family:system-ui,sans-serif;color:#14231d"><h1>Signed out</h1><p>Your setup session has ended. You can close this tab.</p></main>';} catch (e) {b.disabled =false;window.alert(e.message);}});loadSettings();</script>
    
</body></html>)HTML";

const char OTA_PAGE[] PROGMEM = R"HTML(<!doctype html>
<html lang="en">
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width,initial-scale=1">
    <title>OTA Update</title>
    <style>:root{font-family:system-ui,sans-serif;color:#14231d;background:#f3f6f2}*{box-sizing:border-box}body{margin:0 auto;padding:22px;max-width:620px}a{color:#27734f}h1{font-size:25px;margin:16px 0}p{color:#52645b;line-height:1.5}.field{display:grid;gap:8px;margin:20px 0}input{padding:12px;border:1px solid #bccbc1;border-radius:6px;background:#fff;font:inherit}button{padding:11px 15px;border:0;border-radius:6px;background:#27734f;color:white;font:inherit;font-weight:650}.status{margin-top:14px;min-height:24px}.overlay{position:fixed;inset:0;display:grid;place-items:center;background:rgba(243,246,242,.96);z-index:5;text-align:center;padding:24px}.overlay[hidden]{display:none}.panel{max-width:430px;background:#fff;border:1px solid #d3ded6;border-radius:12px;padding:28px;box-shadow:0 8px 30px #14231d22}.spinner{width:28px;height:28px;margin:0 auto 16px;border:3px solid #c8d8cd;border-top-color:#27734f;border-radius:50%;animation:spin 1s linear infinite}@keyframes spin{to{transform:rotate(360deg)}}</style>
</head>
<body>
    <a href="/">&larr; Home</a>
    <h1>OTA Update</h1>
    <p>Select the ESPHome OTA firmware image (usually named <code>firmware.ota.bin</code>
        ). Keep power connected until the device restarts.
    </p>
    <form id="upload">
        <div class="field"><label for="firmware">Firmware file</label>
            <input id="firmware" name="firmware" type="file" accept=".bin" required>
        </div>
        <button>Upload firmware</button>
        <div id="status" class="status" role="status"></div>
    </form>
    <div class="overlay" id="restart_overlay" hidden>
        <div class="panel">
            <div class="spinner" aria-hidden="true"></div>
            <h2>Device is restarting</h2>
            <p>The meter is temporarily unavailable while the update is applied. This page will refresh automatically in about 10 seconds.</p>
            <p id="retry_status">Waiting for the device…</p>
        </div>
    </div><script>function showRestart() {document.getElementById('restart_overlay').hidden =false;let attempts =0;const retry =async () =>{attempts++;try {const r =await fetch('/',{cache:'no-store'});if (r.ok) {location.reload();return}} catch (e) {} document.getElementById('retry_status').textContent =`Still waiting for the device… retry ${attempts}`;setTimeout(retry,2000)};setTimeout(retry,10000)} document.getElementById('upload').addEventListener('submit',async e =>{e.preventDefault();const s =document.getElementById('status');const form =e.target;const button =form.querySelector('button[type="submit"]')||form.querySelector('button');s.textContent ='Uploading... do not disconnect power.';if (button) button.disabled =true;try {const r =await fetch('/update',{method:'POST',body:new FormData(e.target)});const t =await r.text();if (!r.ok) throw new Error(t);s.textContent =t||'Update successful. The device is restarting.';showRestart()} catch (err) {if (err instanceof TypeError||err.message==='Failed to fetch') {s.textContent ='Update uploaded. The device is restarting.';showRestart();return} if (button) button.disabled =false;s.textContent =err.message||'Firmware update failed.'}});</script>
</body></html>)HTML";

const char PASSWORD_PAGE[] PROGMEM = R"HTML(<!doctype html>
<html lang="en">
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width,initial-scale=1">
    <title>Setup Password - P1 Meter</title>
    <style>:root{font-family:system-ui,sans-serif;color:#14231d;background:#f3f6f2}*{box-sizing:border-box}body{margin:0 auto;padding:22px;max-width:620px}a{color:#27734f}h1{font-size:25px;margin:16px 0 6px}p{color:#52645b;line-height:1.5}.field{display:grid;gap:8px;margin:20px 0}label{font-weight:600}input{width:100%;padding:12px;border:1px solid #bccbc1;border-radius:6px;background:#fff;font:inherit;color:#14231d}button{padding:11px 15px;border:0;border-radius:6px;background:#27734f;color:#fff;font:inherit;font-weight:650;cursor:pointer}.status{margin-top:14px;min-height:24px;color:#27734f}.error{color:#a33b2f}.password-field{position:relative}.password-field input{padding-right:52px}.password-toggle{position:absolute;right:4px;top:50%;transform:translateY(-50%);width:40px;height:40px;padding:8px;display:grid;place-items:center;border:0;background:transparent;color:#52645b}.password-toggle:hover{color:#27734f}.password-toggle:focus-visible{outline:2px solid #27734f}.password-toggle svg{width:20px;height:20px}.password-toggle .eye-off{display:none}.password-toggle[aria-pressed="true"] .eye-off{display:block}.password-toggle[aria-pressed="true"] .eye-on{display:none}.overlay{position:fixed;inset:0;background:#f3f6f2ee;display:grid;place-items:center}.overlay[hidden]{display:none}.panel{padding:28px;text-align:center;background:white;border-radius:12px;box-shadow:0 8px 30px #14231d22}</style>
</head>
<body>
    <a href="/">&larr; Home</a>
    <h1>Setup password</h1>
    <p>This password protects the P1-Meter-Setup Wi-Fi network and this web interface. Use 8 to 63 printable ASCII characters. Saving restarts the device; reconnect to P1-Meter-Setup with the new password. Web sign-in username: p1meter.</p>
    <form id="password">
        <div class="field">
            <label for="password_input">New setup password</label>
            <div class="password-field">
                <input id="password_input" name="password" type="password" minlength="8" maxlength="63" required autocomplete="new-password">
                <button class="password-toggle" type="button" aria-controls="password_input" aria-label="Show password" aria-pressed="false" title="Show password">
                    <svg class="eye-on" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">
                        <path d="M2 12s3.5-7 10-7 10 7 10 7-3.5 7-10 7S2 12 2 12Z" />
                        <circle cx="12" cy="12" r="3" />
                    </svg>
                    <svg class="eye-off" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">
                        <path d="M2 12s3.5-7 10-7 10 7 10 7-3.5 7-10 7S2 12 2 12Z" />
                        <path d="M4 4l16 16" />
                    </svg>
                </button>
            </div>
        </div>
        <div class="field">
            <label for="confirm">Confirm password</label>
            <div class="password-field">
                <input id="confirm" type="password" minlength="8" maxlength="63" required autocomplete="new-password">
                <button class="password-toggle" type="button" aria-controls="confirm" aria-label="Show password" aria-pressed="false" title="Show password">
                    <svg class="eye-on" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">
                        <path d="M2 12s3.5-7 10-7 10 7 10 7-3.5 7-10 7S2 12 2 12Z" />
                        <circle cx="12" cy="12" r="3" />
                    </svg>
                    <svg class="eye-off" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">
                        <path d="M2 12s3.5-7 10-7 10 7 10 7-3.5 7-10 7S2 12 2 12Z" />
                        <path d="M4 4l16 16" />
                    </svg>
                </button>
            </div>
        </div>
        <button type="submit">Save and reboot</button>
        <div id="status" class="status" role="status"></div>
    </form>
    <div class="overlay" id="restart_overlay" hidden>
        <div class="panel">
            <h2>Device is restarting</h2>
            <p>Reconnect to the P1-Meter-Setup network with the new password. This page will retry automatically.</p>
            <p id="retry_status">Waiting for the device…</p>
        </div>
    </div>
    <script>document.querySelectorAll('.password-toggle').forEach(button =>button.addEventListener('click',() =>{const input =document.getElementById(button.getAttribute('aria-controls'));const visible =input.type==='password';input.type =visible ?'text' :'password';button.setAttribute('aria-pressed',String(visible));button.setAttribute('aria-label',visible ?'Hide password' :'Show password');button.title =visible ?'Hide password' :'Show password'}));document.getElementById('password').addEventListener('submit',async e =>{e.preventDefault();const s =document.getElementById('status');const p =document.getElementById('password_input').value;const c =document.getElementById('confirm').value;s.className ='status';if (p!==c) {s.className ='status error';s.textContent ='The passwords do not match.';return} if (!/^[\x20-\x7e]{8,63}$/.test(p)) {s.className ='status error';s.textContent ='Use 8 to 63 printable ASCII characters.';return} s.textContent ='Saving…';try {const r =await fetch('/api/setup-password',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:new URLSearchParams({password:p})});if (!r.ok) throw new Error(await r.text());document.getElementById('restart_overlay').hidden =false;let n =0;const retry =async () =>{n++;try {const q =await fetch('/',{cache:'no-store'});if (q.ok) {location.reload();return}} catch (e) {} document.getElementById('retry_status').textContent ='Still waiting… retry ' +n;setTimeout(retry,2000)};setTimeout(retry,10000)} catch (err) {s.className ='status error';s.textContent =err.message}});</script>
    
</body></html>)HTML";

const char REBOOT_PAGE[] PROGMEM = R"HTML(<!doctype html>
<html lang="en">
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width,initial-scale=1">
    <title>Reboot</title>
    <style>:root{font-family:system-ui,sans-serif;color:#14231d;background:#f3f6f2}body{margin:0 auto;padding:22px;max-width:620px}a{color:#27734f}h1{font-size:25px}p{color:#52645b;line-height:1.5}button{padding:11px 15px;border:0;border-radius:6px;background:#27734f;color:white;font:inherit;font-weight:650}.status{margin-top:14px}.overlay{position:fixed;inset:0;display:grid;place-items:center;background:rgba(243,246,242,.96);z-index:5;text-align:center;padding:24px}.overlay[hidden]{display:none}.panel{max-width:430px;background:#fff;border:1px solid #d3ded6;border-radius:12px;padding:28px;box-shadow:0 8px 30px #14231d22}.spinner{width:28px;height:28px;margin:0 auto 16px;border:3px solid #c8d8cd;border-top-color:#27734f;border-radius:50%;animation:spin 1s linear infinite}@keyframes spin{to{transform:rotate(360deg)}}</style>
</head>
<body>
    <a href="/">&larr; Home</a>
    <h1>Reboot device</h1>
    <p>Restart the meter now.</p>
    <button id="reboot">Reboot</button>
    <p class="status" id="status" role="status"></p>
    <div class="overlay" id="restart_overlay" hidden>
        <div class="panel">
            <div class="spinner" aria-hidden="true"></div>
            <h2>Device is restarting</h2>
            <p>The meter is temporarily unavailable while it restarts. This page will refresh automatically in about 10 seconds.</p>
            <p id="retry_status">Waiting for the device…</p>
        </div>
    </div>
    <script>const overlay =document.getElementById('restart_overlay');function showRestart() {overlay.hidden =false;let attempts =0;const retry =async () =>{attempts++;try {const r =await fetch('/',{cache:'no-store'});if (r.ok) {location.reload();return}} catch (e) {} document.getElementById('retry_status').textContent =`Still waiting for the device… retry ${attempts}`;setTimeout(retry,2000)};setTimeout(retry,10000)} document.getElementById('reboot').addEventListener('click',async () =>{if (!confirm('Reboot the P1 meter now?')) return;const s =document.getElementById('status');s.textContent ='Rebooting…';try {const r =await fetch('/api/reboot',{method:'POST'});if (!r.ok) throw new Error(await r.text());showRestart()} catch (e) {s.textContent =e.message}});</script>
     
</body></html>)HTML";

}  // namespace esphome::p1_setup::web_pages
