#pragma once

#include <array>
#include <string>

#include <DNSServer.h>

#ifdef USE_ESP32
#include "esphome/components/web_server_idf/web_server_idf.h"
#else
#include <ESPAsyncWebServer.h>
#endif

#include "esphome/components/mqtt/mqtt_client.h"
#include "esphome/components/ota/ota_backend_factory.h"
#include "esphome/components/web_server_base/web_server_base.h"
#include "esphome/components/wifi/wifi_component.h"
#include "esphome/core/component.h"
#include "esphome/core/preferences.h"

namespace esphome::p1_setup {

class CaptiveProbeHandler final : public AsyncWebHandler {
 public:
  bool canHandle(AsyncWebServerRequest *request) const override;
  void handleRequest(AsyncWebServerRequest *request) override;
};

class P1Setup;

class APSetupHandler final : public AsyncWebHandler {
 public:
  explicit APSetupHandler(P1Setup *setup) : setup_(setup) {}
  bool canHandle(AsyncWebServerRequest *request) const override;
  bool isRequestHandlerTrivial() const override { return false; }
  void handleRequest(AsyncWebServerRequest *request) override;
  void handleUpload(AsyncWebServerRequest *request, const PlatformString &filename, size_t index, uint8_t *data,
                    size_t len, bool final) override;

 private:
  P1Setup *setup_;
  ota::OTABackendPtr ota_backend_;
  AsyncWebServerRequest *upload_request_{nullptr};
  bool upload_success_{false};
};

struct SetupSettings {
  uint32_t magic;
  char wifi_ssid[33];
  char wifi_password[65];
  char mqtt_broker[65];
  uint16_t mqtt_port;
  char mqtt_username[33];
  char mqtt_password[65];
  uint8_t ip_mode;
  uint8_t static_ip[4];
  uint8_t gateway[4];
  uint8_t subnet[4];
  uint8_t dns[4];
};

struct SetupPassword {
  uint32_t magic;
  char password[65];
};

struct HistoryBucket {
  uint32_t hour_key;
  float energy_consumed_kwh;
  float energy_returned_kwh;
  float gas_consumed_m3;
  uint8_t valid;
};

struct HistorySettings {
  uint32_t magic;
  HistoryBucket buckets[72];
};

struct ConfigStorageRecord {
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

struct HistoryStorageRecord {
  uint32_t magic;
  uint32_t version;
  uint32_t sequence;
  HistorySettings history;
  uint32_t checksum;
};

class P1Setup final : public Component, public AsyncWebHandler {
 public:
  void set_mqtt_client(mqtt::MQTTClientComponent *client) { this->mqtt_client_ = client; }
  void set_login_credentials(const std::string &username, const std::string &initial_password) {
    this->login_username_ = username;
    this->initial_setup_password_ = initial_password;
  }
  bool mqtt_enabled() const { return this->mqtt_enabled_; }
  void set_mqtt_enabled(bool enabled);
  bool homewizard_enabled() const { return this->homewizard_enabled_; }
  void set_device_name(const std::string &device_name) { this->device_name_ = device_name; }
  void set_device_model(const std::string &device_model) { this->device_model_ = device_model; }
  bool history_enabled() const { return this->history_enabled_; }
  float get_setup_priority() const override { return setup_priority::WIFI + 1.0f; }
  void setup() override;
  void loop() override;

  bool canHandle(AsyncWebServerRequest *request) const override;
  bool isRequestHandlerTrivial() const override { return false; }
  void handleRequest(AsyncWebServerRequest *request) override;
  void schedule_reboot_();

 protected:
  friend class APSetupHandler;

  struct WebSession {
    char token[33]{};
    uint32_t last_activity{0};
    bool active{false};
  };

  void apply_settings_();
  void apply_mqtt_settings_();
  void reset_history_state_();
  bool rotate_login_realm_();
  bool create_session_(char token[33]);
  bool extract_session_token_(AsyncWebServerRequest *request, char token[33]) const;
  WebSession *find_active_session_(const char token[33]);
  bool session_authenticated_(AsyncWebServerRequest *request);
  void authenticate_page_request_(AsyncWebServerRequest *request);
  void handle_unset_password_request_(AsyncWebServerRequest *request);
  void save_setup_password_(AsyncWebServerRequest *request);
  void handle_logout_(AsyncWebServerRequest *request);
  void send_html_(AsyncWebServerRequest *request, const String &page);
  void send_dashboard_page_(AsyncWebServerRequest *request);
  void send_dashboard_data_(AsyncWebServerRequest *request);
  void send_history_settings_(AsyncWebServerRequest *request);
  void save_history_settings_(AsyncWebServerRequest *request);
  void update_history_();
  bool save_history_(bool force = false);
  bool load_config_storage_();
  bool save_config_storage_();
  bool load_history_storage_();
  bool save_history_storage_();
  void append_history_json_(String &json, uint8_t day_offset) const;
  void send_configure_page_(AsyncWebServerRequest *request);
  void send_settings_(AsyncWebServerRequest *request);
  void send_scan_(AsyncWebServerRequest *request);
  void save_settings_(AsyncWebServerRequest *request);
  bool is_homewizard_route_(const String &url) const;
  void handle_homewizard_request_(AsyncWebServerRequest *request);
  void send_homewizard_info_(AsyncWebServerRequest *request);
  void send_homewizard_data_(AsyncWebServerRequest *request);
  void update_homewizard_mdns_();

  mqtt::MQTTClientComponent *mqtt_client_{nullptr};
  std::string login_username_;
  std::string initial_setup_password_;
  std::string setup_password_;
  std::array<WebSession, 4> sessions_{};
  char login_realm_[32]{};
  bool login_realm_ready_{false};
  std::string device_name_{"ESPHome Device"};
  std::string device_model_{"Unknown device"};
  web_server_base::WebServerBase *web_server_base_{nullptr};
  SetupSettings settings_{};
  HistorySettings history_{};
  uint32_t history_sequence_{0};
  uint8_t history_slot_{0};
  bool history_slot_valid_{false};
  bool history_filesystem_ready_{false};
  ESPPreferenceObject littlefs_initialized_preference_{};
  uint8_t littlefs_initialized_{0};
  bool history_enabled_{false};
  CaptiveProbeHandler captive_probe_;
  APSetupHandler ap_setup_{this};
  DNSServer dns_server_;
  uint32_t reboot_at_{0};
  bool mqtt_enabled_{false};
  bool homewizard_enabled_{false};
  bool homewizard_mdns_started_{false};
  bool scan_active_{false};
  bool dns_active_{false};
  uint32_t last_history_sample_{0};
  uint32_t last_history_save_{0};
  uint32_t previous_sample_ms_{0};
  uint32_t previous_sample_hour_key_{0};
  float previous_power_consumed_{0.0f};
  float previous_power_returned_{0.0f};
  float previous_gas_consumed_{0.0f};
  bool previous_sample_valid_{false};
  bool history_dirty_{false};
  bool history_load_pending_{false};
  uint8_t history_save_failures_{0};
  uint32_t history_retry_at_{0};
};

}  // namespace esphome::p1_setup
