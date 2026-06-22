/*MIT License
*
*Copyright (c) 2026 @AZULKIAROD
*
*Permission is hereby granted, free of charge, to any person obtaining a copy
*of this software and associated documentation files (the "Software"), to deal
*in the Software without restriction, including without limitation the rights
*to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*copies of the Software, and to permit persons to whom the Software is
*furnished to do so, subject to the following conditions:
*
*The above copyright notice and this permission notice shall be included in all
*copies or substantial portions of the Software.
*
*THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
*SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define MAX_LINE_LENGTH 512

void open_in_browser(const char *html_path) {
    char command[512];

    #if defined(_WIN32)
    sprintf(command, "start %s", html_path);
    #elif defined(__APPLE__)
    sprintf(command, "open %s", html_path);
    #else
    sprintf(command, "xdg-open %s", html_path);
    #endif

    system(command);
}

void clear_line_ending(char *str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
    if (len > 1 && str[len - 2] == '\r') {
        str[len - 2] = '\0';
    }
}

void copy_file(const char *source, const char *destination) {
    FILE *src = fopen(source, "rb");
    if (!src) return;

    FILE *dst = fopen(destination, "wb");
    if (!dst) {
        fclose(src);
        return;
    }

    char buffer[4096];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        fwrite(buffer, 1, bytes, dst);
    }

    fclose(src);
    fclose(dst);
}

void transpile_gotozh(const char *input_file, const char *output_dir) {
    FILE *in = fopen(input_file, "r");
    if (!in) {
        printf("Error: Could not open input file %s\n", input_file);
        return;
    }

    #ifdef _WIN32
    mkdir(output_dir);
    #else
    mkdir(output_dir, 0777);
    #endif

    char html_path[512], css_path[512], js_path[512];
    sprintf(html_path, "%s/index.html", output_dir);
    sprintf(css_path, "%s/style.css", output_dir);
    sprintf(js_path, "%s/script.js", output_dir);

    FILE *out_html = fopen(html_path, "w");
    FILE *out_css = fopen(css_path, "w");
    FILE *out_js = fopen(js_path, "w");

    if (!out_html || !out_css || !out_js) {
        printf("Error: Could not create output files.\n");
        if (out_html) fclose(out_html);
        if (out_css) fclose(out_css);
        if (out_js) fclose(out_js);
        fclose(in);
        return;
    }

    fprintf(out_html, "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n");
    fprintf(out_html, "    <meta charset=\"UTF-8\">\n");
    fprintf(out_html, "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n");
    fprintf(out_html, "    <link rel=\"stylesheet\" href=\"style.css\">\n");

    char line[MAX_LINE_LENGTH];
    char current_context[32] = "";
    char attributes_buffer[MAX_LINE_LENGTH] = "";

    while (fgets(line, sizeof(line), in)) {
        clear_line_ending(line);

        if (strcmp(line, "{pag}") == 0) { strcpy(current_context, "pag"); continue; }
        if (strcmp(line, "{style}") == 0) { strcpy(current_context, "style"); continue; }
        if (strcmp(line, "{text}") == 0) { strcpy(current_context, "text"); continue; }
        if (strcmp(line, "{img}") == 0) { strcpy(current_context, "img"); continue; }
        if (strcmp(line, "{button}") == 0) { strcpy(current_context, "button"); continue; }
        if (strcmp(line, "{script}") == 0) { strcpy(current_context, "script"); continue; }

        if (strlen(line) == 0 && strcmp(current_context, "style") != 0 && strcmp(current_context, "script") != 0) {
            continue;
        }

        if (strcmp(current_context, "pag") == 0) {
            fprintf(out_html, "    <title>%s</title>\n</head>\n<body>\n", line);
            strcpy(current_context, "");
        }
        else if (strcmp(current_context, "style") == 0) {
            fprintf(out_css, "%s\n", line);
        }
        else if (strcmp(current_context, "script") == 0) {
            fprintf(out_js, "%s\n", line);
        }
        else if (strcmp(current_context, "text") == 0) {
            fprintf(out_html, "    <p>%s</p>\n", line);
            strcpy(current_context, "");
        }
        else if (strcmp(current_context, "img") == 0) {
            fprintf(out_html, "    <img %s>\n", line);

            char *src_start = strstr(line, "src=\"");
            if (src_start) {
                src_start += 5;
                char *src_end = strchr(src_start, '"');
                if (src_end) {
                    char img_name[256] = {0};
                    strncpy(img_name, src_start, src_end - src_start);

                    char dest_img_path[512];
                    sprintf(dest_img_path, "%s/%s", output_dir, img_name);
                    copy_file(img_name, dest_img_path);
                }
            }
            strcpy(current_context, "");
        }
        else if (strcmp(current_context, "button") == 0) {
            if (strlen(attributes_buffer) == 0) {
                strcpy(attributes_buffer, line);
            } else {
                fprintf(out_html, "    <button %s>%s</button>\n", attributes_buffer, line);
                attributes_buffer[0] = '\0';
                strcpy(current_context, "");
            }
        }
    }

    fprintf(out_html, "    <script src=\"script.js\"></script>\n");
    fprintf(out_html, "</body>\n</html>\n");

    fclose(in);
    fclose(out_html);
    fclose(out_css);
    fclose(out_js);

    printf("[GOTOZH] Universal compilation successful! Assets generated in: %s/\n", output_dir);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("GOTOZH Compiler - Invalid Arguments\n");
        printf("Usage:\n");
        printf("  gotozh --html {script.gtscr} {output_directory}\n");
        printf("  gotozh --view {script.gtscr}\n");
        return 1;
    }

    if (strcmp(argv[1], "--html") == 0) {
        if (argc < 4) {
            printf("Error: Missing output directory for --html flag.\n");
            return 1;
        }
        transpile_gotozh(argv[2], argv[3]);
    }
    else if (strcmp(argv[1], "--view") == 0) {
        transpile_gotozh(argv[2], "./.gotozh_preview");
        open_in_browser("./.gotozh_preview/index.html");
    }
    else {
        printf("Unknown flag: %s\n", argv[1]);
        return 1;
    }

    return 0;
}
