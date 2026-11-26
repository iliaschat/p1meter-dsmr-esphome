#pragma once

#include "esphome/core/defines.h"

#ifndef ESPHOME_PROJECT_VERSION
#define ESPHOME_PROJECT_VERSION "unknown"
#endif

// Keep the attribution markup shared between the standalone footer and the
// dashboard's compile-time, contiguous PROGMEM response.
#define P1_SETUP_ATTRIBUTION_FOOTER_HEAD \
  R"HTML(<footer><span>Firmware version )HTML"
#define P1_SETUP_ATTRIBUTION_FOOTER_TAIL \
  R"HTML(</span><span>Developed by <a href="https://github.com/iliaschat" target="_blank" rel="noopener noreferrer">Ilias Chatzatoglou</a></span><span>Built with <a href="https://esphome.io" target="_blank" rel="noopener noreferrer">ESPHome</a></span></footer>)HTML"
