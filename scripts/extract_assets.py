#!/usr/bin/env python3
"""Extract HTML/CSS/JS from existing C++ raw-string pages into assets/.

Reads the inline page definitions from components/p1_setup/*.cpp and writes
separate assets/pages/*.html, assets/css/*.css and assets/js/*.js files.

The HTML files use build markers that embed_assets.py understands:
  <!-- CSS: name -->   replaced with <style>...</style> at build time
  <!-- JS:  name -->   replaced with <script>...</script> at build time
"""

from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parent.parent
CPP_DIR = ROOT / "components" / "p1_setup"
ASSETS_DIR = ROOT / "components" / "p1_setup" / "assets"

PAGES = {
    "p1_setup_page_index.cpp": ("index", "HTML"),
    "p1_setup_page_configure.cpp": ("configure", "HTML"),
    "p1_setup_page_configure_mqtt.cpp": ("configure_mqtt", "HTML"),
    "p1_setup_page_ota.cpp": ("ota", "HTML"),
    "p1_setup_page_password.cpp": ("password", "HTML"),
    "p1_setup_page_reboot.cpp": ("reboot", "HTML"),
    "p1_setup_page_factory_reset.cpp": ("factory_reset", "HTML"),
    "p1_setup_page_sensors_modal.cpp": ("sensors_modal", "HTML"),
}

EXTRA_JS = {
    "p1_setup_page_configure.cpp": ("configure_field_errors", "JS"),
}

SPLIT_PAGES = {
    "index": ("INDEX_PAGE_HEAD", "INDEX_PAGE_TAIL"),
    "configure": ("CONFIGURE_PAGE_HEAD", "CONFIGURE_PAGE_TAIL"),
}


def read_cpp(filename: str) -> str:
    return (CPP_DIR / filename).read_text(encoding="utf-8")


def extract_raw_strings(cpp_content: str):
    """Yield (var_name, content, delimiter) for each raw string block."""
    var_pattern = re.compile(
        r'const\s+char\s+(\w+)\s*\[\]\s*PROGMEM\s*=\s*R"([A-Za-z0-9_]{0,16})\((.*?)\)\2";',
        re.DOTALL,
    )
    for m in var_pattern.finditer(cpp_content):
        yield m.group(1), m.group(3), m.group(2)


def split_html_css_js(html: str, page_name: str):
    """Split HTML into (html_with_markers, css_blocks, js_blocks)."""
    css_blocks = {}
    js_blocks = {}

    style_pattern = re.compile(r"<style>(.*?)</style>", re.DOTALL | re.IGNORECASE)
    css_counter = 0
    for m in style_pattern.finditer(html):
        css_name = f"{page_name}_style{css_counter}" if css_counter > 0 else page_name
        css_blocks[css_name] = m.group(1).strip()
        html = html.replace(m.group(0), f"<!-- CSS: {css_name} -->", 1)
        css_counter += 1

    script_pattern = re.compile(r"<script>(.*?)</script>", re.DOTALL | re.IGNORECASE)
    js_counter = 0
    for m in script_pattern.finditer(html):
        js_name = f"{page_name}_script{js_counter}" if js_counter > 0 else page_name
        js_blocks[js_name] = m.group(1).strip()
        html = html.replace(m.group(0), f"<!-- JS: {js_name} -->", 1)
        js_counter += 1

    return html.strip(), css_blocks, js_blocks


def merge_split_page(cpp_content: str, page_name: str):
    """Merge HEAD and TAIL raw strings for split pages like dashboard."""
    head_var, tail_var = SPLIT_PAGES[page_name]
    parts = {}
    for var_name, content, delim in extract_raw_strings(cpp_content):
        if var_name == head_var:
            parts["head"] = content
        elif var_name == tail_var:
            parts["tail"] = content

    if "head" not in parts or "tail" not in parts:
        print(f"WARNING: could not find both {head_var} and {tail_var}", file=sys.stderr)
        return None, {}, {}

    combined = parts["head"] + parts["tail"]
    return combined, {}, {}


def main():
    (ASSETS_DIR / "pages").mkdir(parents=True, exist_ok=True)
    (ASSETS_DIR / "css").mkdir(parents=True, exist_ok=True)
    (ASSETS_DIR / "js").mkdir(parents=True, exist_ok=True)

    all_css = {}
    all_js = {}

    for cpp_file, (page_name, delim) in PAGES.items():
        cpp_content = read_cpp(cpp_file)

        if page_name in SPLIT_PAGES:
            html, _, _ = merge_split_page(cpp_content, page_name)
            if html is None:
                continue
        else:
            html = None
            for var_name, content, d in extract_raw_strings(cpp_content):
                if var_name.isupper() and ("PAGE" in var_name or "HTML" in var_name):
                    html = content
                    break
            if html is None:
                print(f"WARNING: no page found in {cpp_file}", file=sys.stderr)
                continue

        html_with_markers, css_blocks, js_blocks = split_html_css_js(html, page_name)

        html_path = ASSETS_DIR / "pages" / f"{page_name}.html"
        html_path.write_text(html_with_markers + "\n", encoding="utf-8")
        print(f"Wrote {html_path} ({len(html_with_markers)} chars)")

        for k, v in css_blocks.items():
            key = f"{page_name}_{k}" if k != page_name else page_name
            all_css[key] = v
        for k, v in js_blocks.items():
            key = f"{page_name}_{k}" if k != page_name else page_name
            all_js[key] = v

    for cpp_file, (js_name, delim) in EXTRA_JS.items():
        cpp_content = read_cpp(cpp_file)
        for var_name, content, d in extract_raw_strings(cpp_content):
            if var_name.upper() == js_name.upper():
                all_js[js_name] = content.strip()
                js_path = ASSETS_DIR / "js" / f"{js_name}.js"
                js_path.write_text(content.strip() + "\n", encoding="utf-8")
                print(f"Wrote {js_path} ({len(content)} chars)")
                break

    for name, content in all_css.items():
        css_path = ASSETS_DIR / "css" / f"{name}.css"
        css_path.write_text(content + "\n", encoding="utf-8")
        print(f"Wrote {css_path} ({len(content)} chars)")

    for name, content in all_js.items():
        js_path = ASSETS_DIR / "js" / f"{name}.js"
        if not js_path.exists():
            js_path.write_text(content + "\n", encoding="utf-8")
            print(f"Wrote {js_path} ({len(content)} chars)")

    print("\nExtraction complete. Review assets/ and then run embed_assets.py to generate C++.")


if __name__ == "__main__":
    main()
