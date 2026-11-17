#include "p1_setup_internal.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"

namespace esphome::p1_setup {
namespace {

sensor::Sensor *homewizard_sensor(const char *name) {
  for (auto *item : App.get_sensors()) {
    if (item != nullptr && item->get_name() == name)
      return item;
  }
  return nullptr;
}

text_sensor::TextSensor *homewizard_text_sensor(const char *name) {
  for (auto *item : App.get_text_sensors()) {
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

void append_text_value(String &json, bool &first, const char *key, const char *sensor_name) {
  auto *item = homewizard_text_sensor(sensor_name);
  if (item == nullptr || !item->has_state() || item->get_state().empty())
    return;
  if (!first)
    json += ',';
  first = false;
  json += '"';
  json += key;
  json += F("\":");
  append_json_string(json, item->get_state().c_str());
}

void append_text_number(String &json, bool &first, const char *key, const char *sensor_name) {
  auto *item = homewizard_text_sensor(sensor_name);
  if (item == nullptr || !item->has_state() || item->get_state().empty())
    return;
  const String value = item->get_state().c_str();
  for (unsigned int i = 0; i < value.length(); i++) {
    if (value[i] < '0' || value[i] > '9')
      return;
  }
  if (!first)
    json += ',';
  first = false;
  json += '"';
  json += key;
  json += F("\":");
  json += String(value.toInt());
}

void append_timestamp_number(String &json, bool &first, const char *key, const char *sensor_name) {
  auto *item = homewizard_text_sensor(sensor_name);
  if (item == nullptr || !item->has_state() || item->get_state().length() < 12)
    return;
  const String value = item->get_state().c_str();
  for (uint8_t i = 0; i < 12; i++) {
    if (value[i] < '0' || value[i] > '9')
      return;
  }
  if (!first)
    json += ',';
  first = false;
  json += '"';
  json += key;
  json += F("\":");
  json += value.substring(0, 12);
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
  const int rssi = WiFi.RSSI();
  const int strength = constrain(2 * (rssi + 100), 0, 100);
  json += String(strength);
}

void append_external_gas(String &json, bool &first) {
  auto *gas = homewizard_sensor("Gas Consumed");
  auto *gas_id = homewizard_text_sensor("Gas Meter ID");
  if (gas == nullptr || !gas->has_state() || gas_id == nullptr || !gas_id->has_state() || gas_id->get_state().empty())
    return;

  String timestamp;
  auto *dsmr_timestamp = homewizard_text_sensor("DSMR Timestamp");
  if (dsmr_timestamp != nullptr && dsmr_timestamp->has_state() && dsmr_timestamp->get_state().length() >= 12) {
    const String candidate = dsmr_timestamp->get_state().c_str();
    bool valid = true;
    for (uint8_t i = 0; i < 12; i++) {
      if (candidate[i] < '0' || candidate[i] > '9') {
        valid = false;
        break;
      }
    }
    if (valid)
      timestamp = candidate.substring(0, 12);
  }

  if (!first)
    json += ',';
  first = false;
  json += F("\"total_gas_m3\":");
  json += String(gas->get_state(), 3);
  if (!timestamp.isEmpty()) {
    json += F(",\"gas_timestamp\":");
    json += timestamp;
  }
  json += F(",\"unique_gas_id\":");
  append_json_string(json, gas_id->get_state().c_str());
  json += F(",\"gas_unique_id\":");
  append_json_string(json, gas_id->get_state().c_str());
  json += F(",\"external\":[{\"unique_id\":");
  append_json_string(json, gas_id->get_state().c_str());
  json += F(",\"type\":\"gas_meter\"");
  if (!timestamp.isEmpty()) {
    json += F(",\"timestamp\":");
    json += timestamp;
  }
  json += F(",\"value\":");
  json += String(gas->get_state(), 3);
  json += F(",\"unit\":\"m3\"}]");
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
  json.reserve(1500);
  json += '{';
  bool first = true;
  append_text_value(json, first, "unique_id", "P1 Meter ID");
  append_text_number(json, first, "smr_version", "P1 Version");
  append_text_number(json, first, "active_tariff", "Actual Tariff Group");
  append_wifi_values(json, first, this->settings_);
  append_sensor_sum(json, first, "total_power_import_kwh", "Energy Consumed Tariff 1", "Energy Consumed Tariff 2");
  append_sensor_sum(json, first, "total_power_export_kwh", "Energy Returned Tariff 1", "Energy Returned Tariff 2");
  append_sensor_value(json, first, "total_power_import_t1_kwh", "Energy Consumed Tariff 1");
  append_sensor_value(json, first, "total_power_import_t2_kwh", "Energy Consumed Tariff 2");
  append_sensor_value(json, first, "total_power_import_t3_kwh", "Energy Consumed Tariff 3");
  append_sensor_value(json, first, "total_power_import_t4_kwh", "Energy Consumed Tariff 4");
  append_sensor_value(json, first, "total_power_export_t1_kwh", "Energy Returned Tariff 1");
  append_sensor_value(json, first, "total_power_export_t2_kwh", "Energy Returned Tariff 2");
  append_sensor_value(json, first, "total_power_export_t3_kwh", "Energy Returned Tariff 3");
  append_sensor_value(json, first, "total_power_export_t4_kwh", "Energy Returned Tariff 4");
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
  append_sensor_value(json, first, "active_frequency_hz", "Grid Frequency");
  append_sensor_value(json, first, "electricity_failures", "Short Power Outages");
  append_sensor_value(json, first, "electricity_long_failures", "Long Power Outages");
  append_sensor_value(json, first, "electricity_sags_l1", "Short Power Drops Phase 1");
  append_sensor_value(json, first, "electricity_sags_l2", "Short Power Drops Phase 2");
  append_sensor_value(json, first, "electricity_sags_l3", "Short Power Drops Phase 3");
  append_sensor_value(json, first, "electricity_swells_l1", "Short Power Peaks Phase 1");
  append_sensor_value(json, first, "electricity_swells_l2", "Short Power Peaks Phase 2");
  append_sensor_value(json, first, "electricity_swells_l3", "Short Power Peaks Phase 3");
  append_sensor_value(json, first, "active_power_average_w", "Active Power Average Demand", 1000.0f);
  append_sensor_value(json, first, "montly_power_peak_w", "Monthly Power Peak", 1000.0f);
  append_external_gas(json, first);
  json += '}';
  AsyncWebServerResponse *response = begin_text_response(request, 200, "application/json", json);
  response->addHeader("Cache-Control", "no-store");
  request->send(response);
}

void P1Setup::update_homewizard_mdns_() {
  // HomeWizard discovery is registered through ESPHome's built-in mDNS
  // component in p1meter_common.yaml. ESPHome owns the single responder on
  // ESP8266, so creating another MDNSResponder here is unsafe and causes
  // repeated "Could not start mDNS" warnings. The runtime toggle still
  // controls whether the HomeWizard HTTP routes return data.
}

}  // namespace esphome::p1_setup
