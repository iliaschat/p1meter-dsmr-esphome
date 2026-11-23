#!/usr/bin/env python3
"""Bundle assets/pages/*.html + assets/css/*.css + assets/js/*.js into C++ PROGMEM strings.

Reads each HTML page file, replaces build markers with inlined CSS/JS, and writes
components/p1_setup/web_assets.h / web_assets.cpp with the resulting strings.

Markers in HTML files:
  <!-- CSS: name -->   → <style>contents of assets/css/name.css</style>
  <!-- JS:  name -->   → <script>contents of assets/js/name.js</script>
"""

from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
OUT_DIR = ROOT / "components" / "p1_setup"
ASSETS_DIR = OUT_DIR / "assets"
OUT_H = OUT_DIR / "web_assets.h"
OUT_CPP = OUT_DIR / "web_assets.cpp"

RAW_STRING_DELIM = "HTML"
ATTRIBUTION_MARKER = "<!-- P1METER_ATTRIBUTION -->"
MAX_EMBEDDED_PAGE_PAYLOAD = 100_000


def minify_css(content: str) -> str:
    """Apply conservative CSS minification for embedded firmware assets."""
    import re

    content = re.sub(r"/\*.*?\*/", "", content, flags=re.DOTALL)
    content = re.sub(r"\s+", " ", content)
    content = re.sub(r"\s*([{}:;,>])\s*", r"\1", content)

    # --- additional compression ---
    # Shorten 6-digit hex to 3: #aabbcc → #abc
    content = re.sub(r"#([0-9a-fA-F])\1([0-9a-fA-F])\2([0-9a-fA-F])\3", r"#\1\2\3", content)
    # Remove trailing semicolons before }
    content = re.sub(r";}", "}", content)
    # Remove zero units: 0px → 0, 0em → 0
    # content = re.sub(r"0(?:px|em|rem|ex|vh|vw|vmin|vmax)", "0", content)

    return content.strip()


def minify_js(content: str) -> str:
    import re
    """Remove comments and non-semantic whitespace outside JS literals."""
    output = []
    state = "code"
    pending_space = False
    index = 0

    while index < len(content):
        char = content[index]
        next_char = content[index + 1] if index + 1 < len(content) else ""

        if state == "code":
            if char == "/" and next_char == "/":
                state = "line_comment"
                index += 2
                continue
            if char == "/" and next_char == "*":
                state = "block_comment"
                index += 2
                continue
            if char in "'\"`":
                state = char
                output.append(char)
                pending_space = False
                index += 1
                continue
            if char.isspace():
                pending_space = True
                index += 1
                continue
            if pending_space and output and output[-1] not in "([{.,;:!?=+-*/%<>|&~":
                output.append(" ")
            pending_space = False
            output.append(char)
            index += 1
            continue

        if state == "line_comment":
            if char == "\n":
                state = "code"
            index += 1
            continue

        if state == "block_comment":
            if char == "*" and next_char == "/":
                state = "code"
                index += 2
            else:
                index += 1
            continue

        output.append(char)
        if char == "\\" and index + 1 < len(content):
            output.append(content[index + 1])
            index += 2
            continue
        if char == state:
            state = "code"
        index += 1
    result = "".join(output).strip()

    # --- additional compression ---
    # Remove space before ), ], }
    result = re.sub(r"\s+([)\]}])", r"\1", result)
    # Remove space after (, [, {
    result = re.sub(r"([(\[{])\s+", r"\1", result)
    # Remove spaces around =, ==, ===, !=, !==, <=, >=, &&, ||, +=, -=, etc.
    result = re.sub(r"\s*(===|!==|==|!=|<=|>=|&&|\|\||\+=|-=|\*=|/=)\s*", r"\1", result)
    # Remove space after keywords that don't need it
    result = re.sub(r"\b(return|typeof|instanceof|in|of|new|delete|void|throw|else|do|case)\s+", r"\1 ", result)
    # Collapse any remaining multiple spaces
    result = re.sub(r" {2,}", " ", result)

    return result.strip()


