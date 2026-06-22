import os
import tkinter as tk
from tkinter import filedialog, messagebox

def amalgamate_assets(target_dir):
    html_path = os.path.join(target_dir, "index.html")
    css_path = os.path.join(target_dir, "style.css")
    js_path = os.path.join(target_dir, "script.js")
    
    if not os.path.exists(html_path):
        raise FileNotFoundError("Target directory must contain an index.html file.")
        
    with open(html_path, "r", encoding="utf-8") as f:
        html_content = f.read()
        
    css_content = ""
    if os.path.exists(css_path):
        with open(css_path, "r", encoding="utf-8") as f:
            css_content = f.read()
            
    js_content = ""
    if os.path.exists(js_path):
        with open(js_path, "r", encoding="utf-8") as f:
            js_content = f.read()

    link_tag = '<link rel="stylesheet" href="style.css">'
    style_block = f"<style>\n{css_content}\n</style>"
    if link_tag in html_content:
        html_content = html_content.replace(link_tag, style_block)
    else:
        html_content = html_content.replace("</head>", f"{style_block}\n</head>")

    script_tag = '<script src="script.js"></script>'
    script_block = f"<script>\n{js_content}\n</script>"
    if script_tag in html_content:
        html_content = html_content.replace(script_tag, script_block)
    else:
        html_content = html_content.replace("</body>", f"{script_block}\n</body>")

    output_path = os.path.join(target_dir, "bundle.html")
    with open(output_path, "w", encoding="utf-8") as f:
        f.write(html_content)
        
    return output_path

def select_directory():
    dir_selected = filedialog.askdirectory(title="Select GOTOZH Output Directory")
    if dir_selected:
        try:
            out_file = amalgamate_assets(dir_selected)
            messagebox.showinfo("Success", f"Amalgamated standalone file created successfully:\n{out_file}")
        except Exception as e:
            messagebox.showerror("Error", f"Failed to amalgamate files:\n{str(e)}")

root = tk.Tk()
root.title("GOTOZH Amalgamator")
root.geometry("400x150")
root.configure(bg="#121212")

label = tk.Label(root, text="GOTOZH Asset Amalgamator Tool", fg="#ffffff", bg="#121212", font=("Arial", 14, "bold"))
label.pack(pady=20)

btn = tk.Button(root, text="Select Compile Directory", command=select_directory, fg="#ffffff", bg="#007bff", activebackground="#0056b3", font=("Arial", 11), padx=10, pady=5)
btn.pack(pady=10)

root.mainloop()