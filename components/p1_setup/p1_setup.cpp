#include "p1_setup_internal.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"

#ifdef USE_API
#include "esphome/components/api/api_server.h"
#endif

#if defined(USE_ESP8266) || (defined(USE_ESP32) && defined(USE_ARDUINO))
#include <LittleFS.h>
#endif

#if defined(USE_ESP32) && defined(USE_ARDUINO)
// ESPHome's selective Arduino build omits the POSIX directory-removal shim,
// while Arduino FS references it even though this component never removes directories.
extern "C" int rmdir(const char *) { return -1; }
#endif

#include <cstddef>
#include <cstdio>

namespace esphome::p1_setup {

namespace {

bool parse_timestamp(const std::string &timestamp, uint32_t *day_key, uint8_t *hour) {
  if (timestamp.size() < 10)
    return false;
  for (size_t i = 0; i < 10; i++) {
    if (timestamp[i] < '0' || timestamp[i] > '9')
      return false;
  }
  const uint16_t year = 2000 + (timestamp[0] - '0') * 10 + timestamp[1] - '0';
  const uint8_t month = (timestamp[2] - '0') * 10 + timestamp[3] - '0';
  const uint8_t day = (timestamp[4] - '0') * 10 + timestamp[5] - '0';
  *hour = (timestamp[6] - '0') * 10 + timestamp[7] - '0';
  if (month < 1 || month > 12 || day < 1 || day > 31 || *hour > 23)
    return false;
  *day_key = static_cast<uint32_t>(year) * 10000UL + month * 100UL + day;
  return true;
}

uint32_t previous_day(uint32_t day_key) {
  uint16_t year = day_key / 10000UL;
  uint8_t month = (day_key / 100UL) % 100;
  uint8_t day = day_key % 100;
  if (day > 1) {
    day--;
  } else {
    if (month == 1) {
      year--;
      month = 12;
    } else {
      month--;
    }
    const bool leap = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
    const uint8_t days_in_month[] = {31, static_cast<uint8_t>(leap ? 29 : 28), 31, 30, 31, 30,
                                     31, 31, 30, 31, 30, 31};
    day = days_in_month[month - 1];
  }
  return static_cast<uint32_t>(year) * 10000UL + month * 100UL + day;
}

sensor::Sensor *find_sensor(const char *name) {
  for (auto *sensor : App.get_sensors()) {
    if (sensor != nullptr && !strcmp(sensor->get_name().c_str(), name))
      return sensor;
  }
  return nullptr;
}

text_sensor::TextSensor *find_text_sensor(const char *name) {
  for (auto *sensor : App.get_text_sensors()) {
    if (sensor != nullptr && !strcmp(sensor->get_name().c_str(), name))
      return sensor;
  }
  return nullptr;
}

#if defined(USE_ESP8266) || (defined(USE_ESP32) && defined(USE_ARDUINO))
template<typename T> uint32_t record_checksum(const T &record) {
  const auto *data = reinterpret_cast<const uint8_t *>(&record);
  constexpr size_t length = offsetof(T, checksum);
  uint32_t crc = 0xFFFFFFFFUL;
  for (size_t i = 0; i < length; i++) {
    crc ^= data[i];
    for (uint8_t bit = 0; bit < 8; bit++)
      crc = (crc >> 1) ^ (0xEDB88320UL & (0UL - (crc & 1UL)));
  }
  return ~crc;
}

struct LegacyConfigStorageRecordV1 {
  uint32_t magic;
  uint32_t version;
  uint32_t sequence;
  SetupSettings settings;
  SetupPassword setup_password;
  uint8_t mqtt_enabled;
  uint8_t history_enabled;
  uint32_t checksum;
};

struct LegacyConfigStorageRecordV2 {
  uint32_t magic;
  uint32_t version;
  uint32_t sequence;
  SetupSettings settings;
  SetupPassword setup_password;
  uint8_t mqtt_enabled;
  uint8_t history_enabled;
  uint8_t homewizard_enabled;
  uint32_t checksum;
};

bool read_config_file(const char *path, ConfigStorageRecord *record) {
  File file = LittleFS.open(path, "r");
  if (!file || file.size() != sizeof(ConfigStorageRecord))
    return false;
  const size_t read = file.read(reinterpret_cast<uint8_t *>(record), sizeof(ConfigStorageRecord));
  file.close();
  return read == sizeof(ConfigStorageRecord) && record->magic == CONFIG_STORAGE_MAGIC &&
         record->version == CONFIG_STORAGE_VERSION && record->settings.magic == SETTINGS_MAGIC &&
         record->setup_password.magic == SETUP_PASSWORD_MAGIC && record->checksum == record_checksum(*record);
}

bool read_legacy_config_file_v1(const char *path, LegacyConfigStorageRecordV1 *record) {
  File file = LittleFS.open(path, "r");
  if (!file || file.size() != sizeof(LegacyConfigStorageRecordV1))
    return false;
  const size_t read = file.read(reinterpret_cast<uint8_t *>(record), sizeof(LegacyConfigStorageRecordV1));
  file.close();
  return read == sizeof(LegacyConfigStorageRecordV1) && record->magic == CONFIG_STORAGE_MAGIC &&
         record->version == 1 && record->settings.magic == SETTINGS_MAGIC &&
         record->setup_password.magic == SETUP_PASSWORD_MAGIC && record->checksum == record_checksum(*record);
}

bool read_legacy_config_file_v2(const char *path, LegacyConfigStorageRecordV2 *record) {
  File file = LittleFS.open(path, "r");
  if (!file || file.size() != sizeof(LegacyConfigStorageRecordV2))
    return false;
  const size_t read = file.read(reinterpret_cast<uint8_t *>(record), sizeof(LegacyConfigStorageRecordV2));
  file.close();
  return read == sizeof(LegacyConfigStorageRecordV2) && record->magic == CONFIG_STORAGE_MAGIC &&
         record->version == 2 && record->settings.magic == SETTINGS_MAGIC &&
         record->setup_password.magic == SETUP_PASSWORD_MAGIC && record->checksum == record_checksum(*record);
}

bool read_history_file(const char *path, HistoryStorageRecord *record) {
  File file = LittleFS.open(path, "r");
  if (!file || file.size() != sizeof(HistoryStorageRecord))
    return false;
  const size_t read = file.read(reinterpret_cast<uint8_t *>(record), sizeof(HistoryStorageRecord));
  file.close();
  return read == sizeof(HistoryStorageRecord) && record->magic == HISTORY_STORAGE_MAGIC &&
         record->version == HISTORY_STORAGE_VERSION && record->history.magic == HISTORY_MAGIC &&
         record->checksum == record_checksum(*record);
}
#endif

}  // namespace

void P1Setup::setup() {
  this->login_realm_ready_ = this->rotate_login_realm_();
  if (!this->login_realm_ready_)
    ESP_LOGE("p1_setup", "Could not initialize the setup login realm");

  memset(&this->settings_, 0, sizeof(this->settings_));
  this->settings_.magic = SETTINGS_MAGIC;
  this->settings_.mqtt_port = 1883;
  this->history_enabled_ = false;
#if defined(USE_ESP8266) || (defined(USE_ESP32) && defined(USE_ARDUINO))
  // Never implicitly format an initialized filesystem. A format is allowed only
  // once, when the persistent marker says this is the first installation.
  this->littlefs_initialized_preference_ =
      global_preferences->make_preference<uint8_t>(LITTLEFS_INITIALIZED_PREFERENCE_KEY, true);
  this->littlefs_initialized_ = 0;
  this->littlefs_initialized_preference_.load(&this->littlefs_initialized_);
#if defined(USE_ESP8266)
  // ESP8266 Arduino 3.1.x exposes auto-format through FSConfig rather than
  // through begin(bool).
  LittleFS.setConfig(LittleFSConfig(false));
  this->history_filesystem_ready_ = LittleFS.begin();
#else
  this->history_filesystem_ready_ = LittleFS.begin(false);
#endif
  if (!this->history_filesystem_ready_ && this->littlefs_initialized_ == 0) {
    ESP_LOGW("p1_setup", "LittleFS mount failed on first installation; formatting partition once");
#if defined(USE_ESP8266)
    this->history_filesystem_ready_ = LittleFS.format() && LittleFS.begin();
#else
    this->history_filesystem_ready_ = LittleFS.format() && LittleFS.begin(false);
#endif
  }
  if (!this->history_filesystem_ready_)
    ESP_LOGE("p1_setup", "LittleFS unavailable; preserving filesystem and disabling file storage");
  else if (this->littlefs_initialized_ == 0) {
    this->littlefs_initialized_ = 1;
    if (!this->littlefs_initialized_preference_.save(&this->littlefs_initialized_) ||
        !global_preferences->sync())
      ESP_LOGW("p1_setup", "Could not persist LittleFS initialization marker");
  }
#endif
  this->reset_history_state_();
  this->setup_password_ = this->initial_setup_password_;
  if (wifi::global_wifi_component != nullptr) {
    wifi::WiFiAP access_point = wifi::global_wifi_component->get_ap();
    access_point.set_password(this->setup_password_);
    wifi::global_wifi_component->set_ap(access_point);
  }

  this->mqtt_enabled_ = false;
  this->homewizard_enabled_ = false;
  this->api_enabled_ = true;

  const bool config_loaded = this->load_config_storage_();
  if (!config_loaded)
    this->save_config_storage_();
  // Do not read the relatively large history record from the ESP8266 setup stack.
  // Defer it until loop(), after the framework has completed component setup.
  this->history_load_pending_ = this->history_enabled_;

  // Apply the configuration record before the AP starts accepting clients.
  if (wifi::global_wifi_component != nullptr) {
    wifi::WiFiAP access_point = wifi::global_wifi_component->get_ap();
    access_point.set_password(this->setup_password_);
    wifi::global_wifi_component->set_ap(access_point);
  }

  this->apply_settings_();
  this->apply_api_settings_();

  this->web_server_base_ = web_server_base::global_web_server_base;
  if (this->web_server_base_ != nullptr) {
    this->web_server_base_->add_handler_without_auth(&this->captive_probe_);
    this->web_server_base_->add_handler_without_auth(&this->ap_setup_);
    this->web_server_base_->add_handler_without_auth(this);
  } else {
    ESP_LOGE("p1_setup", "Shared web server is unavailable");
  }
}

void P1Setup::loop() {
  if (this->history_load_pending_) {
    this->history_load_pending_ = false;
    if (!this->load_history_storage_()) {
      memset(&this->history_, 0, sizeof(this->history_));
      this->history_.magic = HISTORY_MAGIC;
    }
  }
  this->update_history_();
  const bool ap_active = wifi::global_wifi_component != nullptr && wifi::global_wifi_component->is_ap_active();
  if (ap_active && !this->dns_active_) {
    this->dns_server_.setErrorReplyCode(DNSReplyCode::NoError);
    this->dns_active_ = this->dns_server_.start(53, "*", WiFi.softAPIP());
  } else if (!ap_active && this->dns_active_) {
    this->dns_server_.stop();
    this->dns_active_ = false;
  }
  if (this->dns_active_)
    this->dns_server_.processNextRequest();

  this->update_homewizard_mdns_();

#ifdef USE_API
  if (!this->api_enabled_ && api::global_api_server != nullptr) {
    api::global_api_server->set_reboot_timeout(0);
    for (auto &client : api::global_api_server->active_clients()) {
      client->on_fatal_error();
    }
  }
#endif

  if (this->reboot_at_ != 0 && static_cast<int32_t>(millis() - this->reboot_at_) >= 0) {
    this->reboot_at_ = 0;
    App.reboot();
  }
}

bool P1Setup::load_config_storage_() {
#if defined(USE_ESP8266) || (defined(USE_ESP32) && defined(USE_ARDUINO))
  if (!this->history_filesystem_ready_)
    return false;
  // Static allocation avoids placing large structures on the limited ESP8266 stack.
  static ConfigStorageRecord record;
  memset(&record, 0, sizeof(record));
  if (!read_config_file(CONFIG_STORAGE_FILE, &record)) {
    memset(&record, 0, sizeof(record));
    if (!read_config_file(CONFIG_STORAGE_BACKUP_FILE, &record)) {
      static LegacyConfigStorageRecordV2 legacy_v2;
      memset(&legacy_v2, 0, sizeof(legacy_v2));
      if (read_legacy_config_file_v2(CONFIG_STORAGE_FILE, &legacy_v2) ||
          read_legacy_config_file_v2(CONFIG_STORAGE_BACKUP_FILE, &legacy_v2)) {
        this->settings_ = legacy_v2.settings;
        this->setup_password_ = legacy_v2.setup_password.password;
        this->mqtt_enabled_ = legacy_v2.mqtt_enabled != 0;
        this->history_enabled_ = legacy_v2.history_enabled != 0;
        this->homewizard_enabled_ = legacy_v2.homewizard_enabled != 0;
        this->api_enabled_ = true;
        this->save_config_storage_();
        ESP_LOGW("p1_setup", "Migrated v2 configuration; ESPHome API is enabled by default");
        return true;
      }
      static LegacyConfigStorageRecordV1 legacy_v1;
      memset(&legacy_v1, 0, sizeof(legacy_v1));
      if (read_legacy_config_file_v1(CONFIG_STORAGE_FILE, &legacy_v1) ||
          read_legacy_config_file_v1(CONFIG_STORAGE_BACKUP_FILE, &legacy_v1)) {
        this->settings_ = legacy_v1.settings;
        this->setup_password_ = legacy_v1.setup_password.password;
        this->mqtt_enabled_ = legacy_v1.mqtt_enabled != 0;
        this->history_enabled_ = legacy_v1.history_enabled != 0;
        this->homewizard_enabled_ = false;
        this->api_enabled_ = true;
        this->save_config_storage_();
        ESP_LOGW("p1_setup", "Migrated v1 configuration; ESPHome API is enabled by default");
        return true;
      }
      return false;
    }
    ESP_LOGW("p1_setup", "Primary configuration is invalid; using backup configuration");
  }
  this->settings_ = record.settings;
  this->setup_password_ = record.setup_password.password;
  this->mqtt_enabled_ = record.mqtt_enabled != 0;
  this->history_enabled_ = record.history_enabled != 0;
  this->homewizard_enabled_ = record.homewizard_enabled != 0;
  this->api_enabled_ = record.api_enabled != 0;
  return true;
#else
  return false;
#endif
}

bool P1Setup::save_config_storage_() {
#if defined(USE_ESP8266) || (defined(USE_ESP32) && defined(USE_ARDUINO))
  if (this->history_filesystem_ready_) {
    // Static allocations avoid placing large structures on the limited ESP8266 stack.
    static ConfigStorageRecord record;
    static ConfigStorageRecord verified;
    memset(&record, 0, sizeof(record));
    memset(&verified, 0, sizeof(verified));
    record.magic = CONFIG_STORAGE_MAGIC;
    record.version = CONFIG_STORAGE_VERSION;
    record.sequence = 1;
    record.settings = this->settings_;
    record.setup_password.magic = SETUP_PASSWORD_MAGIC;
    strncpy(record.setup_password.password, this->setup_password_.c_str(),
            sizeof(record.setup_password.password) - 1);
    record.mqtt_enabled = this->mqtt_enabled_ ? 1 : 0;
    record.history_enabled = this->history_enabled_ ? 1 : 0;
    record.homewizard_enabled = this->homewizard_enabled_ ? 1 : 0;
    record.api_enabled = this->api_enabled_ ? 1 : 0;
    record.checksum = record_checksum(record);
    File file = LittleFS.open(CONFIG_STORAGE_TEMP_FILE, "w");
    if (file) {
      const size_t written = file.write(reinterpret_cast<const uint8_t *>(&record), sizeof(record));
      file.flush();
      file.close();
      if (written == sizeof(record) && read_config_file(CONFIG_STORAGE_TEMP_FILE, &verified) &&
          verified.sequence == record.sequence) {
        bool primary_moved = false;
        if (LittleFS.exists(CONFIG_STORAGE_FILE)) {
          if (LittleFS.exists(CONFIG_STORAGE_BACKUP_FILE) &&
              !LittleFS.remove(CONFIG_STORAGE_BACKUP_FILE)) {
            ESP_LOGE("p1_setup", "Could not remove old configuration backup");
            LittleFS.remove(CONFIG_STORAGE_TEMP_FILE);
            return false;
          }
          if (!LittleFS.rename(CONFIG_STORAGE_FILE, CONFIG_STORAGE_BACKUP_FILE)) {
            ESP_LOGE("p1_setup", "Could not move configuration to backup");
            LittleFS.remove(CONFIG_STORAGE_TEMP_FILE);
            return false;
          }
          primary_moved = true;
        }
        if (LittleFS.rename(CONFIG_STORAGE_TEMP_FILE, CONFIG_STORAGE_FILE) &&
            read_config_file(CONFIG_STORAGE_FILE, &verified) && verified.sequence == record.sequence) {
          return true;
        }
        ESP_LOGE("p1_setup", "Could not promote or verify new configuration");
        LittleFS.remove(CONFIG_STORAGE_TEMP_FILE);
        if (primary_moved) {
          LittleFS.remove(CONFIG_STORAGE_FILE);
          if (!LittleFS.rename(CONFIG_STORAGE_BACKUP_FILE, CONFIG_STORAGE_FILE))
            ESP_LOGE("p1_setup", "Could not restore configuration backup");
        }
      }
    } else {
      ESP_LOGE("p1_setup", "Could not open temporary configuration file");
    }
    ESP_LOGE("p1_setup", "Could not write or verify configuration LittleFS storage");
  }
#endif
  ESP_LOGE("p1_setup", "Configuration LittleFS storage is unavailable; settings were not saved");
  return false;
}

bool P1Setup::load_history_storage_() {
#if defined(USE_ESP8266) || (defined(USE_ESP32) && defined(USE_ARDUINO))
  if (!this->history_filesystem_ready_)
    return false;
  // Static allocation avoids placing large structures on the limited ESP8266 stack.
  static HistoryStorageRecord candidate;
  memset(&candidate, 0, sizeof(candidate));
  const bool valid_a = read_history_file(HISTORY_STORAGE_FILE_A, &candidate);
  uint32_t selected_sequence = 0;
  uint8_t selected_slot = 0;
  if (valid_a) {
    this->history_ = candidate.history;
    selected_sequence = candidate.sequence;
  }
  memset(&candidate, 0, sizeof(candidate));
  const bool valid_b = read_history_file(HISTORY_STORAGE_FILE_B, &candidate);
  if (valid_b && (!valid_a || candidate.sequence > selected_sequence)) {
    this->history_ = candidate.history;
    selected_sequence = candidate.sequence;
    selected_slot = 1;
  }
  if (!valid_a && !valid_b)
    return false;
  this->history_sequence_ = selected_sequence;
  this->history_slot_ = selected_slot;
  this->history_slot_valid_ = true;
  return true;
#else
  return false;
#endif
}

bool P1Setup::save_history_storage_() {
#if defined(USE_ESP8266) || (defined(USE_ESP32) && defined(USE_ARDUINO))
  if (this->history_filesystem_ready_) {
    // Static allocations avoid placing large structures on the limited ESP8266 stack.
    static HistoryStorageRecord record;
    static HistoryStorageRecord verified;
    memset(&record, 0, sizeof(record));
    memset(&verified, 0, sizeof(verified));
    record.magic = HISTORY_STORAGE_MAGIC;
    record.version = HISTORY_STORAGE_VERSION;
    record.sequence = this->history_sequence_ + 1;
    record.history = this->history_;
    record.checksum = record_checksum(record);
    const uint8_t target_slot = this->history_slot_valid_ ? static_cast<uint8_t>(this->history_slot_ ^ 1U) : 0;
    const char *path = target_slot == 0 ? HISTORY_STORAGE_FILE_A : HISTORY_STORAGE_FILE_B;
    File file = LittleFS.open(path, "w");
    if (file) {
      const size_t written = file.write(reinterpret_cast<const uint8_t *>(&record), sizeof(record));
      file.flush();
      file.close();
      if (written == sizeof(record) && read_history_file(path, &verified) &&
          verified.sequence == record.sequence) {
        this->history_sequence_ = record.sequence;
        this->history_slot_ = target_slot;
        this->history_slot_valid_ = true;
        return true;
      }
    }
    ESP_LOGE("p1_setup", "Could not write or verify hourly history storage");
  }
#endif
  ESP_LOGE("p1_setup", "LittleFS history storage is unavailable; history was not saved");
  return false;
}

bool P1Setup::save_history_(bool force) {
  if (!this->history_enabled_)
    return true;
  const uint32_t now = millis();
  if (!force && this->history_retry_at_ != 0 &&
      static_cast<int32_t>(now - this->history_retry_at_) < 0)
    return false;
  const bool saved = this->save_history_storage_();
  if (saved) {
    this->last_history_save_ = now;
    this->history_dirty_ = false;
    this->history_save_failures_ = 0;
    this->history_retry_at_ = 0;
  } else {
    // Keep the data dirty and back off before retrying. This prevents a
    // permanently failed filesystem from causing a write attempt on every loop.
    if (this->history_save_failures_ < HISTORY_MAX_CONSECUTIVE_FAILURES)
      this->history_save_failures_++;
    if (this->history_save_failures_ >= HISTORY_MAX_CONSECUTIVE_FAILURES) {
      this->history_enabled_ = false;
      this->history_dirty_ = false;
      this->history_retry_at_ = 0;
      ESP_LOGE("p1_setup", "Disabling On-Device History after %u consecutive checkpoint failures",
               this->history_save_failures_);
      if (!this->save_config_storage_())
        ESP_LOGE("p1_setup", "Could not persist automatic history disable");
    } else {
      uint32_t delay = HISTORY_RETRY_BASE_DELAY_MS << (this->history_save_failures_ - 1);
      if (delay > HISTORY_RETRY_MAX_DELAY_MS)
        delay = HISTORY_RETRY_MAX_DELAY_MS;
      this->history_retry_at_ = now + delay;
      ESP_LOGW("p1_setup", "History checkpoint failed (%u/%u); retrying in %lu seconds",
               this->history_save_failures_, HISTORY_MAX_CONSECUTIVE_FAILURES, delay / 1000UL);
    }
  }
  return saved;
}

void P1Setup::update_history_() {
  if (!this->history_enabled_)
    return;
  if (this->last_history_sample_ != 0 && millis() - this->last_history_sample_ < 10000UL)
    return;
  this->last_history_sample_ = millis();

  auto *timestamp_sensor = find_text_sensor("DSMR Timestamp");
  auto *power_consumed = find_sensor("Power Consumed");
  auto *power_returned = find_sensor("Power Returned");
  auto *gas_consumed = find_sensor("Gas Consumed");
  if (timestamp_sensor == nullptr || !timestamp_sensor->has_state() || power_consumed == nullptr ||
      power_returned == nullptr || gas_consumed == nullptr || !power_consumed->has_state() ||
      !power_returned->has_state() || !gas_consumed->has_state())
    return;

  uint32_t day_key;
  uint8_t hour;
  if (!parse_timestamp(timestamp_sensor->get_state(), &day_key, &hour))
    return;
  const uint32_t hour_key = day_key * 24UL + hour;
  const size_t index = hour_key % 72UL;
  auto &bucket = this->history_.buckets[index];
  bool new_bucket = !bucket.valid || bucket.hour_key != hour_key;
  if (new_bucket) {
    memset(&bucket, 0, sizeof(bucket));
    bucket.hour_key = hour_key;
    bucket.valid = 1;
  }

  const uint32_t now = millis();
  if (this->previous_sample_valid_ && this->previous_sample_hour_key_ == hour_key &&
      now >= this->previous_sample_ms_ && now - this->previous_sample_ms_ <= 120000UL) {
    const float elapsed_hours = (now - this->previous_sample_ms_) / 3600000.0f;
    bucket.energy_consumed_kwh +=
        (this->previous_power_consumed_ + power_consumed->get_state()) * 0.5f * elapsed_hours;
    bucket.energy_returned_kwh +=
        (this->previous_power_returned_ + power_returned->get_state()) * 0.5f * elapsed_hours;
    const float gas_delta = gas_consumed->get_state() - this->previous_gas_consumed_;
    if (gas_delta >= 0.0f && gas_delta < 10.0f)
      bucket.gas_consumed_m3 += gas_delta;
  }
  this->previous_sample_ms_ = now;
  this->previous_sample_hour_key_ = hour_key;
  this->previous_power_consumed_ = power_consumed->get_state();
  this->previous_power_returned_ = power_returned->get_state();
  this->previous_gas_consumed_ = gas_consumed->get_state();
  this->previous_sample_valid_ = true;
  this->history_dirty_ = true;
  // Do not write on every 10-second sample. Checkpoint the active bucket every
  // thirty minutes, and always save when a new hour starts so completed hours
  // and the most recent partial hour survive a reboot.
  const bool retry_due = this->history_retry_at_ == 0 ||
                         static_cast<int32_t>(now - this->history_retry_at_) >= 0;
  if (retry_due && (new_bucket || (this->history_dirty_ &&
                                   (this->last_history_save_ == 0 ||
                                    now - this->last_history_save_ >= HISTORY_CHECKPOINT_INTERVAL_MS))))
    this->save_history_();
}

void P1Setup::append_history_json_(String &json, uint8_t day_offset) const {
  const char *wear_min = "2 years";
  const char *wear_max = "5 years";
  if (this->device_model_.find("ESP32-S3") != std::string::npos) {
    wear_min = "8 years";
    wear_max = "15 years";
  } else if (this->device_model_.find("ESP32") != std::string::npos) {
    wear_min = "4 years";
    wear_max = "8 years";
  }
  uint32_t current_day_key = 0;
  uint32_t latest_hour_key = 0;
  for (const auto &bucket : this->history_.buckets) {
    if (bucket.valid && (current_day_key == 0 || bucket.hour_key > latest_hour_key)) {
      latest_hour_key = bucket.hour_key;
      current_day_key = bucket.hour_key / 24UL;
    }
  }
  uint32_t requested_day = current_day_key;
  for (uint8_t i = 0; i < day_offset && requested_day != 0; i++)
    requested_day = previous_day(requested_day);

  json += F(",\"history\":{\"enabled\":");
  json += this->history_enabled_ ? F("true") : F("false");
  json += F(",\"device_model\":");
  append_json_string(json, this->device_model_.c_str());
  json += F(",\"wear_min\":");
  append_json_string(json, wear_min);
  json += F(",\"wear_max\":");
  append_json_string(json, wear_max);
  json += F(",\"day_offset\":");
  json += day_offset;
  json += F(",\"day_key\":");
  json += requested_day;
  json += F(",\"hours\":[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23]");
  auto append_hourly_series = [&](const __FlashStringHelper *label, float HistoryBucket::*field) {
    json += label;
    for (uint8_t hour = 0; hour < 24; hour++) {
      if (hour > 0)
        json += ',';
      const uint32_t hour_key = requested_day * 24UL + hour;
      const auto &bucket = this->history_.buckets[hour_key % 72UL];
      if (!bucket.valid || bucket.hour_key != hour_key) {
        json += F("null");
      } else {
        json += String(bucket.*field, 6);
      }
    }
  };
  append_hourly_series(F(",\"energy_consumed\":["), &HistoryBucket::energy_consumed_kwh);
  append_hourly_series(F("],\"energy_returned\":["), &HistoryBucket::energy_returned_kwh);
  append_hourly_series(F("],\"gas_consumed\":["), &HistoryBucket::gas_consumed_m3);
  json += F("]}");
}

}  // namespace esphome::p1_setup
