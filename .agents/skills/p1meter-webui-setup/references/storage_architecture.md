# P1 Setup Storage Architecture & Flash Wear Protection

This document details the on-device LittleFS storage architecture, data record structures, wear-leveling strategies, and failure recovery mechanisms implemented in `components/p1_setup`.

---

## 1. Storage Files & Dual-File Scheme

`p1_setup` stores configuration and history on LittleFS:

| File Name | Purpose | Lifetime Workload |
| :--- | :--- | :--- |
| `/p1-config.bin` | Active device configuration (`ConfigStorageRecord`) | Infrequent (only on user settings change) |
| `/p1-config.tmp` | Staging file for atomic configuration updates | Ephemeral |
| `/p1-config.bak` | Backup of previous valid configuration | Rollback copy |
| `/p1-history-a.bin` | History slot A (`HistoryStorageRecord`) | Frequent (every 30m / rollover / shutdown) |
| `/p1-history-b.bin` | History slot B (`HistoryStorageRecord`) | Alternates with slot A |

---

## 2. Record Data Structures

All structures are defined in [p1_setup.h](../../../../components/p1_setup/p1_setup.h) and [p1_setup_internal.h](../../../../components/p1_setup/p1_setup_internal.h).

### Configuration Record (`ConfigStorageRecord`)
* **Magic:** `0x50314346` (`P1CF`)
* **Version:** `1`
* **Contents:**
  - `sequence` (uint32_t): Incrementing generation counter.
  - `settings` (`SetupSettings`): WiFi SSID/pass, MQTT broker/port/creds, static IP config.
  - `setup_password` (`SetupPassword`): WebUI setup password.
  - `mqtt_enabled` (uint8_t): 1 if MQTT active, 0 otherwise.
  - `history_enabled` (uint8_t): 1 if on-device history enabled, 0 otherwise.
  - `checksum` (uint32_t): Sum of 32-bit words excluding checksum field.

### History Record (`HistoryStorageRecord`)
* **Magic:** `0x50314846` (`P1HF`)
* **Version:** `1`
* **Contents:**
  - `sequence` (uint32_t): Incrementing generation counter.
  - `history` (`HistorySettings`):
    - `magic`: `0x50314849` (`P1HI`)
    - `buckets[72]`: 72 hourly buckets (representing today, yesterday, 2 days ago).
      - `hour_key` (uint32_t): Formatted as `YYYYMMDDHH`.
      - `energy_consumed_kwh` (float).
      - `energy_returned_kwh` (float).
      - `gas_consumed_m3` (float).
      - `valid` (uint8_t): 1 if bucket has valid measurements.
  - `checksum` (uint32_t): Sum of 32-bit words excluding checksum field.

---

## 3. Atomic Configuration Write Algorithm

To prevent corruption if power is cut during a configuration write, `save_config_storage_()` implements atomic staging and backup rotation:

1. Increment `sequence` counter and compute checksum.
2. Write record to temporary file `/p1-config.tmp`, flush and close.
3. Read back `/p1-config.tmp` to verify byte count and sequence integrity.
4. If valid:
   a. If old `/p1-config.bak` exists, remove it.
   b. If active `/p1-config.bin` exists, rename it to `/p1-config.bak`.
   c. Rename `/p1-config.tmp` to `/p1-config.bin`.
   d. Verify newly promoted `/p1-config.bin`.
5. If promotion fails, restore `/p1-config.bak` to `/p1-config.bin`.
6. On startup (`load_config_storage_()`), if `/p1-config.bin` fails checksum verification, automatically fall back to `/p1-config.bak`.

---

## 4. History Dual-Alternating Slots & Failure Protection

### Dual Alternation
* Consecutive history checkpoints toggle between `/p1-history-a.bin` and `/p1-history-b.bin`.
* On boot, both files are scanned; the file with the higher valid `sequence` counter is loaded.
* If one file is corrupt or truncated, the previous 30-minute checkpoint in the sibling slot remains valid.

### Failure Handling & Exponential Backoff
When flash writes fail (e.g., LittleFS full or worn flash blocks):
1. **Dirty flag preservation:** If write or readback verification fails, `history_dirty_` remains `true` so changes are not lost.
2. **Exponential backoff:**
   - Base retry: 10 seconds (`HISTORY_RETRY_BASE_DELAY_MS`).
   - Doubled on each consecutive failure: 20s, 40s... up to max 60s (`HISTORY_RETRY_MAX_DELAY_MS`).
3. **Consecutive failure limit:**
   - After `HISTORY_MAX_CONSECUTIVE_FAILURES` (3 consecutive failed writes):
     - On-Device History is automatically disabled in RAM to prevent runaway flash wear.
     - The disabled state is saved to config storage.
     - An error is logged: `Disabling On-Device History due to persistent flash write errors`.

---

## 5. Flash Wear Estimation

Flash blocks withstand typically 10,000 to 100,000 erase cycles.
History writes occur:
* Checkpoints: every 30 minutes (48 writes/day).
* Hour rollovers (24 writes/day).
* Deliberate reboots / OTA uploads (~occasional).
Total: ~72 writes/day (~26,280 writes/year).

Because records are small (~2.4 KB) and LittleFS distributes writes across the ~1 MB to 2.8 MB partition, the effective wear leveling extends flash life:
* **ESP8266 (1 MB LittleFS):** 2–5 years estimated additional workload.
* **ESP32 (1.4 MB LittleFS):** 4–8 years estimated additional workload.
* **ESP32-S3 (2.8 MB LittleFS):** 8–15 years estimated additional workload.

History is disabled by default to guarantee device longevity unless explicitly requested by the user.
