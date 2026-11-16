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

void append_integer_sensor(String &json, bool &first, const char *key, const char *sensor_name) {
  auto *item = homewizard_sensor(sensor_name);
  if (item == nullptr || !item->has_state())
    return;
  if (!first)
    json += ',';
  first = false;
  json += '"';
  json += key;
  json += F("\":");
  json += static_cast<long>(item->get_state());
}

void append_import_export_totals(String &json, bool &first) {
  float import_sum = 0.0f;
  bool has_import = false;
  const char *import_names[] = {
      "Energy Consumed Tariff 1", "Energy Consumed Tariff 2",
      "Energy Consumed Tariff 3", "Energy Consumed Tariff 4"
  };
  for (const char *name : import_names) {
    auto *s = homewizard_sensor(name);
    if (s != nullptr && s->has_state()) {
      import_sum += s->get_state();
      has_import = true;
    }
  }
  if (has_import) {
    if (!first)
      json += ',';
    first = false;
    json += F("\"total_power_import_kwh\":");
    json += String(import_sum, 3);
  }

  float export_sum = 0.0f;
  bool has_export = false;
  const char *export_names[] = {
      "Energy Returned Tariff 1", "Energy Returned Tariff 2",
      "Energy Returned Tariff 3", "Energy Returned Tariff 4"
  };
  for (const char *name : export_names) {
    auto *s = homewizard_sensor(name);
    if (s != nullptr && s->has_state()) {
      export_sum += s->get_state();
      has_export = true;
    }
  }
  if (has_export) {
    if (!first)
      json += ',';
    first = false;
    json += F("\"total_power_export_kwh\":");
    json += String(export_sum, 3);
  }
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

void append_phase_power(String &json, bool &first, const char *key, const char *consumed_name, const char *returned_name) {
  auto *consumed = homewizard_sensor(consumed_name);
  auto *returned = homewizard_sensor(returned_name);
  if ((consumed == nullptr || !consumed->has_state()) && (returned == nullptr || !returned->has_state()))
    return;
  const float value = (consumed != nullptr && consumed->has_state() ? consumed->get_state() : 0.0f) -
                      (returned != nullptr && returned->has_state() ? returned->get_state() : 0.0f);
  if (!first)
    json += ',';
  first = false;
  json += '"';
  json += key;
  json += F("\":");
  json += String(value * 1000.0f, 3);
}

void append_wifi_values(String &json, bool &first, const SetupSettings &settings) {
  if (!first)
    json += ',';
  first = false;
  json += F("\"wifi_ssid\":");
  append_json_string(json, settings.wifi_ssid);
  json += F(",\"wifi_strength\":");
  const int8_t rssi = WiFi.RSSI();
  int strength = 0;
  if (rssi <= -100) {
    strength = 0;
  } else if (rssi >= -50) {
    strength = 100;
  } else {
    strength = 2 * (rssi + 100);
  }
  json += strength;
}

void append_smr_version(String &json, bool &first) {
  auto *sensor = homewizard_text_sensor("P1 Version");
  if (sensor == nullptr || !sensor->has_state() || sensor->get_state().empty())
    return;
  const String raw = sensor->get_state().c_str();
  String digits;
  for (size_t i = 0; i < raw.length(); i++) {
    const char c = raw[i];
    if (c >= '0' && c <= '9')
      digits += c;
  }
  if (digits.length() == 1)
    digits += '0';
  if (digits.isEmpty())
    return;
  if (!first)
    json += ',';
  first = false;
  json += F("\"smr_version\":");
  json += digits;
}

void append_meter_identity(String &json, bool &first) {
  auto *sensor = homewizard_text_sensor("P1 Meter ID");
  String model;
  if (sensor != nullptr && sensor->has_state() && !sensor->get_state().empty()) {
    model = sensor->get_state().c_str();
  }
  if (!model.isEmpty()) {
    if (!first)
      json += ',';
    first = false;
    json += F("\"meter_model\":");
    append_json_string(json, model.c_str());
    json += F(",\"unique_id\":");
    append_json_string(json, model.c_str());
  } else {
    const String serial = homewizard_serial();
    if (!first)
      json += ',';
    first = false;
    json += F("\"meter_model\":\"P1 Meter\",\"unique_id\":");
    append_json_string(json, serial.c_str());
  }
}

void append_active_tariff(String &json, bool &first) {
  auto *sensor = homewizard_text_sensor("Actual Tariff Group");
  if (sensor == nullptr || !sensor->has_state())
    return;
  const String raw = sensor->get_state().c_str();
  const int tariff = raw.toInt();
  if (tariff <= 0)
    return;
  if (!first)
    json += ',';
  first = false;
  json += F("\"active_tariff\":");
  json += tariff;
}

void append_gas_metadata(String &json, bool &first) {
  auto *gas = homewizard_sensor("Gas Consumed");
  if (gas == nullptr || !gas->has_state())
    gas = homewizard_sensor("Gas Consumed BE");
  if (gas == nullptr || !gas->has_state())
    return;
  if (!first)
    json += ',';
  first = false;
  json += F("\"total_gas_m3\":");
  json += String(gas->get_state(), 3);

  auto *ts_sensor = homewizard_text_sensor("DSMR Timestamp");
  if (ts_sensor != nullptr && ts_sensor->has_state() && !ts_sensor->get_state().empty()) {
    const String raw = ts_sensor->get_state().c_str();
    String digits;
    for (size_t i = 0; i < raw.length() && digits.length() < 12; i++) {
      const char c = raw[i];
      if (c >= '0' && c <= '9')
        digits += c;
    }
    if (digits.length() == 12) {
      if (!first)
        json += ',';
      first = false;
      json += F("\"gas_timestamp\":");
      json += digits;
    }
  }

  auto *gas_id = homewizard_text_sensor("Gas Equipment ID");
  if (gas_id == nullptr || !gas_id->has_state() || gas_id->get_state().empty())
    gas_id = homewizard_text_sensor("Gas Equipment ID BE");
  if (gas_id == nullptr || !gas_id->has_state() || gas_id->get_state().empty())
    gas_id = homewizard_text_sensor("Gas Meter ID");
  if (gas_id != nullptr && gas_id->has_state() && !gas_id->get_state().empty()) {
    if (!first)
      json += ',';
    first = false;
    json += F("\"gas_unique_id\":");
    append_json_string(json, gas_id->get_state().c_str());
  }
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
  return url == HOMEWIZARD_API_PATH || url == HOMEWIZARD_V1_PATH ||
         url == HOMEWIZARD_DATA_PATH || url == HOMEWIZARD_TELEGRAM_PATH ||
         url == HOMEWIZARD_STATE_PATH;
}

void P1Setup::handle_homewizard_request_(AsyncWebServerRequest *request) {
  const String url = request_url(request);
  if (url == HOMEWIZARD_STATE_PATH && request->method() == HTTP_PUT) {
    this->handle_homewizard_state_put_(request);
    return;
  }
  if (request->method() != HTTP_GET) {
    request->send(405, "text/plain", "Method not allowed");
    return;
  }
  if (url == HOMEWIZARD_DATA_PATH) {
    this->send_homewizard_data_(request);
  } else if (url == HOMEWIZARD_TELEGRAM_PATH) {
    this->send_homewizard_telegram_(request);
  } else if (url == HOMEWIZARD_STATE_PATH) {
    this->send_homewizard_state_(request);
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

void P1Setup::send_homewizard_telegram_(AsyncWebServerRequest *request) {
  auto *telegram_sensor = homewizard_text_sensor("DSMR Telegram");
  if (telegram_sensor == nullptr || !telegram_sensor->has_state() || telegram_sensor->get_state().empty()) {
    request->send(404, "text/plain", "Telegram not available");
    return;
  }
  AsyncWebServerResponse *response =
      begin_text_response(request, 200, "text/plain", telegram_sensor->get_state().c_str());
  response->addHeader("Cache-Control", "no-store");
  request->send(response);
}

void P1Setup::send_homewizard_state_(AsyncWebServerRequest *request) {
  String json;
  json.reserve(128);
  json += F("{\"power_on\":true,\"homewizard_enabled\":true,\"mqtt_enabled\":");
  json += this->mqtt_enabled_ ? "true" : "false";
  json += F(",\"api_enabled\":");
  json += this->api_enabled_ ? "true" : "false";
  json += '}';
  AsyncWebServerResponse *response = begin_text_response(request, 200, "application/json", json);
  response->addHeader("Cache-Control", "no-store");
  request->send(response);
}

void P1Setup::handle_homewizard_state_put_(AsyncWebServerRequest *request) {
  const String body = request_arg(request, "plain");
  auto parse_state_bool = [&](const char *field_name, const String &direct_arg) -> int {
    if (!direct_arg.isEmpty()) {
      if (direct_arg == "true" || direct_arg == "1" || direct_arg == "on")
        return 1;
      if (direct_arg == "false" || direct_arg == "0" || direct_arg == "off")
        return 0;
    }
    if (!body.isEmpty()) {
      int idx = body.indexOf(field_name);
      if (idx >= 0) {
        int colon = body.indexOf(':', idx);
        if (colon >= 0) {
          String sub = body.substring(colon + 1);
          sub.trim();
          if (sub.startsWith("true") || sub.startsWith("1"))
            return 1;
          if (sub.startsWith("false") || sub.startsWith("0"))
            return 0;
        }
      }
    }
    return -1;
  };

  const int api_val = parse_state_bool("api_enabled", request_arg(request, "api_enabled"));
  if (api_val != -1) {
    this->set_api_enabled(api_val == 1);
  }

  const int mqtt_val = parse_state_bool("mqtt_enabled", request_arg(request, "mqtt_enabled"));
  if (mqtt_val != -1) {
    this->set_mqtt_enabled(mqtt_val == 1);
  }

  const int hw_val = parse_state_bool("homewizard_enabled", request_arg(request, "homewizard_enabled"));
  if (hw_val != -1) {
    this->set_homewizard_enabled(hw_val == 1);
  }

  this->send_homewizard_state_(request);
}

void P1Setup::send_homewizard_data_(AsyncWebServerRequest *request) {
  String json;
  json.reserve(1400);
  json += '{';
  bool first = true;
  append_smr_version(json, first);
  append_meter_identity(json, first);
  append_wifi_values(json, first, this->settings_);
  append_import_export_totals(json, first);
  append_sensor_value(json, first, "total_power_import_t1_kwh", "Energy Consumed Tariff 1");
  append_sensor_value(json, first, "total_power_import_t2_kwh", "Energy Consumed Tariff 2");
  append_sensor_value(json, first, "total_power_import_t3_kwh", "Energy Consumed Tariff 3");
  append_sensor_value(json, first, "total_power_import_t4_kwh", "Energy Consumed Tariff 4");
  append_sensor_value(json, first, "total_power_export_t1_kwh", "Energy Returned Tariff 1");
  append_sensor_value(json, first, "total_power_export_t2_kwh", "Energy Returned Tariff 2");
  append_sensor_value(json, first, "total_power_export_t3_kwh", "Energy Returned Tariff 3");
  append_sensor_value(json, first, "total_power_export_t4_kwh", "Energy Returned Tariff 4");
  append_net_power(json, first);
  append_phase_power(json, first, "active_power_l1_w", "Power Consumed Phase 1", "Power Returned Phase 1");
  append_phase_power(json, first, "active_power_l2_w", "Power Consumed Phase 2", "Power Returned Phase 2");
  append_phase_power(json, first, "active_power_l3_w", "Power Consumed Phase 3", "Power Returned Phase 3");
  append_sensor_value(json, first, "active_voltage_l1_v", "Voltage Phase 1");
  append_sensor_value(json, first, "active_voltage_l2_v", "Voltage Phase 2");
  append_sensor_value(json, first, "active_voltage_l3_v", "Voltage Phase 3");
  append_sensor_value(json, first, "active_current_l1_a", "Current Phase 1");
  append_sensor_value(json, first, "active_current_l2_a", "Current Phase 2");
  append_sensor_value(json, first, "active_current_l3_a", "Current Phase 3");
  append_sensor_value(json, first, "active_frequency_hz", "Grid Frequency");
  append_sensor_value(json, first, "apparent_power_w", "Apparent Power Consumed", 1000.0f);
  append_sensor_value(json, first, "reactive_power_var", "Reactive Power Consumed", 1000.0f);
  append_active_tariff(json, first);
  append_gas_metadata(json, first);
  append_sensor_value(json, first, "total_water_m3", "Water Consumed");
  append_integer_sensor(json, first, "electricity_failures", "Short Power Outages");
  append_integer_sensor(json, first, "electricity_long_failures", "Long Power Outages");
  append_integer_sensor(json, first, "electricity_sags_l1", "Short Power Drops Phase 1");
  append_integer_sensor(json, first, "electricity_sags_l2", "Short Power Drops Phase 2");
  append_integer_sensor(json, first, "electricity_sags_l3", "Short Power Drops Phase 3");
  append_integer_sensor(json, first, "electricity_swells_l1", "Short Power Peaks Phase 1");
  append_integer_sensor(json, first, "electricity_swells_l2", "Short Power Peaks Phase 2");
  append_integer_sensor(json, first, "electricity_swells_l3", "Short Power Peaks Phase 3");
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
