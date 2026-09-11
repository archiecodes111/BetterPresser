import os
import subprocess
import re

CSS_STYLE = """
<style>
@import url('https://fonts.googleapis.com/css2?family=Inter:wght@300;400;500;600;700;800&family=JetBrains+Mono:wght@400;600&display=swap');

@page {
    size: A4;
    margin: 20mm 18mm 20mm 18mm;
}

body {
    font-family: 'Inter', -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
    color: #1a202c;
    line-height: 1.6;
    font-size: 10.5pt;
    background-color: #ffffff;
    margin: 0;
    padding: 0;
}

h1 {
    font-size: 22pt;
    font-weight: 800;
    color: #0f172a;
    border-bottom: 2.5px solid #0284c7;
    padding-bottom: 8px;
    margin-top: 0;
    margin-bottom: 12px;
}

h2 {
    font-size: 14pt;
    font-weight: 700;
    color: #1e293b;
    border-bottom: 1px solid #e2e8f0;
    padding-bottom: 5px;
    margin-top: 24px;
    margin-bottom: 10px;
    page-break-after: avoid;
}

h3 {
    font-size: 11.5pt;
    font-weight: 600;
    color: #334155;
    margin-top: 16px;
    margin-bottom: 6px;
    page-break-after: avoid;
}

p {
    margin-top: 0;
    margin-bottom: 10px;
}

strong {
    font-weight: 600;
    color: #0f172a;
}

ul, ol {
    margin-top: 4px;
    margin-bottom: 12px;
    padding-left: 22px;
}

li {
    margin-bottom: 4px;
}

table {
    width: 100%;
    border-collapse: collapse;
    margin-top: 12px;
    margin-bottom: 16px;
    font-size: 9.5pt;
    page-break-inside: avoid;
}

th {
    background-color: #0f172a;
    color: #ffffff;
    font-weight: 600;
    text-align: left;
    padding: 7px 10px;
    border: 1px solid #0f172a;
}

td {
    padding: 6px 10px;
    border: 1px solid #cbd5e1;
}

tr:nth-child(even) {
    background-color: #f8fafc;
}

pre, code {
    font-family: 'JetBrains Mono', Consolas, Monaco, monospace;
}

code {
    background-color: #f1f5f9;
    color: #0284c7;
    padding: 2px 5px;
    border-radius: 4px;
    font-size: 9pt;
}

pre {
    background-color: #0f172a;
    color: #f8fafc;
    padding: 12px 14px;
    border-radius: 6px;
    font-size: 8.5pt;
    line-height: 1.45;
    overflow-x: auto;
    margin-top: 10px;
    margin-bottom: 14px;
    page-break-inside: avoid;
}

pre code {
    background-color: transparent;
    color: inherit;
    padding: 0;
}

blockquote {
    border-left: 4px solid #0284c7;
    background-color: #f0f9ff;
    padding: 8px 14px;
    margin: 12px 0;
    border-radius: 0 4px 4px 0;
    color: #0369a1;
}

.header-meta {
    background: #f8fafc;
    border: 1px solid #e2e8f0;
    border-radius: 6px;
    padding: 10px 14px;
    margin-bottom: 20px;
    font-size: 9.5pt;
}

.badge-pass {
    background-color: #16a34a;
    color: white;
    font-weight: bold;
    padding: 2px 6px;
    border-radius: 3px;
    font-size: 8pt;
}
</style>
"""