def load_asset(name: str, subdir: str) -> str:
    """Load an asset file (css or js) by name."""
    path = ASSETS_DIR / subdir / f"{name}.{subdir}"
    content = path.read_text(encoding="utf-8").strip()
    return minify_css(content) if subdir == "css" else minify_js(content)


def bundle_html(page_name: str) -> str:
    """Read an HTML page and inline all CSS/JS markers."""
    html_path = ASSETS_DIR / "pages" / f"{page_name}.html"
    html = html_path.read_text(encoding="utf-8")

    # Replace CSS markers
    import re

    def replace_css(m):
        name = m.group(1)
        css = load_asset(name, "css")
        return f"<style>{css}</style>"

    def replace_js(m):
        name = m.group(1)
        js = load_asset(name, "js")
        return f"<script>{js}</script>"

    html = re.sub(r"<!--\s*CSS:\s*(\w+)\s*-->", replace_css, html)
    html = re.sub(r"<!--\s*JS:\s*(\w+)\s*-->", replace_js, html)

    html = html.strip()
    if ATTRIBUTION_MARKER not in html:
        raise ValueError(f"{html_path} is missing {ATTRIBUTION_MARKER}")
    html = html.replace(ATTRIBUTION_MARKER, "")
    html = re.sub(r"</body>\s*</html>$", "</body></html>", html)
    return html


def cpp_raw_string(content: str) -> str:
    """Wrap content in a C++11 raw string literal R\"HTML(...)HTML\"."""
    return f'R"{RAW_STRING_DELIM}({content}){RAW_STRING_DELIM}"'


def make_var_name(page_name: str) -> str:
    """Convert a page file name to a C++ variable name."""
    return page_name.upper().replace("-", "_") + "_PAGE"


def main():
    # Discover all page HTML files
    page_files = sorted((ASSETS_DIR / "pages").glob("*.html"))

    # Bundle each page
    pages = {}  # var_name -> bundled html
    for html_path in page_files:
        page_name = html_path.stem
        var_name = make_var_name(page_name)
        bundled = bundle_html(page_name)
        pages[var_name] = bundled

    # Sort for deterministic output
    sorted_vars = sorted(pages.keys())

    # Write header
    h_lines = [
        "#pragma once",
        "",
        "#include <Arduino.h>",
        "",
        "namespace esphome::p1_setup::web_pages {",
        "",
    ]
    for var_name in sorted_vars:
        h_lines.append(f"extern const char {var_name}[] PROGMEM;")
    h_lines += [
        "",
        "}  // namespace esphome::p1_setup::web_pages",
        "",
    ]
    OUT_H.write_text("\n".join(h_lines), encoding="utf-8")
    print(f"Wrote {OUT_H} ({len(sorted_vars)} declarations)")

    # Write implementation
    cpp_lines = [
        '#include "web_assets.h"',
        "",
        "namespace esphome::p1_setup::web_pages {",
        "",
    ]
    for var_name in sorted_vars:
        content = pages[var_name]
        raw = cpp_raw_string(content)
        cpp_lines.append(f"const char {var_name}[] PROGMEM = {raw};")
        cpp_lines.append("")
    cpp_lines += [
        "}  // namespace esphome::p1_setup::web_pages",
        "",
    ]
    embedded_payload = sum(len(pages[name]) for name in sorted_vars)
    if embedded_payload > MAX_EMBEDDED_PAGE_PAYLOAD:
        raise ValueError(
            f"Embedded page payload is {embedded_payload} bytes; "
            f"maximum is {MAX_EMBEDDED_PAGE_PAYLOAD} bytes"
        )
    OUT_CPP.write_text("\n".join(cpp_lines), encoding="utf-8")
    print(f"Wrote {OUT_CPP} ({len(sorted_vars)} definitions)")
    print(f"Embedded page payload: {embedded_payload} bytes")
    print("\nDone. You can now delete the old page .cpp files that defined these constants.")


if __name__ == "__main__":
    main()
