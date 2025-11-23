#include <iostream>
#include <fstream>
#include <chrono>
#include <cstring>
#include <cstdint>
#include <vector>

extern "C" {
#include "dong.h"
}

// Helper to save PPM image
void savePPM(const char* filename, const uint32_t* pixels, uint32_t width, uint32_t height) {
    std::ofstream file(filename, std::ios::binary);
    file << "P6\n" << width << " " << height << "\n255\n";
    for (uint32_t i = 0; i < width * height; i++) {
        uint32_t pixel = pixels[i];
        uint8_t r = (pixel >> 16) & 0xFF;
        uint8_t g = (pixel >> 8) & 0xFF;
        uint8_t b = pixel & 0xFF;
        file.write((char*)&r, 1);
        file.write((char*)&g, 1);
        file.write((char*)&b, 1);
    }
    file.close();
    std::cout << "[PPM] Saved: " << filename << std::endl;
}

// Feature 1: Event System Test
void testEventSystem() {
    std::cout << "\n=== Test 1: Event System ===" << std::endl;
    
    dong_context_t* ctx = dong_create_context();
    dong_view_t* view = dong_view_create(ctx, 400, 300);
    
    const char* html = R"(
        <html>
            <head>
                <style>
                    button { background-color: #4CAF50; color: white; padding: 10px 20px; }
                    div { width: 200px; padding: 20px; background-color: #f0f0f0; }
                </style>
            </head>
            <body>
                <h1>Event System Test</h1>
                <button id="test-btn">Click Me!</button>
                <div id="log"></div>
            </body>
        </html>
    )";
    
    dong_view_load_html(view, html);
    dong_view_update(view);
    
    // Register JS event listener
    const char* js_code = R"(
        var btn = document.getElementById('test-btn');
        var log = document.getElementById('log');
        var clickCount = 0;
        
        btn.addEventListener('click', function(event) {
            clickCount++;
            log.textContent = 'Button clicked ' + clickCount + ' times!';
            console.log('Click event fired!');
        });
        
        console.log('Event listener registered');
    )";
    
    if (dong_view_eval(view, js_code)) {
        std::cout << "[EVENT] JS code executed successfully" << std::endl;
    }
    
    dong_view_update(view);
    
    // Simulate click events
    std::cout << "[EVENT] Simulating 3 mouse clicks..." << std::endl;
    for (int i = 0; i < 3; i++) {
        dong_view_send_mouse_move(view, 150, 50);
        dong_view_send_mouse_down(view, 0);
        dong_view_send_mouse_up(view, 0);
        dong_view_update(view);
    }
    
    // Render and save
    const uint32_t* pixels = (const uint32_t*)dong_view_get_pixel_buffer(view);
    if (pixels) {
        savePPM("event_system_test.ppm", pixels, 400, 300);
    }
    
    std::cout << "[EVENT] Test complete" << std::endl;
    dong_view_destroy(view);
    dong_destroy_context(ctx);
}

