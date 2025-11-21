#include <dong.h>
#include <iostream>
#include <cstring>

int main() {
    // Create context
    dong_context_t* ctx = dong_create_context();
    if (!ctx) {
        std::cerr << "Failed to create context\n";
        return 1;
    }

    // Create view (800x600)
    dong_view_t* view = dong_view_create(ctx, 800, 600);
    if (!view) {
        std::cerr << "Failed to create view\n";
        dong_destroy_context(ctx);
        return 1;
    }

    // Simple HTML test
    const char* html = R"(
        <!DOCTYPE html>
        <html>
            <head><title>DOM Test</title></head>
            <body>
                <div id="container">
                    <h1>Hello DOM</h1>
                    <p>This is a test paragraph.</p>
                    <span class="highlight">Highlighted text</span>
                </div>
            </body>
        </html>
    )";

    // Load HTML
    dong_view_load_html(view, html);
    std::cout << "HTML loaded successfully\n";

    // Trigger update
    dong_view_update(view);
    std::cout << "View updated\n";

    // Cleanup
    dong_view_destroy(view);
    dong_destroy_context(ctx);

    std::cout << "Test completed successfully\n";
    return 0;
}
