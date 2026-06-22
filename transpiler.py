import sys
import os
import re

def decompile_html_to_gtscr(html_file_path, output_gtscr_path):
    if not os.path.exists(html_file_path):
        print(f"Error: HTML file '{html_file_path}' not found.")
        return

    base_dir = os.path.dirname(html_file_path)

    with open(html_file_path, "r", encoding="utf-8") as f:
        html_content = f.read()

    # Extract Page Title
    title_match = re.search(r"<title>(.*?)</title>", html_content, re.IGNORECASE)
    page_title = title_match.group(1) if title_match else "Imported Site"

    # Extract CSS (both internal styles and linked style.css if present)
    css_blocks = []
    internal_styles = re.findall(r"<style[^>]*>(.*?)</style>", html_content, re.DOTALL | re.IGNORECASE)
    css_blocks.extend(internal_styles)
    
    css_link_match = re.search(r'<link[^>]+href=["\'](.*?.css)["\']', html_content, re.IGNORECASE)
    if css_link_match:
        css_file = os.path.join(base_dir, css_link_match.group(1))
        if os.path.exists(css_file):
            with open(css_file, "r", encoding="utf-8") as f:
                css_blocks.append(f.read())

    # Extract JavaScript (both internal scripts and linked script.js if present)
    js_blocks = []
    internal_scripts = re.findall(r"<script[^>]*>(.*?)</script>", html_content, re.DOTALL | re.IGNORECASE)
    for src_code in internal_scripts:
        if src_code.strip():
            js_blocks.append(src_code)
            
    js_link_match = re.search(r'<script[^>]+src=["\'](.*?.js)["\']', html_content, re.IGNORECASE)
    if js_link_match:
        js_file = os.path.join(base_dir, js_link_match.group(1))
        if os.path.exists(js_file):
            with open(js_file, "r", encoding="utf-8") as f:
                js_blocks.append(f.read())

    # Extract structural DOM body elements (paragraphs, images, buttons)
    body_match = re.search(r"<body[^>]*>(.*?)</body>", html_content, re.DOTALL | re.IGNORECASE)
    body_content = body_match.group(1) if body_match else html_content

    # Strip script tags and style tags out of the body content for clean reading
    body_content = re.sub(r"<script[^>]*>.*?</script>", "", body_content, flags=re.DOTALL | re.IGNORECASE)
    body_content = re.sub(r"<style[^>]*>.*?</style>", "", body_content, flags=re.DOTALL | re.IGNORECASE)

    # Tokenize elements sequentially to preserve ordering
    elements = re.split(r"(<p[^>]*>.*?</p>|<img[^>]*>|<button[^>]*>.*?</button>)", body_content, flags=re.DOTALL | re.IGNORECASE)

    gtscr_lines = []
    gtscr_lines.append("{pag}")
    gtscr_lines.append(page_title)
    gtscr_lines.append("")

    if css_blocks:
        gtscr_lines.append("{style}")
        for block in css_blocks:
            gtscr_lines.append(block.strip())
        gtscr_lines.append("")

    for elem in elements:
        elem = elem.strip()
        if not elem:
            continue

        if re.match(r"^<p[^>]*>", elem, re.IGNORECASE):
            text_inner = re.sub(r"<[^>]+>", "", elem)  # Extract raw string inside paragraph
            if text_inner.strip():
                gtscr_lines.append("{text}")
                gtscr_lines.append(text_inner.strip())
                gtscr_lines.append("")
                
        elif re.match(r"^<img[^>]*>", elem, re.IGNORECASE):
            attrs_match = re.search(r"<img\s+(.*?)>", elem, re.IGNORECASE)
            if attrs_match:
                attrs = attrs_match.group(1).strip()
                attrs = re.sub(r"\s*/?$", "", attrs) # clean trailing slashes
                gtscr_lines.append("{img}")
                gtscr_lines.append(attrs)
                gtscr_lines.append("")
                
        elif re.match(r"^<button[^>]*>", elem, re.IGNORECASE):
            btn_match = re.search(r"<button\s*([^>]*)\s*>(.*?)</button>", elem, re.DOTALL | re.IGNORECASE)
            if btn_match:
                attrs = btn_match.group(1).strip()
                content = btn_match.group(2).strip()
                gtscr_lines.append("{button}")
                gtscr_lines.append(attrs if attrs else 'id="btn"')
                gtscr_lines.append(content)
                gtscr_lines.append("")

    if js_blocks:
        gtscr_lines.append("{script}")
        for block in js_blocks:
            gtscr_lines.append(block.strip())
        gtscr_lines.append("")

    with open(output_gtscr_path, "w", encoding="utf-8") as f:
        f.write("\n".join(gtscr_lines))

    print(f"[GOTOZH Transpiler] Successfully reversed HTML into: {output_gtscr_path}")

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python transpiler.py {input.html} {output.gtscr}")
    else:
        decompile_html_to_gtscr(sys.argv[1], sys.argv[2])