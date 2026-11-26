#pragma once

#include <Arduino.h>

#include "web_assets.h"

namespace esphome::p1_setup::web_pages {

// Substitutes the device name into a page stored in web_assets.h. The
// dashboard attribution footer is part of INDEX_PAGE itself, so no other page
// needs any post-processing to render its footer.
String render(const char *page, const char *device_name);

}  // namespace esphome::p1_setup::web_pages
