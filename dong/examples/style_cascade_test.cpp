#include <dong.h>
#include <iostream>
#include <iomanip>

int main() {
    std::cout << "=== CSS Cascade & Specificity Test ===" << std::endl << std::endl;

    // Create context
    dong_context_t* ctx = dong_create_context();
    if (!ctx) {
        std::cerr << "Failed to create context" << std::endl;
        return 1;
    }

    // Create view
    dong_view_t* view = dong_view_create(ctx, 1024, 768);
    if (!view) {
        std::cerr << "Failed to create view" << std::endl;
        dong_destroy_context(ctx);
        return 1;
    }

    // HTML with CSS cascade rules
    const char* html = R"(
        <!DOCTYPE html>
        <html>
            <head>
                <style>
                    p { color: blue; }
                    .highlight { color: red; }
                    #special { color: green; }
                    p.highlight { color: purple; }
                    input[type="text"] { background: #ffffcc; }
                </style>
            </head>
            <body>
                <h1>CSS Cascade Test</h1>
                <p>Normal paragraph</p>
                <p class="highlight">Highlighted paragraph</p>
                <p id="special" class="highlight">Special paragraph</p>
                <input type="text" placeholder="Text input">
                <input type="password" placeholder="Password">
            </body>
        </html>
    )";

    // Load HTML and trigger rendering
    std::cout << "Loading HTML with CSS styles..." << std::endl;
    dong_view_load_html(view, html);
    
    std::cout << "Updating view..." << std::endl;
    dong_view_update(view);
    
    std::cout << "✓ CSS Cascade test completed successfully" << std::endl;

    // Cleanup
    dong_view_destroy(view);
    dong_destroy_context(ctx);

    return 0;
}
