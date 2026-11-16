#include "p1_setup_internal.h"

#ifdef USE_API
#include "esphome/components/api/api_server.h"
#endif

#if defined(USE_ESP8266) || (defined(USE_ESP32) && defined(USE_ARDUINO))
#include <LittleFS.h>
#endif

namespace esphome::p1_setup {

void P1Setup::save_setup_password_(AsyncWebServerRequest *request) {
  String origin;
  String host;
  if (!request_header(request, "Origin", origin) || !request_header(request, "Host", host) ||
      origin != String("http://") + host) {
    request->send(403, "text/plain", "Invalid request origin");
    return;
  }

  const String password = request_arg(request, "password");
  if (!valid_setup_password(password)) {
    request->send(400, "text/plain", "Use 8 to 63 printable ASCII characters.");
    return;
  }
  const bool first_setup = this->setup_password_.empty();
  this->setup_password_ = password.c_str();
  if (!this->save_config_storage_()) {
    request->send(500, "text/plain", "Could not save the setup password to flash");
    return;
  }
  if (first_setup) {
    request->send(200, "text/plain", "Password saved; rebooting");
    this->schedule_reboot_();
  } else {
    this->login_realm_ready_ = this->rotate_login_realm_();
    request->send(200, "text/plain", "Password saved and applied");
  }
}

void P1Setup::apply_mqtt_settings_() {
  if (this->mqtt_client_ == nullptr)
    return;

  this->mqtt_client_->set_broker_address(this->settings_.mqtt_broker);
  this->mqtt_client_->set_broker_port(this->settings_.mqtt_port == 0 ? 1883 : this->settings_.mqtt_port);
  this->mqtt_client_->set_username(this->settings_.mqtt_username);
  this->mqtt_client_->set_password(this->settings_.mqtt_password);
  this->mqtt_client_->set_enable_on_boot(this->mqtt_enabled_ && this->settings_.mqtt_broker[0] != '\0');
  this->mqtt_client_->set_reboot_timeout(0);
  if (this->mqtt_enabled_) {
    this->mqtt_client_->enable();
  } else {
    this->mqtt_client_->disable();
  }
}

void P1Setup::set_mqtt_enabled(bool enabled) {
  this->mqtt_enabled_ = enabled && this->settings_.mqtt_broker[0] != '\0';
  this->save_config_storage_();
  this->apply_mqtt_settings_();
}

void P1Setup::set_homewizard_enabled(bool enabled) {
  this->homewizard_enabled_ = enabled;
  this->save_config_storage_();
  this->update_homewizard_mdns_();
}

void P1Setup::set_api_enabled(bool enabled) {
  this->api_enabled_ = enabled;
  this->save_config_storage_();
  this->apply_api_settings_();
}

void P1Setup::apply_api_settings_() {
#ifdef USE_API
  if (api::global_api_server != nullptr) {
    if (this->api_enabled_) {
      api::global_api_server->set_reboot_timeout(30UL * 60UL * 1000UL);
    } else {
      api::global_api_server->set_reboot_timeout(0);
      for (auto &client : api::global_api_server->active_clients()) {
        client->on_fatal_error();
      }
    }
  }
#endif
}

void P1Setup::send_settings_(AsyncWebServerRequest *request) {
  String origin;
  String host;
  String fetch_site;
  const bool has_origin = request_header(request, "Origin", origin);
  const bool has_host = request_header(request, "Host", host);
  const bool has_fetch_site = request_header(request, "Sec-Fetch-Site", fetch_site);
  if ((has_origin && (!has_host || origin != String("http://") + host)) ||
      (has_fetch_site && fetch_site == "cross-site") ||
      (wifi::global_wifi_component != nullptr && wifi::global_wifi_component->is_ap_active() &&
       is_softap_client(request) && (!has_host || host != "192.168.4.1"))) {
    request->send(403, "text/plain", "Invalid request origin");
    return;
  }
  String json;
  json.reserve(512);
  json += F("{\"wifi_ssid\":");
  append_json_string(json, this->settings_.wifi_ssid);
  json += F(",\"wifi_password\":");
  append_json_string(json, this->settings_.wifi_password);
  json += F(",\"mqtt_broker\":");
  append_json_string(json, this->settings_.mqtt_broker);
  json += F(",\"mqtt_enabled\":");
  json += this->mqtt_enabled_ ? "true" : "false";
  json += F(",\"homewizard_enabled\":");
  json += this->homewizard_enabled_ ? "true" : "false";
  json += F(",\"api_enabled\":");
  json += this->api_enabled_ ? "true" : "false";
  json += F(",\"mqtt_port\":");
  json += String(this->settings_.mqtt_port == 0 ? 1883 : this->settings_.mqtt_port);
  json += F(",\"mqtt_username\":");
  append_json_string(json, this->settings_.mqtt_username);
  json += F(",\"mqtt_password\":");
  append_json_string(json, this->settings_.mqtt_password);
  json += F(",\"ip_mode\":");
  append_json_string(json, this->settings_.ip_mode == 1 ? "static" : "dhcp");
  json += F(",\"static_ip\":");
  append_json_ip(json, this->settings_.static_ip);
  json += F(",\"gateway\":");
  append_json_ip(json, this->settings_.gateway);
  json += F(",\"subnet\":");
  append_json_ip(json, this->settings_.subnet);
  json += F(",\"dns\":");
  append_json_ip(json, this->settings_.dns);
  json += '}';
  AsyncWebServerResponse *response = begin_text_response(request, 200, "application/json", json);
  response->addHeader("Cache-Control", "no-store");
  request->send(response);
}

void P1Setup::reset_history_state_() {
  this->history_enabled_ = false;
  memset(&this->history_, 0, sizeof(this->history_));
  this->history_.magic = HISTORY_MAGIC;
  this->history_sequence_ = 0;
  this->history_slot_ = 0;
  this->history_slot_valid_ = false;
  this->history_dirty_ = false;
  this->history_save_failures_ = 0;
  this->history_retry_at_ = 0;
}

void P1Setup::save_history_settings_(AsyncWebServerRequest *request) {
  if (!same_origin(request)) {
    request->send(403, "text/plain", "Invalid request origin");
    return;
  }
  const bool enable = request_arg(request, "enabled") == "true";
  if (enable == this->history_enabled_) {
    request->send(200, "text/plain", enable ? "On-Device History is already enabled" :
                                                "On-Device History is already disabled");
    return;
  }
  if (!enable) {
    this->reset_history_state_();
    if (!this->save_config_storage_()) {
      request->send(500, "text/plain", "Could not save the history setting");
      return;
    }
#if defined(USE_ESP8266) || (defined(USE_ESP32) && defined(USE_ARDUINO))
    if (this->history_filesystem_ready_) {
      LittleFS.remove(HISTORY_STORAGE_FILE_A);
      LittleFS.remove(HISTORY_STORAGE_FILE_B);
    }
#endif
    request->send(200, "text/plain", "On-Device History disabled and saved history deleted");
    return;
  }
  // Build and verify the initial history record before advertising history as
  // enabled in the configuration file. This prevents booting into an enabled
  // state with no valid history record if the second write fails.
  this->reset_history_state_();
  if (!this->save_history_storage_()) {
    request->send(500, "text/plain", "Could not save the history setting");
    return;
  }
  this->history_enabled_ = true;
  if (!this->save_config_storage_()) {
    this->history_enabled_ = false;
#if defined(USE_ESP8266) || (defined(USE_ESP32) && defined(USE_ARDUINO))
    if (this->history_filesystem_ready_) {
      LittleFS.remove(HISTORY_STORAGE_FILE_A);
      LittleFS.remove(HISTORY_STORAGE_FILE_B);
    }
#endif
    request->send(500, "text/plain", "Could not save the history setting");
    return;
  }
  request->send(200, "text/plain", "On-Device History enabled");
}

void P1Setup::save_settings_(AsyncWebServerRequest *request) {
  SetupSettings next = this->settings_;
  next.magic = SETTINGS_MAGIC;
  next.mqtt_port = 1883;
  next.ip_mode = 0;

  const String ssid = request_arg(request, "wifi_ssid");
  const String wifi_password = request_arg(request, "wifi_password");
  const String broker = request_arg(request, "mqtt_broker");
  const String port_text = request_arg(request, "mqtt_port");
  const String username = request_arg(request, "mqtt_username");
  const String mqtt_password = request_arg(request, "mqtt_password");
  const bool mqtt_enabled = request_arg(request, "mqtt_enabled") == "on";
  const bool homewizard_enabled = request_arg(request, "homewizard_enabled") == "on";
  const bool api_enabled = request_arg(request, "api_enabled") == "on";
  if (ssid.isEmpty()) {
    send_field_error(request, "wifi_ssid", "Enter a Wi-Fi network name.");
    return;
  }
  if (!copy_value(next.wifi_ssid, sizeof(next.wifi_ssid), ssid)) {
    send_field_error(request, "wifi_ssid", "Wi-Fi network name must be 32 characters or fewer.");
    return;
  }
  if (!copy_value(next.wifi_password, sizeof(next.wifi_password), wifi_password)) {
    send_field_error(request, "wifi_password", "Wi-Fi password must be 64 characters or fewer.");
    return;
  }
  if (!valid_wpa_password(wifi_password)) {
    send_field_error(request, "wifi_password", "Use 8 to 63 printable ASCII characters or 64 hexadecimal digits.");
    return;
  }
  if (!mqtt_enabled) {
    memset(next.mqtt_broker, 0, sizeof(next.mqtt_broker));
    memset(next.mqtt_username, 0, sizeof(next.mqtt_username));
    memset(next.mqtt_password, 0, sizeof(next.mqtt_password));
  } else {
    if (broker.isEmpty()) {
      send_field_error(request, "mqtt_broker", "Enter an MQTT broker host name or IP address.");
      return;
    }
    if (!copy_value(next.mqtt_broker, sizeof(next.mqtt_broker), broker)) {
      send_field_error(request, "mqtt_broker", "MQTT broker host must be 64 characters or fewer.");
      return;
    }
    if (!copy_value(next.mqtt_username, sizeof(next.mqtt_username), username)) {
      send_field_error(request, "mqtt_username", "MQTT username must be 32 characters or fewer.");
      return;
    }
    if (!copy_value(next.mqtt_password, sizeof(next.mqtt_password), mqtt_password)) {
      send_field_error(request, "mqtt_password", "MQTT password must be 64 characters or fewer.");
      return;
    }
    if (!username.isEmpty() && mqtt_password.isEmpty()) {
      send_field_error(request, "mqtt_password", "Enter an MQTT password when a username is provided.");
      return;
    }

    char *end = nullptr;
    const long port = strtol(port_text.c_str(), &end, 10);
    if (port_text.isEmpty()) {
      send_field_error(request, "mqtt_port", "Enter an MQTT port (default is 1883).");
      return;
    }
    if (*end != '\0' || port < 1 || port > 65535) {
      send_field_error(request, "mqtt_port", "Enter a whole number from 1 to 65535.");
      return;
    }
    next.mqtt_port = static_cast<uint16_t>(port);
  }

  const String ip_mode = request_arg(request, "ip_mode");
  if (ip_mode == "static") {
    next.ip_mode = 1;
    if (!parse_ipv4(request_arg(request, "static_ip"), next.static_ip)) {
      send_field_error(request, "static_ip", "Enter a valid device IPv4 address.");
      return;
    }
    if (!parse_ipv4(request_arg(request, "gateway"), next.gateway)) {
      send_field_error(request, "gateway", "Enter a valid gateway IPv4 address.");
      return;
    }
    if (!parse_ipv4(request_arg(request, "subnet"), next.subnet)) {
      send_field_error(request, "subnet", "Enter a valid subnet mask.");
      return;
    }
    const String dns = request_arg(request, "dns");
    memset(next.dns, 0, sizeof(next.dns));
    if (!dns.isEmpty() && !parse_ipv4(dns, next.dns)) {
      send_field_error(request, "dns", "Enter a valid DNS IPv4 address, or leave this blank.");
      return;
    }
  }

  this->settings_ = next;
  this->homewizard_enabled_ = homewizard_enabled;
  this->api_enabled_ = api_enabled;
  if (!this->save_config_storage_()) {
    request->send(500, "text/plain", "Could not save settings to flash");
    return;
  }
  this->apply_settings_();
  this->apply_api_settings_();
  this->update_homewizard_mdns_();
  request->send(200, "text/plain", "Settings saved and applied");
}

void P1Setup::apply_settings_() {
  if (wifi::global_wifi_component != nullptr && this->settings_.wifi_ssid[0] != '\0') {
    wifi::WiFiAP station;
    station.set_ssid(this->settings_.wifi_ssid);
    station.set_password(this->settings_.wifi_password);
    if (this->settings_.ip_mode == 1) {
      wifi::ManualIP manual_ip{
          .static_ip = network::IPAddress(this->settings_.static_ip[0], this->settings_.static_ip[1],
                                          this->settings_.static_ip[2], this->settings_.static_ip[3]),
          .gateway = network::IPAddress(this->settings_.gateway[0], this->settings_.gateway[1],
                                        this->settings_.gateway[2], this->settings_.gateway[3]),
          .subnet = network::IPAddress(this->settings_.subnet[0], this->settings_.subnet[1],
                                       this->settings_.subnet[2], this->settings_.subnet[3]),
          .dns1 = network::IPAddress(this->settings_.dns[0], this->settings_.dns[1],
                                     this->settings_.dns[2], this->settings_.dns[3]),
          .dns2 = network::IPAddress(),
      };
      station.set_manual_ip(optional<wifi::ManualIP>(manual_ip));
    }
    wifi::global_wifi_component->set_sta(station);
  }
  this->apply_mqtt_settings_();
}

void P1Setup::schedule_reboot_() {
  if (this->reboot_at_ != 0)
    return;
  if (!this->save_history_(true))
    ESP_LOGW("p1_setup", "Could not checkpoint history before reboot");
  this->reboot_at_ = millis() + 5000;
}

}  // namespace esphome::p1_setup
