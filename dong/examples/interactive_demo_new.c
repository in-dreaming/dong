/**
 * Dong Engine Interactive Demo (Simplified with AppCore)
 *
 * Demonstrates button clicks, ScrollView wheel scrolling, and text input.
 * Uses the new AppCore API for simplified application development.
 *
 * Original: ~450 lines -> New: ~80 lines
 */

#include "dong_app.h"
#include <stdio.h>
#include <string.h>

// Demo HTML content (C doesn't support raw strings, so we concatenate)
static const char* DEMO_HTML =
    "<!DOCTYPE html>\n"
    "<html>\n"
    "<head>\n"
    "    <style>\n"
    "        * { box-sizing: border-box; }\n"
    "        body {\n"
    "            margin: 0;\n"
    "            padding: 20px;\n"
    "            background-color: #1a1a2e;\n"
    "            color: #eaeaea;\n"
    "            font-family: Arial, sans-serif;\n"
    "        }\n"
    "        h1 { font-size: 28px; color: #00d4ff; margin-bottom: 20px; }\n"
    "        .section {\n"
    "            background-color: #16213e;\n"
    "            border-radius: 8px;\n"
    "            padding: 16px;\n"
    "            margin-bottom: 16px;\n"
    "        }\n"
    "        .section-title { font-size: 18px; color: #e94560; margin-bottom: 12px; }\n"
    "        .button-row { display: flex; gap: 12px; }\n"
    "        button {\n"
    "            background-color: #0f3460;\n"
    "            color: #ffffff;\n"
    "            padding: 12px 24px;\n"
    "            border-radius: 6px;\n"
    "            font-size: 16px;\n"
    "        }\n"
    "        #click-count { margin-top: 12px; font-size: 14px; color: #94a3b8; }\n"
    "        .scroll-container {\n"
    "            overflow: scroll;\n"
    "            height: 150px;\n"
    "            background-color: #0f3460;\n"
    "            border-radius: 6px;\n"
    "            padding: 8px;\n"
    "        }\n"
    "        .scroll-item {\n"
    "            padding: 10px;\n"
    "            margin-bottom: 8px;\n"
    "            background-color: #1a1a2e;\n"
    "            border-radius: 4px;\n"
    "        }\n"
    "        .input-group { margin-bottom: 12px; }\n"
    "        .input-label { font-size: 14px; color: #94a3b8; margin-bottom: 6px; }\n"
    "        input {\n"
    "            width: 100%;\n"
    "            padding: 12px;\n"
    "            background-color: #0f3460;\n"
    "            color: #ffffff;\n"
    "            border-radius: 6px;\n"
    "            font-size: 16px;\n"
    "        }\n"
    "        #input-preview {\n"
    "            margin-top: 12px;\n"
    "            padding: 12px;\n"
    "            background-color: #0f3460;\n"
    "            border-radius: 6px;\n"
    "            font-size: 14px;\n"
    "            color: #94a3b8;\n"
    "        }\n"
    "        .status-bar {\n"
    "            position: fixed;\n"
    "            bottom: 0;\n"
    "            left: 0;\n"
    "            right: 0;\n"
    "            padding: 8px 20px;\n"
    "            background-color: #0f3460;\n"
    "            font-size: 12px;\n"
    "            color: #64748b;\n"
    "        }\n"
    "    </style>\n"
    "</head>\n"
    "<body>\n"
    "    <h1>Dong Interactive Demo</h1>\n"
    "    <div class=\"section\">\n"
    "        <div class=\"section-title\">Button Clicks</div>\n"
    "        <div class=\"button-row\">\n"
    "            <button id=\"btn-primary\">Primary Button</button>\n"
    "            <button id=\"btn-secondary\">Secondary</button>\n"
    "            <button id=\"btn-reset\">Reset</button>\n"
    "        </div>\n"
    "        <div id=\"click-count\">Clicks: 0</div>\n"
    "    </div>\n"
    "    <div class=\"section\">\n"
    "        <div class=\"section-title\">ScrollView (use mouse wheel)</div>\n"
    "        <div class=\"scroll-container\" id=\"scroll-area\">\n"
    "            <div class=\"scroll-item\">Item 1 - Scroll down to see more</div>\n"
    "            <div class=\"scroll-item\">Item 2 - Lorem ipsum dolor sit amet</div>\n"
    "            <div class=\"scroll-item\">Item 3 - Consectetur adipiscing elit</div>\n"
    "            <div class=\"scroll-item\">Item 4 - Sed do eiusmod tempor</div>\n"
    "            <div class=\"scroll-item\">Item 5 - Incididunt ut labore</div>\n"
    "            <div class=\"scroll-item\">Item 6 - Et dolore magna aliqua</div>\n"
    "            <div class=\"scroll-item\">Item 7 - Ut enim ad minim veniam</div>\n"
    "            <div class=\"scroll-item\">Item 8 - Quis nostrud exercitation</div>\n"
    "        </div>\n"
    "    </div>\n"
    "    <div class=\"section\">\n"
    "        <div class=\"section-title\">Text Input</div>\n"
    "        <div class=\"input-group\">\n"
    "            <div class=\"input-label\">Enter your name:</div>\n"
    "            <input type=\"text\" id=\"name-input\" placeholder=\"Type here...\" />\n"
    "        </div>\n"
    "        <div class=\"input-group\">\n"
    "            <div class=\"input-label\">Enter your message:</div>\n"
    "            <input type=\"text\" id=\"message-input\" placeholder=\"Your message...\" />\n"
    "        </div>\n"
    "        <div id=\"input-preview\">Preview: (type something above)</div>\n"
    "    </div>\n"
    "    <div class=\"status-bar\" id=\"status\">Ready. Click buttons, scroll, or type.</div>\n"
    "    <script>\n"
    "        var clicks = 0;\n"
    "        var btnPrimary = document.getElementById('btn-primary');\n"
    "        var btnSecondary = document.getElementById('btn-secondary');\n"
    "        var btnReset = document.getElementById('btn-reset');\n"
    "        var clickCount = document.getElementById('click-count');\n"
    "        var status = document.getElementById('status');\n"
    "        function updateCount() { clickCount.textContent = 'Clicks: ' + clicks; }\n"
    "        function setStatus(msg) { status.textContent = msg; }\n"
    "        btnPrimary.addEventListener('click', function() {\n"
    "            clicks++; updateCount(); setStatus('Primary clicked! Total: ' + clicks);\n"
    "        });\n"
    "        btnSecondary.addEventListener('click', function() {\n"
    "            clicks++; updateCount(); setStatus('Secondary clicked! Total: ' + clicks);\n"
    "        });\n"
    "        btnReset.addEventListener('click', function() {\n"
    "            clicks = 0; updateCount(); setStatus('Counter reset!');\n"
    "        });\n"
    "        var nameInput = document.getElementById('name-input');\n"
    "        var msgInput = document.getElementById('message-input');\n"
    "        var preview = document.getElementById('input-preview');\n"
    "        function updatePreview() {\n"
    "            var txt = 'Preview: ';\n"
    "            if (nameInput.value) txt += 'Hello, ' + nameInput.value + '! ';\n"
    "            if (msgInput.value) txt += 'Message: ' + msgInput.value;\n"
    "            if (!nameInput.value && !msgInput.value) txt += '(type something above)';\n"
    "            preview.textContent = txt;\n"
    "        }\n"
    "        nameInput.addEventListener('input', function() { updatePreview(); setStatus('Typing name...'); });\n"
    "        msgInput.addEventListener('input', function() { updatePreview(); setStatus('Typing message...'); });\n"
    "    </script>\n"
    "</body>\n"
    "</html>\n";

int main(void) {
    printf("=== Dong Engine Interactive Demo (AppCore) ===\n");
    printf("Features: Button clicks, ScrollView, Text input\n");

    // Create application with AppCore
    dong_app_config_t config = {0};
    config.title = "Dong Interactive Demo";
    config.width = 800;
    config.height = 1280;
    config.enable_dong = 1;
    config.resizable = 1;

    dong_app_t* app = dong_app_create(&config);
    if (!app) {
        fprintf(stderr, "Failed to create application\n");
        return 1;
    }

    // Load HTML content
    dong_app_load_html(app, DEMO_HTML);

    printf("Instructions:\n");
    printf("  - Click buttons to increment counter\n");
    printf("  - Scroll mouse wheel over the list\n");
    printf("  - Click input fields and type text\n");
    printf("  - Press ESC or close window to exit\n");

    // Run main loop (blocking)
    dong_app_run(app, NULL, NULL);

    // Cleanup
    printf("Shutting down...\n");
    dong_app_destroy(app);

    printf("=== Interactive Demo Complete ===\n");
    return 0;
}
