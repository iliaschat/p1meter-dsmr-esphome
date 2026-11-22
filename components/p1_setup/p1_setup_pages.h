#pragma once

#include <Arduino.h>

#include "web_assets.h"

namespace esphome::p1_setup::web_pages {

extern const char ATTRIBUTION_FOOTER[] PROGMEM;

String render(const char *page, const char *device_name);

}  // namespace esphome::p1_setup::web_pages
