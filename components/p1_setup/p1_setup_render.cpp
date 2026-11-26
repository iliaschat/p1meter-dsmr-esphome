#include "p1_setup_pages.h"

namespace esphome::p1_setup::web_pages {

namespace {

// The escaped forms are spelled with a hex escape for the leading ampersand so
// the entity text stays readable in the source without being re-encoded.
constexpr char ENTITY_AMPERSAND[] = "\x26" "amp;";
constexpr char ENTITY_LESS_THAN[] = "\x26" "lt;";
constexpr char ENTITY_GREATER_THAN[] = "\x26" "gt;";
constexpr char ENTITY_QUOTE[] = "\x26" "quot;";
constexpr char ENTITY_APOSTROPHE[] = "\x26" "#39;";

const char *escape_character(char character) {
	switch (character) {
		case '&':
			return ENTITY_AMPERSAND;
		case '<':
			return ENTITY_LESS_THAN;
		case '>':
			return ENTITY_GREATER_THAN;
		case '"':
			return ENTITY_QUOTE;
		case '\'':
			return ENTITY_APOSTROPHE;
		default:
			return nullptr;
	}
}

}  // namespace

String render(const char *page, const char *device_name) {
	String escaped_name;
	for (const char *character = device_name; *character != '\0'; character++) {
		const char *entity = escape_character(*character);
		if (entity != nullptr) {
			escaped_name += entity;
		} else {
			escaped_name += *character;
		}
	}
	// The pages are stored as complete HTML documents; only the device name is
	// substituted here. The dashboard attribution footer is compiled into
	// INDEX_PAGE and is deliberately not added to any other page.
	String rendered = FPSTR(page);
	rendered.replace(F("P1 Meter"), escaped_name);
	rendered.replace(F("P1 meter"), escaped_name);
	return rendered;
}

}  // namespace esphome::p1_setup::web_pages
