# Changelog

## 0.6.0

### First public release under the GNU General Public License v3.0

This release re-licenses the entire project under the **GNU General Public License v3.0** (GPL-3.0), resolving the licensing conflict with ESPHome (which is itself GPL-3.0). All previously released firmware binaries built under the earlier PolyForm Noncommercial License have been removed.

P1 Meter for ESPHome is an ESPHome configuration for reading a DSMR-compatible electricity meter P1 port with a Wemos D1 mini (ESP8266) or ESP32 devices.

### Supported hardware
- Wemos D1 mini / NodeMCU v2 (ESP8266)
- ESP32 DevKit, ESP32-S2, ESP32-C3, ESP32-S3 DevKitC-1

### Firmware variants
- **Standard:** Connects to Home Assistant automatically via the ESPHome Native API. MQTT can be enabled or disabled with a switch.
- **Advanced:** Set up Wi-Fi and MQTT from a web page on the device's own setup network, without editing files. Update firmware, factory reset, and reboot from the same web page. Updates can be installed over Wi-Fi.

### Core features
- Reads electricity and gas meters through the P1 port using a Wemos D1 mini
- Shows electricity use and return per tariff, total and per-phase power, current and voltage per phase, and gas use in Home Assistant
- Shows power outages, short voltage dips and peaks on phase 1, meter ID, P1 version, and active tariff group
- Onboard LED shows Wi-Fi status

### Web interface (Advanced variant)
- Live Meter Dashboard page with real-time readings:
  - Power now: Current power consumed and returned (Watts)
  - Energy totals: Energy consumed/returned per tariff and gas consumption (kWh, m³)
  - Phase readings: Power, current, and voltage per phase (kW, A, V)
  - Meter information: P1 Meter ID, P1 Version, and active tariff group
  - Power quality: Short and long power outages, and voltage drops/peaks on phase 1
- Wi-Fi and MQTT configuration without editing files
- Firmware update, factory reset, and reboot from the web page
- Over-the-air (OTA) updates via Wi-Fi

### HomeWizard Energy integration
- Support for the HomeWizard Local API (v1) for direct integration with Home Assistant and third-party apps
- Automatic network discovery (mDNS)
- Toggle to enable or disable HomeWizard Local API support from the configuration web page

### Settings and connectivity
- Toggle in the configuration web page to turn the ESPHome Native API on or off (with restart prompt)

### Meter readings and sensors
- Support for 4-tariff smart meters (Tariff 3 and Tariff 4 for energy consumed and returned)
- Average active power demand and monthly peak power demand tracking
- Grid frequency monitoring
- Voltage drop (sag) and voltage peak (swell) tracking for Phase 2 and Phase 3

### Data storage
- Hourly on-device history for power and gas consumption using LittleFS
- History storage initialization, validation, backups, and retry handling

### Documentation
- Simplified installation guide for released firmware (no coding required)
- Software usage guide for the web interface
- VS Code and PlatformIO build instructions with development container support
