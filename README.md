# ESPHome P1 Meter

ESPHome configuration for reading a DSMR-compatible electricity meter P1 port with a Wemos D1 mini (ESP8266).<br>
$\color{gray}{\textsf{Every firmware variant exposes meter readings through the ESPHome native API and provides switches for MQTT control and reboot.}}$

## Contents

- [Installation](#installation)
	- [Useful Terms](#useful-terms)
- [Supported Devices](#supported-devices)
- [Hardware and Wiring](#hardware-and-wiring)
	- [Check Your Meter First](#check-your-meter-first)
	- [P1 Wiring](#p1-wiring)
	- [Optional Meter Power](#optional-meter-power)
- [Firmware Options](#firmware-options)
- [On-Device History Storage](#on-device-history-storage)
- [Home Assistant and MQTT](#home-assistant-and-mqtt)
- [Troubleshooting](#troubleshooting)
- [Safety and Security](#safety-and-security)
- [License](#license)

## Installation

Choose a guide to configure, compile, and install the software:

- [Install Released Version via web.esphome.io](docs/README-Install-Released-Version.md): <strong>Non-technical people start here.</strong><br>Install the released firmware (no compiling required).
- [ESPHome dashboard in Docker](docs/README-ESPHome-Dashboard-Dockered.md) <br>For those who prefer a browser-based ESPHome dashboard. Copy either YAML configuration into the dashboard; the advanced configuration's component is fetched automatically, so you do not need to download the project code. Use it for the Dashboard-Standard or Dashboard-Advanced configuration.
- [VS Code](docs/README-VSCode.md): build and flash the Public and Advanced firmwares with VS Code tasks or PlatformIO IDE, using a development container.

[Software Usage Guide](docs/README-Software-Usage-Guide.md): Instructions of how to use the software.

### Useful Terms

- **Configuration / YAML:** A text file containing the device settings. YAML is the format used for these files; the spaces at the start of lines matter.
- **DSMR:** A smart meter data standard, mostly used in the Netherlands. The meter sends readings in a format this firmware understands.
- **P1 port:** The data port on a smart meter, where the D1 mini connects with an RJ11 or RJ12 cable.
- **Firmware:** The software that runs on the D1 mini. **Compile** means build that software from a configuration; **flash** means install it on the device.
- **Firmware file (`firmware.bin`):** The file for the first installation over USB. **OTA file (`firmware.ota.bin`):** The file for later wireless updates; boards that publish only one file use `firmware.bin` for both.
- **OTA (over the air):** Updating the device over Wi-Fi instead of USB.
- **MQTT:** An optional way to send readings through a separate server called a broker. It is not needed to use the ESPHome integration in Home Assistant.
- **Secrets:** Private values such as Wi-Fi passwords, stored in `secrets.yaml` rather than directly in the configuration.
- **Terminal / PowerShell:** An application where you type commands. **Docker** runs the ESPHome dashboard in an isolated application called a container.

## Hardware and Wiring

- Wemos D1 mini (ESP8266)
- A 10 kΩ resistor
- A 4-contact RJ11 or 6-contact RJ12 cable and a compatible smart meter with its P1 port enabled
- A USB power supply, unless the meter can supply enough power through its P1 port

### Check Your Meter First

1. Confirm the meter is DSMR-compatible and its P1 port is enabled. Ask your energy supplier or meter operator if you are unsure; they may need to enable the port.
2. Find the P1 wiring diagram for your exact meter model and check that its pins match the table below. Wire colors and plug orientation can differ.

If anything is unclear, ask the meter operator, supplier, or a qualified technician before connecting. Do not guess from cable colors, and never connect to mains wiring.

### P1 Wiring

Verify the meter's P1 pinout and signal voltage before connecting: GPIO3 accepts 3.3 V logic, not 5 V. Use a suitable interface if your meter requires different signaling or voltage levels; the resistor alone does not make a 5 V signal safe.

| P1 pin | Signal | D1 mini connection |
| --- | --- | --- |
| 2 | RTS / data request | 3.3 V |
| 3 | Signal ground | GND |
| 4 | Not used | Not connected |
| 5 | Data | RX / GPIO3 |

Connect a 10 kΩ pull-up resistor between 3.3 V and the data line (P1 pin 5 / GPIO3). Many Landis+Gyr models require this resistor; some meters also accept 3.3 V on RTS, but requirements vary. With a 4-contact RJ11 plug, the outer contacts, P1 pins 1 and 6, are absent; the remaining contacts are pins 2 through 5. Check the actual cable's pinout, since wire colors and cable orientation can vary.

Power the D1 mini over USB for this wiring.

<details>
<summary><strong>🔌 Show USB-powered wiring diagram</strong></summary>

![USB-powered wiring diagram](docs/images/p1-wiring-usb-powered.jpeg)
</details>

### Optional Meter Power

On compatible DSMR 5+ meters with a 6-contact cable, these additional connections can power the D1 mini instead of USB:

| P1 pin | Signal | D1 mini connection |
| --- | --- | --- |
| 1 | 5 V supply | 5V / VIN |
| 6 | Power ground | GND |

Keep the connections and pull-up resistor above. Before using meter power, verify that the meter supplies 5 V and enough current for the D1 mini, including Wi-Fi peaks.

<details>
<summary><strong>⚡ Show meter-powered wiring diagram</strong></summary>

![Meter-powered wiring diagram](docs/images/p1-wiring-meter-powered.jpeg)
</details><br>

Both configurations read inverted serial data on GPIO3 (`RX`) at 115200 baud, 8 data bits, no parity, and 1 stop bit. The D1 mini's UART transmit pin is not used.

## Supported Devices

- Wemos D1 mini (ESP8266): tested with the project; recommended
- NodeMCU v2 (ESP8266): compiles; not tested on hardware
- ESP32 DevKit (ESP32): compiles; not tested on hardware
- ESP32-S2 Saola-1 (ESP32-S2): compiles; not tested on hardware
- ESP32-S3 DevKitC-1 (ESP32-S3): compiles; not tested on hardware
- ESP32-C3 DevKitM-1 (ESP32-C3): compiles; not tested on hardware

The ESP32-C6 is not supported.

<details>
<summary><strong>📋 Show GPIO pin table</strong></summary>

| Board | Chip | Status | Light (LED) GPIO | RX GPIO | RX GPIO (Alternative) |
| --- | --- | --- | --- | --- | --- |
| Wemos D1 mini | ESP8266 | Tested with the project; recommended | GPIO2 | GPIO3 <br><br>$\color{gray}{\textsf{GPIO3 is the ESP8266 UART0 RX pin. The ESPHome docs list GPIO3 as UART0 RX.}}$ | GPIO13 (D7) <br><br>$\color{gray}{\textsf{ESPHome docs say hardware UART0 is RX GPIO13 with TX GPIO15. An RX-only config on GPIO13 may fall back to software UART, which is less accurate at 115200. GPIO15 is a boot pin, so it should not be used as TX.}}$ |
| NodeMCU v2 | ESP8266 | Compiles; not tested on hardware | GPIO2 | GPIO3 <br><br>$\color{gray}{\textsf{GPIO3 is the ESP8266 UART0 RX pin. The ESPHome docs list GPIO3 as UART0 RX.}}$ | GPIO13 (D7) <br><br>$\color{gray}{\textsf{ESPHome docs say hardware UART0 is RX GPIO13 with TX GPIO15. An RX-only config on GPIO13 may fall back to software UART, which is less accurate at 115200. GPIO15 is a boot pin, so it should not be used as TX.}}$ |
| ESP32 DevKit | ESP32 | Compiles; not tested on hardware | GPIO2 | GPIO3 <br><br>$\color{gray}{\textsf{GPIO3 is the default UART0 RX (the USB serial console). That conflicts only if logging is on.}}$ | GPIO16 <br><br>$\color{gray}{\textsf{Default UART2 RX. Avoid it on modules with PSRAM (e.g. WROVER), which use GPIO16/17. Any GPIO can be routed through the matrix, so GPIO25 also works.}}$ |
| ESP32-S2 Saola-1 | ESP32-S2 | Compiles; not tested on hardware | GPIO18 (addressable LED) | GPIO3 <br><br>$\color{gray}{\textsf{Logger uses USB_CDC, so GPIO3 is free.}}$ | GPIO44 <br><br>$\color{gray}{\textsf{Default UART0 RX. The logger uses USB CDC, so it is free. Any unused GPIO also works via the matrix. Avoid GPIO26–32 on PSRAM modules.}}$ |
| ESP32-S3 DevKitC-1 | ESP32-S3 | Compiles; not tested on hardware | GPIO48 (addressable LED) | GPIO3 <br><br>$\color{gray}{\textsf{Logger uses USB_SERIAL_JTAG, so GPIO3 is free. Verify the GPIO3 strapping behavior on the ESP32-S3 before relying on it.}}$ | GPIO4, GPIO5, GPIO6 or GPIO7 <br><br>$\color{gray}{\textsf{Avoid GPIO43/44 on DevKitC-1 (UART bridge pins). Avoid GPIO19/20 (USB), GPIO3 (strapping), and GPIO26–37 (PSRAM/flash).}}$ |
| ESP32-C3 DevKitM-1 | ESP32-C3 | Compiles; not tested on hardware | GPIO8 (addressable LED) | GPIO3 <br><br>$\color{gray}{\textsf{Logger uses USB_SERIAL_JTAG. GPIO3 is a plain GPIO.}}$ | GPIO20 <br><br>$\color{gray}{\textsf{Default UART0 RX. The logger uses USB Serial/JTAG, so it is not claimed. GPIO4, GPIO5, GPIO6, GPIO7 and GPIO10 are also free. Avoid GPIO2, GPIO8 and GPIO9 (strapping), and GPIO12–17 (internal flash on some modules; verify for your module).}}$ |
</details>

## Firmware Options

- Public $\color{gray}{\textsf{(released version)}}$
- Advanced $\color{gray}{\textsf{(VSCode/PlatformIO compile)}}$
- Dashboard-Standard $\color{gray}{\textsf{(ESPHome Dashboard, file}}$ `p1meter_wemosd1mini_noWebUI.yaml` $\color{gray}{\textsf{)}}$
- Dashboard-Advanced $\color{gray}{\textsf{(ESPHome Dashboard, file}}$ `p1meter_wemosd1mini_withWebUI.yaml` $\color{gray}{\textsf{)}}$

<details>
<summary><strong>⚙️ Show firmware variants table</strong></summary>

| Variant | Install with | Web interface | Encrypted API | OTA updates | Use when |
| --- | --- | --- | --- | --- | --- |
| Public (`firmwares/public`) | ESPHome Web (Download `.bin` from `releases`), VS Code or PlatformIO | Yes (setup password on first run) | No | Web interface only | You want the quickest start on a trusted network. |
| Advanced (`firmwares/advanced`) | VS Code or PlatformIO (using your own keys) | Yes | Yes | ESPHome API and web interface | You compile yourself with your own encryption key and OTA password. |
| Dashboard-Standard (`p1meter_wemosd1mini_noWebUI.yaml`) | ESPHome Dashboard | No | Yes | ESPHome API | You install from the ESPHome Dashboard and can only modify Wi-Fi and MQTT via the ESPHome Dashboard. |
| Dashboard-Advanced (`p1meter_wemosd1mini_withWebUI.yaml`) | ESPHome Dashboard | Yes | Yes | ESPHome API and web interface | You install from the Dashboard and can modify Wi-Fi and MQTT anytime via the web interface. |
</details><br>

All variants expose the following configured DSMR sensor set:
- Imported and exported energy for tariffs 1 and 2
- Total imported and exported power
- Imported power, current, and voltage per phase
- Gas consumption
- Short and long power-outage counts
- Voltage-drop and voltage-peak counts on phase 1
- Meter identification, P1 version, and active tariff group

A meter only reports values supported by its telegram, so some entities may remain unavailable.

Gas is detected automatically on MBUS 1 or MBUS 2. The firmware starts with MBUS 1, switches when
the other MBUS is the one reporting a valid gas value, and persists that selection in flash. No
`gas_mbus_id` setting is required in the ESPHome YAML.

$\color{orange}{\textsf{Notes:}}$<br>
$\color{orange}{\textsf{- Public variant supports OTA only through its web interface.}}$<br>
$\color{orange}{\textsf{- Dashboard-Standard supports ESPHome OTA only.}}$<br>
$\color{orange}{\textsf{- Advanced and Dashboard-Advanced variants support ESPHome OTA and browser-based OTA through the web interface. (ESPHome OTA is protected by}}$ `p1meter__ota_password` $\color{orange}{\textsf{from}}$ `secrets.yaml`$\color{orange}{\textsf{)}}$

## On-Device History Storage

The WebUI dashboard can optionally keep hourly usage for Energy Consumed, Energy Returned, and Gas
Consumed for today and the two previous days. **On-Device History is off by default**, so no history
is written to flash until the user enables it in the dashboard.<br>
$\color{red}{\textbf{\textsf{Enabling this feature can wear your device faster; if previously enabled it, then disabling it deletes all already-saved history.}}}$

Additionally:

- The active hour is checkpointed every 30 minutes
and at hour rollover.
- The latest history is also flushed before a WebUI OTA update or deliberate WebUI reboot.
- When history is disabled, the dashboard hides the day selector and graphs because there is no data to display.
- The wear figures shown in the confirmation dialog are conservative engineering estimates for the additional flash-writing workload, not guarantees:
-- Wemos D1 mini and NodeMCU v2: approximately 2–5 years
-- ESP32 DevKit, ESP32-S2 Saola-1, and ESP32-C3 DevKitM-1: approximately 4–8 years
-- ESP32-S3 DevKitC-1: approximately 8–15 years.
(Actual life depends on flash quality, filesystem wear-leveling, temperature, power stability, and other firmware writes.)

<details>
<summary><strong>💾 Show History storage and flash layouts</strong></summary>

The dashboard history is stored in LittleFS as two alternating, checksummed records. Each record is small (the 72 hourly buckets plus metadata), so the filesystem is intentionally much larger than the data file to leave room for filesystem wear-leveling and future metadata. Writes occur every 30 minutes, at an hour rollover, and before a WebUI OTA update or deliberate WebUI reboot.

| Supported target | Flash assumption | OTA app slots | LittleFS/data partition | Configuration |
| --- | ---: | ---: | ---: | --- |
| Wemos D1 mini / NodeMCU v2 | 4 MB | approximately 1 MB each | approximately 1 MB | `eagle.flash.4m1m.ld` |
| ESP32 DevKit / ESP32-S2 / ESP32-C3 | 4 MB | 1.25 MiB each | 1.4375 MiB | `firmwares/partitions/esp32-4mb.csv` |
| ESP32-S3 DevKitC-1 | 8 MB documented target | 2.5 MiB each | 2.875 MiB | `firmwares/partitions/esp32-8mb.csv` |

The ESP8266 linker layout and both ESP32 partition tables reserve separate OTA slots and keep the
LittleFS/data area outside those slots, so normal OTA updates preserve history. The ESP32 4 MB layout
has 1,310,720-byte app slots; the current advanced ESP32 DevKit image is 1,184,271 bytes, leaving
126,449 bytes (about 9.6%) in the smallest slot. The ESP32-S3 configuration targets the commonly
documented 8 MB DevKitC-1 variant; verify the actual flash size of a different S3 module before
flashing it. The ESP32-C6 is not supported.

</details>

## Home Assistant - ESPHome API and MQTT

Add or accept the discovered `p1-meter` node in Home Assistant's ESPHome integration. The native API is enabled in both configurations and has a 30-minute reboot timeout if the API client remains disconnected.

- You can use the **MQTT** switch to enable or disable the client
- You can use the **Reboot** switch to reboot the device.

## Troubleshooting

- **Configuration validation fails:** Check YAML indentation, `secrets.yaml` presence and key names, and that the installed ESPHome version supports the configured components.
- **Cannot flash over USB:** Try a known data cable and another USB port, confirm the serial device name and permissions, and close serial monitors or other programs using the port.
- **Device does not join Wi-Fi:** Verify the SSID and password, 2.4 GHz Wi-Fi availability for the ESP8266, and router client isolation settings.
- **Dashboard shows the device as offline:** Check that mDNS multicast traffic on UDP port 5353 can reach the Dashboard. On Linux, run the Dashboard with host networking as described in the [Dashboard guide](docs/README-ESPHome-Dashboard-Dockered.md#start-the-dashboard). The Dashboard and device should be on the same network; separate VLANs or subnets need an mDNS reflector and appropriate firewall rules. This allows discovery with either DHCP or a static IP, without `use_address`.
- **No meter values:** Confirm the meter's P1 port is enabled, the interface wiring and signal levels are correct, the interface output reaches GPIO3, and the meter's telegram baud rate and format match the configuration. Check logs with `esphome logs p1meter_wemosd1mini_noWebUI.yaml` or `esphome logs p1meter_wemosd1mini_withWebUI.yaml`.
- **MQTT does not connect:** Verify that MQTT is enabled, then check broker address, port/reachability, username, password, and broker logs.
- **OTA cannot find the node:** Check that the computer and device can communicate on the LAN, then specify its current IP or hostname. For the standard configuration, `wifi.use_address` can also be set. Use USB flashing if network connectivity has not been established.

The onboard LED on GPIO2 stays on while Wi-Fi is connected. The standard firmware blinks it once per second while disconnected; the advanced firmware blinks it every 250 ms while its setup AP is active, otherwise once per second while disconnected.

## Safety and Security

Keep `secrets.yaml` private. Do not commit Wi-Fi or broker credentials. If credentials have been exposed, rotate them. Verify the meter's P1 wiring and voltage levels; use a compatible interface where needed. Incorrect wiring or voltage levels can damage the meter or the D1 mini.

## License

Unless otherwise noted, original project code and documentation are licensed under the [GNU General Public License v3.0](LICENSE). Third-party material is not relicensed by this file and remains subject to its own terms and notices.

> **Note:** Prior to commit eaecb40ffc8321b3ec5dbb5de69d3f795c0d6c2d, the license was incorrectly stated. The correct license for all code derived from ESPHome has always been GPL-3.0.
