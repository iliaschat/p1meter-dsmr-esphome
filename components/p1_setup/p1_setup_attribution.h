#pragma once

#include "esphome/core/defines.h"

#ifndef ESPHOME_PROJECT_VERSION
#define ESPHOME_PROJECT_VERSION "unknown"
#endif

// Keep the attribution markup shared between the standalone footer and the
// dashboard's compile-time, contiguous PROGMEM response.
#define P1_SETUP_ATTRIBUTION_FOOTER_HEAD \
  R"HTML(<footer style="display:flex;flex-wrap:wrap;gap:4px 14px;margin:32px 0 0;padding:14px 0 0;border-top:1px solid #d5dfd8;color:#62736a;font-size:12px;line-height:1.6"><span>Firmware version )HTML"
#define P1_SETUP_ATTRIBUTION_FOOTER_TAIL \
  R"HTML(</span><span>Developed by <a href="https://github.com/iliaschat" target="_blank" rel="noopener noreferrer" style="color:#27734f;text-decoration:underline;text-underline-offset:2px">Ilias Chatzatoglou</a></span><span>Built with <a href="https://esphome.io" target="_blank" rel="noopener noreferrer" style="color:#27734f;text-decoration:underline;text-underline-offset:2px">ESPHome</a></span></footer>)HTML"
