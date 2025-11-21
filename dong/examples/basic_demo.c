#include <dong.h>
#include <stdio.h>

int main(void) {
    printf("Dong UI Engine - Basic Demo\n");

    // Create context
    dong_context_t* ctx = dong_create_context();
    if (!ctx) {
        printf("Failed to create context\n");
        return 1;
    }

    // Create view
    dong_view_t* view = dong_view_create(ctx, 800, 600);
    if (!view) {
        printf("Failed to create view\n");
        dong_destroy_context(ctx);
        return 1;
    }

    // Load simple HTML
    const char* html = "<html><body><h1>Hello Dong!</h1></body></html>";
    dong_view_load_html(view, html);

    // Update and render
    dong_view_update(view);

    printf("Demo completed successfully\n");

    // Cleanup
    dong_view_destroy(view);
    dong_destroy_context(ctx);

    return 0;
}
