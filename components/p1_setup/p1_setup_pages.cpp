#include "p1_setup_pages.h"

// The firmware attribution footer is not a standalone page asset: it is
// assembled directly into INDEX_PAGE by scripts/embed_assets.py so the
// dashboard stays a single contiguous PROGMEM response, and no other page
// renders it.

namespace esphome::p1_setup::web_pages {}  // namespace esphome::p1_setup::web_pages
