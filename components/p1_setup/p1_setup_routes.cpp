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

void P1Setup::send_password_page_(AsyncWebServerRequest *request) {
  constexpr size_t closing_length = 14;
  const size_t page_length = strlen_P(web_pages::PASSWORD_PAGE) - closing_length;
  const size_t attribution_length = strlen_P(web_pages::ATTRIBUTION_FOOTER);
  const size_t total_length = page_length + attribution_length + closing_length;
#ifdef USE_ESP32
  // web_server_idf has no filler-callback response; stream the same pieces in order.
  AsyncResponseStream *response = request->beginResponseStream("text/html; charset=utf-8");
  response->print(std::string(web_pages::PASSWORD_PAGE, page_length));
  response->print(std::string(web_pages::ATTRIBUTION_FOOTER, attribution_length));
  response->print(std::string(web_pages::PASSWORD_PAGE + page_length, closing_length));
#else
  AsyncWebServerResponse *response = request->beginChunkedResponse(
      "text/html; charset=utf-8",
      [page_length, attribution_length, total_length](uint8_t *buffer, size_t max_length, size_t index) -> size_t {
        size_t written = 0;
        while (written < max_length && index < total_length) {
          size_t available;
          const char *source;
          if (index < page_length) {
            available = page_length - index;
            source = web_pages::PASSWORD_PAGE + index;
          } else if (index < page_length + attribution_length) {
            const size_t footer_index = index - page_length;
            available = attribution_length - footer_index;
            source = web_pages::ATTRIBUTION_FOOTER + footer_index;
          } else {
            const size_t closing_index = index - page_length - attribution_length;
            available = closing_length - closing_index;
            source = web_pages::PASSWORD_PAGE + page_length + closing_index;
          }
          size_t count = max_length - written;
          if (count > available)
            count = available;
          memcpy_P(buffer + written, source, count);
          written += count;
          index += count;
        }
        return written;
      });
#endif
  response->addHeader("Cache-Control", "no-store");
  request->send(response);
}

bool P1Setup::canHandle(AsyncWebServerRequest *request) const {
  const String url = request_url(request);
  if (this->homewizard_enabled_ && this->is_homewizard_route_(url))
    return true;
  if (request->method() == HTTP_GET) {
      return url == "/" || url == "/dashboard" || url == "/configure" || url == "/ota" || url == "/factory-reset" || url == "/reboot" ||
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
      String page = web_pages::render(web_pages::INDEX_PAGE, this->device_name_.c_str());
      this->send_html_(request, page);
    } else if (url == "/dashboard") {
      this->send_dashboard_page_(request);
    } else if (url == "/api/dashboard") {
      this->send_dashboard_data_(request);
    } else if (url == "/configure") {
      this->send_configure_page_(request);
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

void P1Setup::send_dashboard_page_(AsyncWebServerRequest *request) {
  constexpr size_t closing_length = 14;
  const size_t page_length = strlen_P(web_pages::DASHBOARD_PAGE) - closing_length;
  const size_t attribution_length = strlen_P(web_pages::ATTRIBUTION_FOOTER);
#ifdef USE_ESP32
  // Stream the page so the complete dashboard does not need to exist as a heap String.
  AsyncResponseStream *response = request->beginResponseStream("text/html; charset=utf-8");
  response->print(std::string(web_pages::DASHBOARD_PAGE, page_length));
  response->print(std::string(web_pages::ATTRIBUTION_FOOTER, attribution_length));
  response->print(std::string(web_pages::DASHBOARD_PAGE + page_length, closing_length));
#else
  const size_t total_length = page_length + attribution_length + closing_length;
  AsyncWebServerResponse *response = request->beginChunkedResponse(
      "text/html; charset=utf-8",
      [page_length, attribution_length, total_length](uint8_t *buffer, size_t max_length, size_t index) -> size_t {
        size_t written = 0;
        while (written < max_length && index < total_length) {
          size_t available;
          const char *source;
          if (index < page_length) {
            available = page_length - index;
            source = web_pages::DASHBOARD_PAGE + index;
          } else if (index < page_length + attribution_length) {
            const size_t footer_index = index - page_length;
            available = attribution_length - footer_index;
            source = web_pages::ATTRIBUTION_FOOTER + footer_index;
          } else {
            const size_t closing_index = index - page_length - attribution_length;
            available = closing_length - closing_index;
            source = web_pages::DASHBOARD_PAGE + page_length + closing_index;
          }
          size_t count = max_length - written;
          if (count > available)
            count = available;
          memcpy_P(buffer + written, source, count);
          written += count;
          index += count;
        }
        return written;
      });
#endif
  response->addHeader("Cache-Control", "no-store");
  request->send(response);
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
  constexpr size_t closing_length = 14;
  const size_t page_length = strlen_P(web_pages::CONFIGURE_PAGE) - closing_length;
  const size_t attribution_length = strlen_P(web_pages::ATTRIBUTION_FOOTER);
#ifdef USE_ESP32
  AsyncResponseStream *response = request->beginResponseStream("text/html; charset=utf-8");
  response->print(std::string(web_pages::CONFIGURE_PAGE, page_length));
  response->print(std::string(web_pages::ATTRIBUTION_FOOTER, attribution_length));
  response->print(std::string(web_pages::CONFIGURE_PAGE + page_length, closing_length));
#else
  const size_t total_length = page_length + attribution_length + closing_length;
  AsyncWebServerResponse *response = request->beginChunkedResponse(
      "text/html; charset=utf-8",
      [page_length, attribution_length, total_length](uint8_t *buffer, size_t max_length, size_t index) -> size_t {
        size_t written = 0;
        while (written < max_length && index < total_length) {
          size_t available;
          const char *source;
          if (index < page_length) {
            available = page_length - index;
            source = web_pages::CONFIGURE_PAGE + index;
          } else if (index < page_length + attribution_length) {
            const size_t footer_index = index - page_length;
            available = attribution_length - footer_index;
            source = web_pages::ATTRIBUTION_FOOTER + footer_index;
          } else {
            const size_t closing_index = index - page_length - attribution_length;
            available = closing_length - closing_index;
            source = web_pages::CONFIGURE_PAGE + page_length + closing_index;
          }
          size_t count = max_length - written;
          if (count > available)
            count = available;
          memcpy_P(buffer + written, source, count);
          written += count;
          index += count;
        }
        return written;
      });
#endif
  response->addHeader("Cache-Control", "no-store");
  request->send(response);
}
}  // namespace esphome::p1_setup
