#include "p1_setup_pages.h"
#include "p1_setup_attribution.h"

namespace esphome::p1_setup::web_pages {

const char ATTRIBUTION_FOOTER[] PROGMEM = P1_SETUP_ATTRIBUTION_FOOTER_HEAD ESPHOME_PROJECT_VERSION
    P1_SETUP_ATTRIBUTION_FOOTER_TAIL;

}  // namespace esphome::p1_setup::web_pages
