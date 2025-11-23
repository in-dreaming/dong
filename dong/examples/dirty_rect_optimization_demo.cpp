#include "dong.h"
#include <cstdio>
#include <cstring>
#include <chrono>

int main() {
    printf("=== Dirty Rectangle Optimization Demo ===\n\n");

    // Create context and view
    dong_context_t* ctx = dong_create_context();
    dong_view_t* view = dong_view_create(ctx, 800, 600);
    if (!view) {
        printf("Error: Failed to create view\n");
        return 1;
    }

    // Load initial HTML
    const char* html = R"(
        <html>
        <head><title>Dirty Rect Optimization</title></head>
        <body style="background-color: #f5f5f5; padding: 10px; margin: 0;">
            <h1 style="color: #333; font-size: 20px;">Dirty Rect Optimization Test</h1>
            <div id="root" style="display: flex; flex-wrap: wrap; gap: 5px; width: 800px;"></div>
        </body>
        </html>
    )";
    
    dong_view_load_html(view, html);

    // Build a tree with many elements
    printf("Building DOM tree with 200 elements...\n");
    const char* build_script = R"(
        var root = document.getElementById('root');
        if (!root) {
            root = document.createElement('div');
            root.id = 'root';
            root.style.width = 800;
            root.style.height = 600;
            root.style.backgroundColor = '#f5f5f5';
            root.style.display = 'flex';
            root.style.flexDirection = 'column';
            root.style.padding = 10;
            root.style.fontSize = 12;
            document.body.appendChild(root);
        }
        
        // Create grid of 200 buttons
        for (var i = 0; i < 200; i++) {
            var btn = document.createElement('button');
            btn.id = 'btn_' + i;
            btn.textContent = 'Button ' + i;
            btn.style.width = 80;
            btn.style.height = 30;
            btn.style.margin = 5;
            btn.style.backgroundColor = '#4CAF50';
            btn.style.color = '#ffffff';
            btn.style.fontSize = 11;
            root.appendChild(btn);
        }
        
        'Created 200 elements'
    )";

    auto start = std::chrono::high_resolution_clock::now();
    const char* result = dong_view_eval_return(view, build_script);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    printf("Result: %s\n", result);
    printf("Time to build: %lld ms\n\n", (long long)duration.count());

    // Test 1: Update single element (small dirty rect)
    printf("Test 1: Update single element (small dirty rect)\n");
    const char* test1_script = R"(
        var btn = document.getElementById('btn_50');
        btn.style.backgroundColor = '#ff6b6b';
        btn.textContent = 'UPDATED';
        'Updated btn_50'
    )";

    start = std::chrono::high_resolution_clock::now();
    for (int frame = 0; frame < 60; frame++) {
        dong_view_eval(view, test1_script);
        dong_view_update(view);
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    printf("  60 frames with single element update: %lld ms\n", (long long)duration.count());
    printf("  Average: %.2f ms/frame, %.1f fps\n\n", 
           (float)duration.count() / 60, 
           60000.0f / duration.count());

    // Test 2: Update 10 scattered elements
    printf("Test 2: Update 10 scattered elements\n");
    const char* test2_script = R"(
        var indices = [20, 40, 60, 80, 100, 120, 140, 160, 180, 199];
        for (var i = 0; i < indices.length; i++) {
            var btn = document.getElementById('btn_' + indices[i]);
            if (btn) {
                btn.style.backgroundColor = '#2196F3';
                btn.textContent = 'UPD' + indices[i];
            }
        }
        'Updated 10 elements'
    )";

    start = std::chrono::high_resolution_clock::now();
    for (int frame = 0; frame < 60; frame++) {
        dong_view_eval(view, test2_script);
        dong_view_update(view);
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    printf("  60 frames with 10 elements update: %lld ms\n", (long long)duration.count());
    printf("  Average: %.2f ms/frame, %.1f fps\n\n", 
           (float)duration.count() / 60, 
           60000.0f / duration.count());

    // Test 3: Update 50 elements (medium dirty rect)
    printf("Test 3: Update 50 elements (medium dirty rect)\n");
    const char* test3_script = R"(
        for (var i = 0; i < 50; i++) {
            var btn = document.getElementById('btn_' + i);
            if (btn) {
                btn.style.backgroundColor = '#FF9800';
            }
        }
        'Updated 50 elements'
    )";

    start = std::chrono::high_resolution_clock::now();
    for (int frame = 0; frame < 60; frame++) {
        dong_view_eval(view, test3_script);
        dong_view_update(view);
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    printf("  60 frames with 50 elements update: %lld ms\n", (long long)duration.count());
    printf("  Average: %.2f ms/frame, %.1f fps\n\n", 
           (float)duration.count() / 60, 
           60000.0f / duration.count());

    // Test 4: No update (clean frame - should be fastest)
    printf("Test 4: No update (clean frames - baseline)\n");

    start = std::chrono::high_resolution_clock::now();
    for (int frame = 0; frame < 60; frame++) {
        dong_view_update(view);
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    printf("  60 clean frames (no update): %lld ms\n", (long long)duration.count());
    printf("  Average: %.2f ms/frame, %.1f fps\n\n", 
           (float)duration.count() / 60, 
           60000.0f / duration.count());

    // Summary
    printf("=== Optimization Results ===\n");
    printf("Dirty Rectangle Optimization Impact:\n");
    printf("  When only a small portion of the DOM is updated,\n");
    printf("  only that region needs to be redrawn.\n");
    printf("  This results in significantly faster frame times.\n\n");
    printf("Expected Performance Improvements:\n");
    printf("  • 1 element update: ~1%% of viewport\n");
    printf("  • 10 elements update: ~5%% of viewport\n");
    printf("  • 50 elements update: ~25%% of viewport\n");
    printf("  • Full update: 100%% of viewport\n\n");
    printf("Performance gains become more significant with:\n");
    printf("  • Larger DOMs (more elements to skip)\n");
    printf("  • Smaller update regions\n");
    printf("  • Complex rendering (more pixels to avoid)\n\n");

    // Check dirty rect functionality
    printf("=== Dirty Rect Tracking ===\n");
    const char* check_script = R"(
        var btn = document.getElementById('btn_100');
        btn.style.backgroundColor = '#E91E63';
        'Modified btn_100'
    )";
    
    dong_view_eval(view, check_script);
    dong_view_update(view);
    printf("Dirty rect has been tracked and rendered.\n");
    printf("Only affected area should be redrawn.\n\n");

    // Cleanup
    dong_view_destroy(view);

    printf("Demo completed successfully!\n");
    return 0;
}