def md_to_html(md_content, title="Document"):
    # Basic markdown parsing
    lines = md_content.split('\n')
    html_lines = []
    in_code_block = False
    in_table = False
    table_has_header = False
    
    for line in lines:
        if line.startswith('```'):
            if in_code_block:
                html_lines.append('</code></pre>')
                in_code_block = False
            else:
                html_lines.append('<pre><code>')
                in_code_block = True
            continue
            
        if in_code_block:
            html_lines.append(line.replace('&', '&amp;').replace('<', '&lt;').replace('>', '&gt;'))
            continue

        # Handle tables
        if line.strip().startswith('|') and line.strip().endswith('|'):
            cells = [c.strip() for c in line.strip()[1:-1].split('|')]
            if all(set(c).issubset({'-', ':', ' '}) for c in cells):
                table_has_header = True
                continue
            
            if not in_table:
                html_lines.append('<table>')
                in_table = True
                table_has_header = False
                
            tag = 'th' if not table_has_header else 'td'
            row_html = '<tr>' + ''.join(f'<{tag}>{format_inline(c)}</{tag}>' for c in cells) + '</tr>'
            html_lines.append(row_html)
            continue
        else:
            if in_table:
                html_lines.append('</table>')
                in_table = False

        # Headers
        if line.startswith('# '):
            html_lines.append(f'<h1>{format_inline(line[2:])}</h1>')
        elif line.startswith('## '):
            html_lines.append(f'<h2>{format_inline(line[3:])}</h2>')
        elif line.startswith('### '):
            html_lines.append(f'<h3>{format_inline(line[4:])}</h3>')
        elif line.startswith('---'):
            html_lines.append('<hr style="border: none; border-top: 1px solid #e2e8f0; margin: 18px 0;" />')
        elif line.startswith('- ') or line.startswith('* '):
            html_lines.append(f'<li>{format_inline(line[2:])}</li>')
        elif line.strip():
            html_lines.append(f'<p>{format_inline(line)}</p>')

    if in_table:
        html_lines.append('</table>')

    body_html = '\n'.join(html_lines)
    return f"""<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<title>{title}</title>
{CSS_STYLE}
</head>
<body>
{body_html}
</body>
</html>"""

def format_inline(text):
    text = re.sub(r'\*\*(.*?)\*\*', r'<strong>\1</strong>', text)
    text = re.sub(r'`(.*?)`', r'<code>\1</code>', text)
    text = text.replace('PASS', '<span class="badge-pass">PASS</span>')
    text = text.replace('✅', '')
    return text

def convert_to_pdf(html_path, pdf_path):
    browsers = [
        r"C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe",
        r"C:\Program Files\Google\Chrome\Application\chrome.exe",
        r"C:\Program Files\Microsoft\Edge\Application\msedge.exe"
    ]
    browser = next((b for b in browsers if os.path.exists(b)), None)
    if not browser:
        print("No browser found for PDF conversion")
        return False

    cmd = [
        browser,
        "--headless",
        "--disable-gpu",
        "--run-all-compositor-stages-before-draw",
        f"--print-to-pdf={pdf_path}",
        html_path
    ]
    res = subprocess.run(cmd, capture_output=True)
    return res.returncode == 0 and os.path.exists(pdf_path)

def main():
    project_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    docs_dir = os.path.join(project_dir, 'docs')
    os.makedirs(docs_dir, exist_ok=True)

    files = [
        ('USER_GUIDE.md', 'User_Guide.html', 'User_Guide.pdf', 'BetterPresser User Guide'),
        ('TECHNICAL_SUMMARY.md', 'Technical_Summary.html', 'Technical_Summary.pdf', 'BetterPresser Technical Summary')
    ]

    for md_name, html_name, pdf_name, title in files:
        md_path = os.path.join(docs_dir, md_name)
        html_path = os.path.join(docs_dir, html_name)
        pdf_path = os.path.join(docs_dir, pdf_name)

        if os.path.exists(md_path):
            with open(md_path, 'r', encoding='utf-8') as f:
                content = f.read()
            html = md_to_html(content, title)
            with open(html_path, 'w', encoding='utf-8') as f:
                f.write(html)
            print(f"Generated HTML: {html_path}")

            if convert_to_pdf(html_path, pdf_path):
                print(f"Successfully generated PDF: {pdf_path}")
            else:
                print(f"PDF generation failed for {pdf_name}")

if __name__ == '__main__':
    main()
