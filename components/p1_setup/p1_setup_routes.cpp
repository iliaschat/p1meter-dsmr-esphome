#include <string>

#include "p1_setup_internal.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"

#if defined(USE_ESP8266) || (defined(USE_ESP32) && defined(USE_ARDUINO))
#include <LittleFS.h>
#endif

namespace esphome::p1_setup {

void P1Setup::handle_unset_password_request_(AsyncWebServerRequest *request) {
  const String url = request_url(request);
  if (request->method() == HTTP_GET && (url == "/" || url == "/setup-password")) {
    this->send_password_page_(request);
  } else if (request->method() == HTTP_POST && url == "/api/setup-password") {
    this->save_setup_password_(request);
  } else if (request->method() == HTTP_GET && !url.startsWith("/api/")) {
    request->redirect("/");
  } else {
    request->send(403, "text/plain", "Set the setup password first");
  }
}

void P1Setup::send_html_(AsyncWebServerRequest *request, const String &page) {
  AsyncWebServerResponse *response = begin_text_response(request, 200, "text/html; charset=utf-8", page);
  response->addHeader("Cache-Control", "no-store");
  request->send(response);
}

void P1Setup::send_progmem_page_(AsyncWebServerRequest *request, const char *page) {
  const size_t page_length = strlen_P(page);
#ifdef USE_ESP32
  // web_server_idf has no PROGMEM response helper; copy the page to a string and
  // send it as a regular response. beginResponseStream() uses a small internal
  // buffer (default 1460 bytes) and Print::print() silently drops data that
  // doesn't fit, truncating pages larger than the buffer.
  std::string body(page, page_length);
  AsyncWebServerResponse *response = request->beginResponse(200, "text/html; charset=utf-8", body);
#else
  // Serve the whole page straight out of PROGMEM without copying it to the heap.
  // ESPAsyncWebServer reads from PROGMEM in small chunks as the TCP stack accepts
  // data — no heap allocation, no String, no fragmentation, works for any page size.
  AsyncWebServerResponse *response = request->beginResponse(
      200, "text/html; charset=utf-8", reinterpret_cast<const uint8_t *>(page), page_length);
#endif
  response->addHeader("Cache-Control", "no-store");
  request->send(response);
}

void P1Setup::send_password_page_(AsyncWebServerRequest *request) {
  this->send_progmem_page_(request, web_pages::PASSWORD_PAGE);
}

bool P1Setup::canHandle(AsyncWebServerRequest *request) const {
  const String url = request_url(request);
  if (this->homewizard_enabled_ && this->is_homewizard_route_(url))
    return true;
  if (request->method() == HTTP_GET) {
      return url == "/" || url == "/configure" || url == "/configure-mqtt" || url == "/ota" || url == "/factory-reset" || url == "/reboot" ||
        url == "/setup-password" || url == "/api/dashboard" || url == "/api/settings" || url == "/api/scan";
  }
  return request->method() == HTTP_POST &&
      (url == "/api/settings" || url == "/api/factory-reset" || url == "/api/reboot" || url == "/api/logout" ||
       url == "/api/setup-password" || url == "/api/history-settings");
}

void P1Setup::handleRequest(AsyncWebServerRequest *request) {
  const String url = request_url(request);
  if (this->homewizard_enabled_ && this->is_homewizard_route_(url)) {
    this->handle_homewizard_request_(request);
    return;
  }
  if (this->setup_password_.empty()) {
    this->handle_unset_password_request_(request);
    return;
  }
  if (!this->session_authenticated_(request)) {
    if (request->method() == HTTP_GET && !url.startsWith("/api/")) {
      this->authenticate_page_request_(request);
    } else {
      request->send(401, "text/plain", "Setup session expired");
    }
    return;
  }
  if (request->method() == HTTP_POST && url == "/api/logout") {
    this->handle_logout_(request);
    return;
  }

  if (request->method() == HTTP_GET) {
    if (url == "/") {
      this->send_index_page_(request);
    } else if (url == "/api/dashboard") {
      this->send_dashboard_data_(request);
    } else if (url == "/configure") {
      this->send_configure_page_(request);
    } else if (url == "/configure-mqtt") {
      this->send_configure_mqtt_page_(request);
    } else if (url == "/ota") {
      String page = web_pages::render(web_pages::OTA_PAGE, this->device_name_.c_str());
      this->send_html_(request, page);
    } else if (url == "/factory-reset") {
      String page = web_pages::render(web_pages::FACTORY_RESET_PAGE, this->device_name_.c_str());
      this->send_html_(request, page);
    } else if (url == "/reboot") {
      String page = web_pages::render(web_pages::REBOOT_PAGE, this->device_name_.c_str());
      this->send_html_(request, page);
    } else if (url == "/setup-password") {
      this->send_password_page_(request);
    } else if (url == "/api/settings") {
      this->send_settings_(request);
    } else if (url == "/api/scan") {
      this->send_scan_(request);
    }
    return;
  }

  if (url == "/api/setup-password") {
    this->save_setup_password_(request);
  } else if (url == "/api/history-settings") {
    this->save_history_settings_(request);
  } else if (url == "/api/settings") {
    this->save_settings_(request);
  } else if (url == "/api/reboot") {
    request->send(200, "text/plain", "Rebooting");
    this->schedule_reboot_();
  } else if (url == "/api/factory-reset") {
    if (!global_preferences->reset()) {
      request->send(500, "text/plain", "Could not reset saved settings");
      return;
    }
#if defined(USE_ESP8266) || (defined(USE_ESP32) && defined(USE_ARDUINO))
    if (this->history_filesystem_ready_) {
      LittleFS.remove(CONFIG_STORAGE_FILE);
      LittleFS.remove(CONFIG_STORAGE_TEMP_FILE);
      LittleFS.remove(CONFIG_STORAGE_BACKUP_FILE);
      LittleFS.remove(HISTORY_STORAGE_FILE_A);
      LittleFS.remove(HISTORY_STORAGE_FILE_B);
      LittleFS.remove(OLD_UNIFIED_STORAGE_FILE_A);
      LittleFS.remove(OLD_UNIFIED_STORAGE_FILE_B);
    }
#endif
    request->send(200, "text/plain", "Factory reset complete; rebooting");
    this->schedule_reboot_();
  }
}

void P1Setup::send_dashboard_data_(AsyncWebServerRequest *request) {
  String json;
  json.reserve(7000);
  json += F("{\"sensors\":[");
  bool first = true;
  for (auto *sensor : App.get_sensors()) {
    if (sensor == nullptr || sensor->is_internal())
      continue;
    if (!first)
      json += ',';
    first = false;
    json += F("{\"name\":");
    append_json_string(json, sensor->get_name().c_str());
    json += F(",\"has_state\":");
    json += sensor->has_state() ? F("true") : F("false");
    if (sensor->has_state()) {
      json += F(",\"value\":");
      json += String(sensor->get_state(), 6);
    }
    json += '}';
  }
  json += F("],\"text_sensors\":[");
  first = true;
  for (auto *sensor : App.get_text_sensors()) {
    if (sensor == nullptr || sensor->is_internal())
      continue;
    if (!first)
      json += ',';
    first = false;
    json += F("{\"name\":");
    append_json_string(json, sensor->get_name().c_str());
    json += F(",\"state\":");
    append_json_string(json, sensor->get_state().c_str());
    json += '}';
  }
  json += ']';
  int requested_day = request_arg(request, "day").toInt();
  if (requested_day < 0)
    requested_day = 0;
  if (requested_day > 2)
    requested_day = 2;
  this->append_history_json_(json, static_cast<uint8_t>(requested_day));
  json += F("}");
  AsyncWebServerResponse *response = begin_text_response(request, 200, "application/json", json);
  response->addHeader("Cache-Control", "no-store");
  request->send(response);
}

void P1Setup::send_configure_page_(AsyncWebServerRequest *request) {
  this->send_progmem_page_(request, web_pages::CONFIGURE_PAGE);
}

void P1Setup::send_configure_mqtt_page_(AsyncWebServerRequest *request) {
  this->send_progmem_page_(request, web_pages::CONFIGURE_MQTT_PAGE);
}

void P1Setup::send_index_page_(AsyncWebServerRequest *request) {
  // The dashboard is the only page that carries the compiled-in attribution
  // footer; every other page is served without it.
  this->send_progmem_page_(request, web_pages::INDEX_PAGE);
}
}  // namespace esphome::p1_setup
