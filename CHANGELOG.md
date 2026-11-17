# Changelog

## 0.5.0

### HomeWizard Energy integration
- Added support for the HomeWizard Local API (v1), allowing the P1 meter to integrate directly with the Home Assistant HomeWizard Energy integration and third-party apps.
- Automatic network discovery (mDNS), allowing Home Assistant and other compatible apps to detect the P1 meter automatically on your local network.
- Easily enable or disable HomeWizard Local API support from the device configuration web page.

### Settings and connectivity
- Added a toggle in the configuration web page to turn the ESPHome Native API on or off, with a prompt to restart the device when changed.

### Meter readings and sensors
- Added support for 4-tariff smart meters (Tariff 3 and Tariff 4 for energy consumed and returned).
- Added average active power demand and monthly peak power demand tracking (useful for capacity-based billing, such as the Belgian capacity tariff).
- Added grid frequency monitoring.
- Added voltage drop (sag) and voltage peak (swell) tracking for Phase 2 and Phase 3.

## 0.4.0

> **BREAKING CHANGE — READ BEFORE UPGRADING:** This version introduces LittleFS to store configuration and data in flash and help reduce flash wear. Install it on a reset device; do not OTA-update an existing installation, as the device may stop working.

### Dashboard and history
- Added hourly on-device history for power and gas consumption, displayed in the dashboard.
- Improved history storage initialization and reliability, including validation, backups, and retry handling for failed writes.

### Web interface
- Improved OTA and reboot pages with restart overlays and clearer feedback.
- Streamlined dashboard rendering and MQTT settings application.

## 0.3.1

### Meter compatibility
- The firmware automatically detects whether the gas meter reports on M-Bus 1 or M-Bus 2 and remembers the selection after restart.
- Added support for encrypted DSMR telegrams when a decryption key is configured.

## 0.3.0

### Web interface
- Added Live Meter Dashboard page
Live readings displayed in this Dashboard:
  - **Power now:** Current power consumed and returned (Watts)
  - **Energy totals:** Energy consumed/returned per tariff and gas consumption (kWh, m³)
  - **Phase readings:** Power, current, and voltage per phase (kW, A, V)
  - **Meter information:** P1 Meter ID, P1 Version, and active tariff group
  - **Power quality:** Short and long power outages, and voltage drops/peaks on phase 1

## 0.2.0

- Removed the footer text from the setup home page.
- Added a simplified installation guide for released firmware that needs no coding.
- Added a software usage guide for the web interface.
- Reorganized the documentation.
- The example `secrets-example.yaml` now uses a placeholder encryption key. Generate your own with `openssl rand -base64 32`.
- The build script now links `secrets.yaml` into each device folder, so `!secret` values resolve when building.
- Updated the VS Code setup and the release workflow.

## 0.1.0

- Reads your electricity and gas meter through its P1 port using a Wemos D1 mini.
- Shows electricity use and return per tariff, total and per phase power, current and voltage per phase, and gas use in Home Assistant.
- Shows power outages, short voltage dips and peaks on phase 1, meter ID, P1 version, and active tariff group.
- Standard version: connects to Home Assistant automatically. MQTT can be turned on or off with a switch.
- Advanced version: set up Wi-Fi and MQTT from a web page on the device's own setup network, without editing files.
- Advanced version: update firmware, factory reset, and reboot from the same web page.
- Updates can be installed over Wi-Fi. Reboot switch available in Home Assistant.
- Onboard LED shows Wi-Fi status.