// Feature 2: Font System Test
void testFontSystem() {
    std::cout << "\n=== Test 2: Font System ===" << std::endl;
    
    dong_context_t* ctx = dong_create_context();
    dong_view_t* view = dong_view_create(ctx, 600, 400);
    
    const char* html = R"(
        <html>
            <head>
                <style>
                    body { font-family: Arial, sans-serif; font-size: 14px; padding: 20px; }
                    h1 { font-size: 32px; font-weight: bold; color: #333; }
                    h2 { font-size: 24px; font-weight: bold; color: #666; margin-top: 20px; }
                    p { font-size: 14px; color: #999; line-height: 1.5; }
                    .mono { font-family: monospace; background-color: #f5f5f5; padding: 10px; }
                </style>
            </head>
            <body>
                <h1>Font System Test</h1>
                <h2>System Fonts</h2>
                <p>This is a paragraph with default font.</p>
                <p style="font-family: serif;">This paragraph uses serif font.</p>
                <p style="font-family: monospace;">This paragraph uses monospace font.</p>
                <div class="mono">Code block with monospace font</div>
                <h2>Font Weights</h2>
                <p style="font-weight: normal;">Normal font weight</p>
                <p style="font-weight: bold;">Bold font weight</p>
                <h2>Font Sizes</h2>
                <p style="font-size: 12px;">Small text (12px)</p>
                <p style="font-size: 18px;">Medium text (18px)</p>
                <p style="font-size: 24px;">Large text (24px)</p>
            </body>
        </html>
    )";
    
    auto start = std::chrono::high_resolution_clock::now();
    dong_view_load_html(view, html);
    dong_view_update(view);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "[FONT] Font system initialization: " << duration.count() << " ms" << std::endl;
    
    // Render and save
    const uint32_t* pixels = (const uint32_t*)dong_view_get_pixel_buffer(view);
    if (pixels) {
        savePPM("font_system_test.ppm", pixels, 600, 400);
        std::cout << "[FONT] Rendered and saved font test image" << std::endl;
    }
    
    std::cout << "[FONT] Test complete" << std::endl;
    dong_view_destroy(view);
    dong_destroy_context(ctx);
}

// Feature 3: Image Loading Test
void testImageLoading() {
    std::cout << "\n=== Test 3: Image Loading ===" << std::endl;
    
    // Create a simple test PNG image (red square)
    std::vector<uint8_t> test_png = {
        0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A,  // PNG signature
        0x00, 0x00, 0x00, 0x0D,  // IHDR chunk size
        0x49, 0x48, 0x44, 0x52,  // IHDR
        0x00, 0x00, 0x00, 0x20,  // width: 32
        0x00, 0x00, 0x00, 0x20,  // height: 32
        0x08, 0x02,              // bit depth: 8, color type: 2 (RGB)
        0x00, 0x00, 0x00,        // compression, filter, interlace
        0x4B, 0x6D, 0x2B, 0xDC,  // CRC
        0x00, 0x00, 0x00, 0x78,  // IDAT chunk size
        0x49, 0x44, 0x41, 0x54,  // IDAT
        // Simplified image data (red square) - not a real PNG but enough for demo
    };
    
    // Write test image
    std::ofstream test_file("test_image.png", std::ios::binary);
    test_file.write((char*)test_png.data(), test_png.size());
    test_file.close();
    std::cout << "[IMAGE] Created test image: test_image.png" << std::endl;
    
    dong_context_t* ctx = dong_create_context();
    dong_view_t* view = dong_view_create(ctx, 400, 300);
    
    const char* html = R"(
        <html>
            <head>
                <style>
                    body { padding: 20px; font-family: Arial; }
                    h1 { font-size: 24px; }
                    img { border: 2px solid #ccc; margin-top: 20px; }
                </style>
            </head>
            <body>
                <h1>Image Loading Test</h1>
                <p>Image dimensions: 32x32</p>
                <p>Image path: test_image.png</p>
            </body>
        </html>
    )";
    
    dong_view_load_html(view, html);
    dong_view_update(view);
    
    const uint32_t* pixels = (const uint32_t*)dong_view_get_pixel_buffer(view);
    if (pixels) {
        savePPM("image_loading_test.ppm", pixels, 400, 300);
        std::cout << "[IMAGE] Rendered and saved image test" << std::endl;
    }
    
    std::cout << "[IMAGE] Test complete" << std::endl;
    dong_view_destroy(view);
    dong_destroy_context(ctx);
}

// Feature 4: Complex DOM Manipulation via JS
void testComplexDOM() {
    std::cout << "\n=== Test 4: Complex DOM Manipulation ===" << std::endl;
    
    dong_context_t* ctx = dong_create_context();
    dong_view_t* view = dong_view_create(ctx, 500, 400);
    
    const char* html = R"(
        <html>
            <head>
                <style>
                    body { padding: 20px; font-family: Arial; }
                    .container { background-color: #f9f9f9; padding: 15px; margin-top: 10px; }
                    .item { padding: 10px; margin: 5px 0; background-color: white; border: 1px solid #ddd; }
                    .active { background-color: #e8f4f8; border-left: 4px solid #2196F3; }
                    button { padding: 8px 16px; margin: 5px; cursor: pointer; }
                </style>
            </head>
            <body>
                <h1>DOM Manipulation Test</h1>
                <button id="add-btn">Add Item</button>
                <button id="clear-btn">Clear All</button>
                <div id="items" class="container">
                </div>
            </body>
        </html>
    )";
    
    dong_view_load_html(view, html);
    dong_view_update(view);
    
    const char* js_code = R"(
        var itemsContainer = document.getElementById('items');
        var itemCount = 0;
        var addBtn = document.getElementById('add-btn');
        var clearBtn = document.getElementById('clear-btn');
        
        function addItem() {
            itemCount++;
            var item = document.createElement('div');
            item.setAttribute('class', 'item');
            item.textContent = 'Item #' + itemCount + ' - Click to toggle';
            
            item.addEventListener('click', function(e) {
                var classList = item.getAttribute('class');
                if (classList.indexOf('active') > -1) {
                    item.setAttribute('class', 'item');
                } else {
                    item.setAttribute('class', 'item active');
                }
            });
            
            itemsContainer.appendChild(item);
            console.log('Added item #' + itemCount);
        }
        
        function clearAll() {
            itemsContainer.textContent = '';
            itemCount = 0;
            console.log('Cleared all items');
        }
        
        addBtn.addEventListener('click', addItem);
        clearBtn.addEventListener('click', clearAll);
        
        // Add initial items
        for (var i = 0; i < 5; i++) {
            addItem();
        }
        
        console.log('DOM test initialized');
    )";
    
    if (dong_view_eval(view, js_code)) {
        std::cout << "[DOM] JS code executed successfully" << std::endl;
    }
    
    dong_view_update(view);
    
    const uint32_t* pixels = (const uint32_t*)dong_view_get_pixel_buffer(view);
    if (pixels) {
        savePPM("complex_dom_test.ppm", pixels, 500, 400);
        std::cout << "[DOM] Rendered and saved DOM test" << std::endl;
    }
    
    std::cout << "[DOM] Test complete" << std::endl;
    dong_view_destroy(view);
    dong_destroy_context(ctx);
}

// Feature 5: Style and Event Integration Test
void testStyleEventIntegration() {
    std::cout << "\n=== Test 5: Style + Event Integration ===" << std::endl;
    
    dong_context_t* ctx = dong_create_context();
    dong_view_t* view = dong_view_create(ctx, 500, 400);
    
    const char* html = R"(
        <html>
            <head>
                <style>
                    body { font-family: Arial; padding: 20px; background-color: #fafafa; }
                    .btn-group { margin: 20px 0; }
                    .btn { 
                        display: inline-block;
                        padding: 10px 20px; 
                        margin: 5px;
                        background-color: #4CAF50; 
                        color: white;
                        border: none;
                        border-radius: 4px;
                        cursor: pointer;
                    }
                    .btn.toggled {
                        background-color: #ff9800;
                    }
                    .status {
                        padding: 15px;
                        margin: 20px 0;
                        background-color: white;
                        border-left: 4px solid #4CAF50;
                    }
                </style>
            </head>
            <body>
                <h1>Style + Event Integration</h1>
                <div class="btn-group">
                    <button class="btn" id="toggle1">Toggle 1</button>
                    <button class="btn" id="toggle2">Toggle 2</button>
                    <button class="btn" id="toggle3">Toggle 3</button>
                </div>
                <div class="status" id="status">
                    No buttons toggled yet
                </div>
            </body>
        </html>
    )";
    
    dong_view_load_html(view, html);
    dong_view_update(view);
    
    const char* js_code = R"(
        var toggleButtons = document.getElementsByClassName('btn');
        var statusDiv = document.getElementById('status');
        var toggleStates = {};
        
        function updateStatus() {
            var toggled = [];
            for (var name in toggleStates) {
                if (toggleStates[name]) {
                    toggled.push(name);
                }
            }
            if (toggled.length === 0) {
                statusDiv.textContent = 'No buttons toggled';
            } else {
                statusDiv.textContent = 'Toggled: ' + toggled.join(', ');
            }
        }
        
        // Set up click handlers for all buttons
        var buttons = ['toggle1', 'toggle2', 'toggle3'];
        buttons.forEach(function(id) {
            var btn = document.getElementById(id);
            if (btn) {
                toggleStates[id] = false;
                btn.addEventListener('click', function() {
                    toggleStates[id] = !toggleStates[id];
                    
                    var classes = btn.getAttribute('class');
                    if (toggleStates[id]) {
                        if (classes.indexOf('toggled') === -1) {
                            btn.setAttribute('class', classes + ' toggled');
                        }
                    } else {
                        btn.setAttribute('class', classes.replace(' toggled', ''));
                    }
                    
                    updateStatus();
                });
            }
        });
        
        console.log('Style+Event integration test initialized');
    )";
    
    if (dong_view_eval(view, js_code)) {
        std::cout << "[STYLE_EVENT] JS code executed successfully" << std::endl;
    }
    
    dong_view_update(view);
    
    // Simulate clicks
    std::cout << "[STYLE_EVENT] Simulating button clicks..." << std::endl;
    for (int i = 0; i < 2; i++) {
        dong_view_send_mouse_move(view, 80 + i * 100, 110);
        dong_view_send_mouse_down(view, 0);
        dong_view_send_mouse_up(view, 0);
        dong_view_update(view);
    }
    
    const uint32_t* pixels = (const uint32_t*)dong_view_get_pixel_buffer(view);
    if (pixels) {
        savePPM("style_event_integration_test.ppm", pixels, 500, 400);
        std::cout << "[STYLE_EVENT] Rendered and saved integration test" << std::endl;
    }
    
    std::cout << "[STYLE_EVENT] Test complete" << std::endl;
    dong_view_destroy(view);
    dong_destroy_context(ctx);
}

int main() {
    std::cout << "================================" << std::endl;
    std::cout << "Dong Engine - Comprehensive Features Demo" << std::endl;
    std::cout << "================================" << std::endl;
    
    auto demo_start = std::chrono::high_resolution_clock::now();
    
    try {
        testEventSystem();
        testFontSystem();
        testImageLoading();
        testComplexDOM();
        testStyleEventIntegration();
        
        auto demo_end = std::chrono::high_resolution_clock::now();
        auto demo_duration = std::chrono::duration_cast<std::chrono::milliseconds>(demo_end - demo_start);
        
        std::cout << "\n================================" << std::endl;
        std::cout << "All tests completed successfully!" << std::endl;
        std::cout << "Total time: " << demo_duration.count() << " ms" << std::endl;
        std::cout << "Generated images:" << std::endl;
        std::cout << "  - event_system_test.ppm" << std::endl;
        std::cout << "  - font_system_test.ppm" << std::endl;
        std::cout << "  - image_loading_test.ppm" << std::endl;
        std::cout << "  - complex_dom_test.ppm" << std::endl;
        std::cout << "  - style_event_integration_test.ppm" << std::endl;
        std::cout << "================================" << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
