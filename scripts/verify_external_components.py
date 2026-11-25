#!/usr/bin/env python3
"""Verify the external_components setup is complete and correct."""
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
COMPONENT = REPO_ROOT / "components" / "p1_setup"

print("=== External components target check ===")
print(f"Target dir: {COMPONENT}")

required = [
    "p1_setup.cpp",
    "p1_setup.h",
    "p1_setup_auth.cpp",
    "p1_setup_device.cpp",
    "p1_setup_handlers.cpp",
    "p1_setup_homewizard.cpp",
    "p1_setup_http.cpp",
    "p1_setup_internal.h",
    "p1_setup_pages.cpp",
    "p1_setup_attribution.h",
    "p1_setup_pages.h",
    "p1_setup_render.cpp",
    "p1_setup_routes.cpp",
    "p1_setup_settings.cpp",
    "p1_setup_validation.cpp",
    "web_assets.cpp",
    "web_assets.h",
]

all_found = True
for f in required:
    p = COMPONENT / f
    status = "OK" if p.exists() else "MISSING"
    if p.exists():
        print(f"  {f}: {status} ({p.stat().st_size} bytes)")
    else:
        print(f"  {f}: {status}")
        all_found = False

# Assets
asset_files = list((COMPONENT / "assets").rglob("*"))
print(f"\nAssets: {len(asset_files)} files in {COMPONENT / 'assets'}")

# Generated build artifacts
for f in ["web_assets.cpp", "web_assets.h"]:
    p = COMPONENT / f
    print(f"  {f}: {p.stat().st_size} bytes")

# Verify the C++ is valid (compile check via preprocessor would be ideal, but we'll do a syntax sanity check)
print("\n=== C++ syntax sanity check ===")
web_assets_h = (COMPONENT / "web_assets.h").read_text()
web_assets_cpp = (COMPONENT / "web_assets.cpp").read_text()
import re

ATTRIBUTION_MARKER = "<!-- P1METER_ATTRIBUTION -->"
checks_passed = all_found

# Check for PROGMEM declarations
var_count = len(re.findall(r'const char \w+_PAGE\[\] PROGMEM', web_assets_cpp))
print(f"  web_assets.cpp: {var_count} PROGMEM constants")
page_count = len(list((COMPONENT / "assets" / "pages").glob("*.html")))
checks_passed = checks_passed and var_count == page_count

# Check for extern declarations
extern_count = len(re.findall(r'extern const char \w+_PAGE\[\] PROGMEM;', web_assets_h))
print(f"  web_assets.h: {extern_count} extern declarations")
checks_passed = checks_passed and extern_count == var_count

source_marker_count = sum(
    path.read_text(encoding="utf-8").count(ATTRIBUTION_MARKER)
    for path in (COMPONENT / "assets" / "pages").glob("*.html")
)
generated_marker_count = web_assets_cpp.count(ATTRIBUTION_MARKER)
generated_pages = re.findall(r'R"HTML\((.*?)\)HTML"', web_assets_cpp, re.DOTALL)
# INDEX_PAGE is deliberately assembled from two raw-string fragments around
# the compile-time attribution footer. The other pages remain single literals.
compact_closing_count = sum(page.endswith("</body></html>") for page in generated_pages)
index_footer_expression = (
    "P1_SETUP_ATTRIBUTION_FOOTER_HEAD ESPHOME_PROJECT_VERSION" in web_assets_cpp
    and "P1_SETUP_ATTRIBUTION_FOOTER_TAIL" in web_assets_cpp
)
print(f"  Attribution markers in source pages: {source_marker_count}")
print(f"  Attribution markers in generated pages: {generated_marker_count}")
print(f"  Generated raw-string fragments ending with compact closing: {compact_closing_count}")
print(f"  Dashboard compile-time attribution footer: {index_footer_expression}")
checks_passed = checks_passed and source_marker_count == var_count and generated_marker_count == 0
checks_passed = checks_passed and compact_closing_count == var_count and index_footer_expression

# Verify script order in configure page (configure.js before configure_field_errors.js)
css_start = web_assets_cpp.find('CONFIGURE_PAGE[]')
css_end = web_assets_cpp.find('FACTORY_RESET_PAGE')
section = web_assets_cpp[css_start:css_end]
i1 = section.find('<script>')
i2 = section.find('<script>', i1 + 1)
block1 = section[i1:i1 + 80]
block2 = section[i2:i2 + 80]
correct = re.search(r'const\s+\$\s*=', block1) is not None and 'document.querySelectorAll' in block2
print(f"  Configure JS order correct: {correct}")
print(f"    1: {block1}")
print(f"    2: {block2}")

# Verify web_assets.cpp compiles standalone (syntax check)
print("\n=== Final status ===")
checks_passed = checks_passed and correct and css_start >= 0 and css_end > css_start
if checks_passed:
    print("ALL CHECKS PASSED — external_components ready.")
else:
    print("SOME CHECKS FAILED — review before committing.")
    raise SystemExit(1)
