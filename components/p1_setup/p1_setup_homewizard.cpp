#include "p1_setup_internal.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"

#if defined(USE_ESP8266)
#include <ESP8266mDNS.h>
#elif defined(USE_ESP32) && defined(USE_ARDUINO)
#include <ESPmDNS.h>
#endif

namespace esphome::p1_setup {
namespace {

#if defined(USE_ESP8266)
::esp8266::MDNSImplementation::MDNSResponder homewizard_mdns;
#endif

sensor::Sensor *homewizard_sensor(const char *name) {
  for (auto *item : App.get_sensors()) {
    if (item != nullptr && item->get_name() == name)
      return item;
  }
  return nullptr;
}

void append_sensor_value(String &json, bool &first, const char *key, const char *sensor_name, float scale = 1.0f) {
  auto *item = homewizard_sensor(sensor_name);
  if (item == nullptr || !item->has_state())
    return;
  if (!first)
    json += ',';
  first = false;
  json += '"';
  json += key;
  json += F("\":");
  json += String(item->get_state() * scale, 3);
}

void append_sensor_sum(String &json, bool &first, const char *key, const char *first_name, const char *second_name) {
  auto *left = homewizard_sensor(first_name);
  auto *right = homewizard_sensor(second_name);
  if ((left == nullptr || !left->has_state()) && (right == nullptr || !right->has_state()))
    return;
  const float value = (left != nullptr && left->has_state() ? left->get_state() : 0.0f) +
                      (right != nullptr && right->has_state() ? right->get_state() : 0.0f);
  if (!first)
    json += ',';
  first = false;
  json += '"';
  json += key;
  json += F("\":");
  json += String(value, 3);
}

void append_net_power(String &json, bool &first) {
  auto *consumed = homewizard_sensor("Power Consumed");
  auto *returned = homewizard_sensor("Power Returned");
  if ((consumed == nullptr || !consumed->has_state()) && (returned == nullptr || !returned->has_state()))
    return;
  const float value = (consumed != nullptr && consumed->has_state() ? consumed->get_state() : 0.0f) -
                      (returned != nullptr && returned->has_state() ? returned->get_state() : 0.0f);
  if (!first)
    json += ',';
  first = false;
  json += F("\"active_power_w\":");
  json += String(value * 1000.0f, 3);
}

void append_wifi_values(String &json, bool &first, const SetupSettings &settings) {
  if (!first)
    json += ',';
  first = false;
  json += F("\"wifi_ssid\":");
  append_json_string(json, settings.wifi_ssid);
  json += F(",\"wifi_strength\":");
  json += String(WiFi.RSSI());
}

String homewizard_serial() {
  char serial[17];
#if defined(USE_ESP8266)
  snprintf(serial, sizeof(serial), "%08lx", static_cast<unsigned long>(ESP.getChipId()));
#elif defined(USE_ESP32) && defined(USE_ARDUINO)
  snprintf(serial, sizeof(serial), "%012llx", static_cast<unsigned long long>(ESP.getEfuseMac()));
#else
  snprintf(serial, sizeof(serial), "p1meter");
#endif
  return String(serial);
}

}  // namespace

bool P1Setup::is_homewizard_route_(const String &url) const {
  return url == HOMEWIZARD_API_PATH || url == HOMEWIZARD_V1_PATH || url == HOMEWIZARD_DATA_PATH;
}

void P1Setup::handle_homewizard_request_(AsyncWebServerRequest *request) {
  const String url = request_url(request);
  if (request->method() != HTTP_GET) {
    request->send(405, "text/plain", "Method not allowed");
    return;
  }
  if (url == HOMEWIZARD_DATA_PATH) {
    this->send_homewizard_data_(request);
  } else {
    this->send_homewizard_info_(request);
  }
}

void P1Setup::send_homewizard_info_(AsyncWebServerRequest *request) {
  String json;
  json.reserve(220);
  json += F("{\"product_name\":\"P1 Meter\",\"product_type\":\"HWE-P1\",\"serial\":");
  append_json_string(json, homewizard_serial().c_str());
  json += F(",\"api_version\":\"1.0\",\"firmware_version\":\"ESPHome P1 meter\"}");
  AsyncWebServerResponse *response = begin_text_response(request, 200, "application/json", json);
  response->addHeader("Cache-Control", "no-store");
  request->send(response);
}

void P1Setup::send_homewizard_data_(AsyncWebServerRequest *request) {
  String json;
  json.reserve(900);
  json += '{';
  bool first = true;
  append_wifi_values(json, first, this->settings_);
  append_sensor_sum(json, first, "total_power_import_kwh", "Energy Consumed Tariff 1", "Energy Consumed Tariff 2");
  append_sensor_sum(json, first, "total_power_export_kwh", "Energy Returned Tariff 1", "Energy Returned Tariff 2");
  append_sensor_value(json, first, "total_power_import_t1_kwh", "Energy Consumed Tariff 1");
  append_sensor_value(json, first, "total_power_import_t2_kwh", "Energy Consumed Tariff 2");
  append_sensor_value(json, first, "total_power_export_t1_kwh", "Energy Returned Tariff 1");
  append_sensor_value(json, first, "total_power_export_t2_kwh", "Energy Returned Tariff 2");
  append_net_power(json, first);
  append_sensor_value(json, first, "active_power_l1_w", "Power Consumed Phase 1", 1000.0f);
  append_sensor_value(json, first, "active_power_l2_w", "Power Consumed Phase 2", 1000.0f);
  append_sensor_value(json, first, "active_power_l3_w", "Power Consumed Phase 3", 1000.0f);
  append_sensor_value(json, first, "active_voltage_l1_v", "Voltage Phase 1");
  append_sensor_value(json, first, "active_voltage_l2_v", "Voltage Phase 2");
  append_sensor_value(json, first, "active_voltage_l3_v", "Voltage Phase 3");
  append_sensor_value(json, first, "active_current_l1_a", "Current Phase 1");
  append_sensor_value(json, first, "active_current_l2_a", "Current Phase 2");
  append_sensor_value(json, first, "active_current_l3_a", "Current Phase 3");
  append_sensor_value(json, first, "total_gas_m3", "Gas Consumed");
  append_sensor_value(json, first, "electricity_failures", "Short Power Outages");
  append_sensor_value(json, first, "electricity_long_failures", "Long Power Outages");
  append_sensor_value(json, first, "electricity_sags_l1", "Short Power Drops Phase 1");
  append_sensor_value(json, first, "electricity_swells_l1", "Short Power Peaks Phase 1");
  json += '}';
  AsyncWebServerResponse *response = begin_text_response(request, 200, "application/json", json);
  response->addHeader("Cache-Control", "no-store");
  request->send(response);
}

void P1Setup::update_homewizard_mdns_() {
#if defined(USE_ESP8266) || (defined(USE_ESP32) && defined(USE_ARDUINO))
  const bool station_connected = wifi::global_wifi_component != nullptr && wifi::global_wifi_component->is_connected();
  if (!this->homewizard_enabled_ || !station_connected) {
    if (this->homewizard_mdns_started_) {
#if defined(USE_ESP8266)
      homewizard_mdns.end();
#else
      MDNS.end();
#endif
      this->homewizard_mdns_started_ = false;
    }
    return;
  }
  if (this->homewizard_mdns_started_) {
#if defined(USE_ESP8266)
    homewizard_mdns.update();
#endif
    return;
  }

#if defined(USE_ESP8266)
  if (!homewizard_mdns.begin("p1-meter")) {
#else
  if (!MDNS.begin("p1-meter")) {
#endif
    ESP_LOGW("p1_setup", "Could not start mDNS for HomeWizard discovery");
    return;
  }
#if defined(USE_ESP8266)
  homewizard_mdns.addService("hwenergy", "tcp", 80);
  homewizard_mdns.addServiceTxt("hwenergy", "tcp", "api_enabled", "1");
  homewizard_mdns.addServiceTxt("hwenergy", "tcp", "path", "/api/v1");
  homewizard_mdns.addServiceTxt("hwenergy", "tcp", "serial", homewizard_serial().c_str());
  homewizard_mdns.addServiceTxt("hwenergy", "tcp", "product_name", "P1 Meter");
  homewizard_mdns.addServiceTxt("hwenergy", "tcp", "product_type", "HWE-P1");
#else
  MDNS.addService("hwenergy", "tcp", 80);
  MDNS.addServiceTxt("hwenergy", "tcp", "api_enabled", "1");
  MDNS.addServiceTxt("hwenergy", "tcp", "path", "/api/v1");
  MDNS.addServiceTxt("hwenergy", "tcp", "serial", homewizard_serial().c_str());
  MDNS.addServiceTxt("hwenergy", "tcp", "product_name", "P1 Meter");
  MDNS.addServiceTxt("hwenergy", "tcp", "product_type", "HWE-P1");
#endif
  this->homewizard_mdns_started_ = true;
  ESP_LOGI("p1_setup", "HomeWizard Local API v1 enabled; discovery advertised via _hwenergy._tcp");
#else
  if (this->homewizard_enabled_)
    ESP_LOGW("p1_setup", "HomeWizard Local API is only supported on ESP8266 and Arduino ESP32 builds");
#endif
}

}  // namespace esphome::p1_setup
