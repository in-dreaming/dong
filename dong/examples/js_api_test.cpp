#include <dong.h>
#include "../src/dom/dom_manager.hpp"
#include "../src/script/script_engine.hpp"
#include "../src/script/js_bindings.hpp"
#include <iostream>
#include <iomanip>

using namespace dong;
using namespace dong::dom;
using namespace dong::script;

void print_section(const std::string& title) {
    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << title << std::endl;
    std::cout << std::string(50, '=') << std::endl;
}

void print_test(const std::string& test_name, bool passed) {
    std::cout << (passed ? "✓ " : "✗ ") << test_name << std::endl;
}

int main() {
    std::cout << "=== Dong Engine JavaScript API Test Suite ===" << std::endl;

    // Initialize engine and DOM
    dong_context_t* ctx = dong_create_context();
    dong_view_t* view = dong_view_create(ctx, 800, 600);
    
    if (!ctx || !view) {
        std::cerr << "Failed to initialize engine" << std::endl;
        return 1;
    }

    // Test 1: Basic HTML with IDs and Classes
    print_section("Test 1: DOM Selection API");
    
    const char* html1 = R"(
        <html>
            <head><title>Test</title></head>
            <body>
                <div id="container" class="wrapper">
                    <h1 id="title" class="header">Welcome</h1>
                    <p id="content" class="text">This is a test</p>
                    <div class="box">Box 1</div>
                    <div class="box">Box 2</div>
                    <div class="box">Box 3</div>
                </div>
            </body>
        </html>
    )";

    dong_view_load_html(view, html1);
    std::cout << "HTML loaded successfully" << std::endl;

    // Test 2: Console API
    print_section("Test 2: Console API");
    
    auto script_engine = std::make_unique<ScriptEngine>();
    auto dom_manager = std::make_unique<Manager>();
    auto event_dispatcher = std::make_unique<EventDispatcher>();
    auto js_bindings = std::make_unique<JSBindings>(script_engine.get(), dom_manager.get(), event_dispatcher.get());
    
    js_bindings->initialize();
    
    // Parse HTML to populate DOM
    Parser parser;
    auto root = parser.parse(html1);
    
    if (!root) {
        std::cerr << "Failed to parse HTML" << std::endl;
        return 1;
    }

    // Test console.log
    std::cout << "\nTesting console.log:" << std::endl;
    bool console_log_ok = script_engine->eval("console.log('Hello', 'World');");
    print_test("console.log()", console_log_ok);

    bool console_warn_ok = script_engine->eval("console.warn('Warning message');");
    print_test("console.warn()", console_warn_ok);

    bool console_error_ok = script_engine->eval("console.error('Error message');");
    print_test("console.error()", console_error_ok);

    // Test 3: Document Selection API
    print_section("Test 3: Document Selection API");

    // Test getElementById
    std::cout << "\nTesting document.getElementById():" << std::endl;
    std::string test_id_code = R"(
        var elem = document.getElementById('container');
        console.log('Found element with ID:', elem ? elem.tagName : 'null');
    )";
    bool id_ok = script_engine->eval(test_id_code);
    print_test("document.getElementById()", id_ok);

    // Test getElementsByTagName
    std::cout << "\nTesting document.getElementsByTagName():" << std::endl;
    std::string test_tag_code = R"(
        var divs = document.getElementsByTagName('div');
        console.log('Found', divs.length, 'div elements');
    )";
    bool tag_ok = script_engine->eval(test_tag_code);
    print_test("document.getElementsByTagName()", tag_ok);

    // Test getElementsByClassName
    std::cout << "\nTesting document.getElementsByClassName():" << std::endl;
    std::string test_class_code = R"(
        var boxes = document.getElementsByClassName('box');
        console.log('Found', boxes.length, 'elements with class box');
    )";
    bool class_ok = script_engine->eval(test_class_code);
    print_test("document.getElementsByClassName()", class_ok);

    // Test querySelector
    std::cout << "\nTesting document.querySelector():" << std::endl;
    std::string test_query_code = R"(
        var first = document.querySelector('div');
        console.log('querySelector found:', first ? first.tagName : 'null');
        
        var byId = document.querySelector('#title');
        console.log('querySelector #title:', byId ? byId.tagName : 'null');
        
        var byClass = document.querySelector('.text');
        console.log('querySelector .text:', byClass ? byClass.tagName : 'null');
    )";
    bool query_ok = script_engine->eval(test_query_code);
    print_test("document.querySelector()", query_ok);

    // Test querySelectorAll
    std::cout << "\nTesting document.querySelectorAll():" << std::endl;
    std::string test_query_all_code = R"(
        var allDivs = document.querySelectorAll('div');
        console.log('querySelectorAll found', allDivs.length, 'divs');
    )";
    bool query_all_ok = script_engine->eval(test_query_all_code);
    print_test("document.querySelectorAll()", query_all_ok);

    // Test 4: Element API
    print_section("Test 4: Element API");

    std::cout << "\nTesting element properties:" << std::endl;
    std::string test_elem_props = R"(
        var elem = document.getElementById('container');
        if (elem) {
            console.log('Element tagName:', elem.tagName);
            console.log('Element id:', elem.id);
            console.log('Element className:', elem.className);
        }
    )";
    bool elem_props_ok = script_engine->eval(test_elem_props);
    print_test("Element properties (tagName, id, className)", elem_props_ok);

    std::cout << "\nTesting getAttribute/setAttribute:" << std::endl;
    std::string test_attr_code = R"(
        var elem = document.getElementById('title');
        if (elem) {
            var id = elem.getAttribute('id');
            console.log('getAttribute(id):', id);
            
            elem.setAttribute('data-test', 'test-value');
            var dataAttr = elem.getAttribute('data-test');
            console.log('After setAttribute:', dataAttr);
        }
    )";
    bool attr_ok = script_engine->eval(test_attr_code);
    print_test("getAttribute/setAttribute", attr_ok);

    std::cout << "\nTesting textContent:" << std::endl;
    std::string test_text_code = R"(
        var elem = document.getElementById('content');
        if (elem) {
            console.log('Original textContent:', elem.textContent);
            elem.textContent = 'Updated text';
            console.log('Updated textContent:', elem.textContent);
        }
    )";
    bool text_ok = script_engine->eval(test_text_code);
    print_test("textContent property", text_ok);

    std::cout << "\nTesting getChildren:" << std::endl;
    std::string test_children_code = R"(
        var container = document.getElementById('container');
        if (container) {
            var children = container.getChildren();
            console.log('Container has', children.length, 'children');
            for (var i = 0; i < children.length; i++) {
                console.log('  Child', i, ':', children[i].tagName);
            }
        }
    )";
    bool children_ok = script_engine->eval(test_children_code);
    print_test("getChildren()", children_ok);

    // Test 5: Style API
    print_section("Test 5: Computed Style API");

    std::string test_style_code = R"(
        var elem = document.getElementById('title');
        if (elem) {
            var style = elem.getComputedStyle();
            console.log('Computed display:', style.display);
            console.log('Computed fontSize:', style.fontSize);
            console.log('Computed color:', style.color);
        }
    )";
    bool style_ok = script_engine->eval(test_style_code);
    print_test("getComputedStyle()", style_ok);

    // Test 6: HTML with Inline Styles
    print_section("Test 6: Styled Elements");

    const char* html_styled = R"(
        <html>
            <body>
                <h1 style="color: red; font-size: 32px;">Red Header</h1>
                <p style="color: blue; background-color: yellow;">Styled paragraph</p>
                <div style="display: flex; margin: 10px; padding: 20px;">Flex container</div>
            </body>
        </html>
    )";

    std::cout << "\nAnalyzing styled elements:" << std::endl;
    std::string test_styled_code = R"(
        var header = document.getElementsByTagName('h1')[0];
        if (header) {
            var style = header.getComputedStyle();
            console.log('H1 color:', style.color);
            console.log('H1 fontSize:', style.fontSize);
        }
    )";
    bool styled_ok = script_engine->eval(test_styled_code);
    print_test("Styled element queries", styled_ok);

    // Test 7: Event Objects
    print_section("Test 7: Event Objects");

    std::cout << "\nCreating event objects:" << std::endl;
    std::string test_event_code = R"(
        var event = new Event('click');
        console.log('Event type:', event.type);
        console.log('Event bubbles:', event.bubbles);
        
        var mouseEvent = new MouseEvent('mousemove');
        console.log('MouseEvent type:', mouseEvent.type);
        console.log('MouseEvent clientX:', mouseEvent.clientX);
        
        var keyEvent = new KeyboardEvent('keydown');
        console.log('KeyboardEvent type:', keyEvent.type);
        console.log('KeyboardEvent key:', keyEvent.key);
    )";
    bool event_ok = script_engine->eval(test_event_code);
    print_test("Event object creation", event_ok);

    // Test 8: Error Handling
    print_section("Test 8: Error Handling");

    std::cout << "\nTesting graceful error handling:" << std::endl;
    std::string test_error_code = R"(
        var elem = document.getElementById('nonexistent');
        console.log('Query nonexistent:', elem ? 'found' : 'not found (expected)');
        
        var attr = elem ? elem.getAttribute('id') : 'element is null';
        console.log('Checking null safety:', attr ? attr : 'handled correctly');
    )";
    bool error_ok = script_engine->eval(test_error_code);
    print_test("Null/error handling", error_ok);

    // Test 9: Complex Queries
    print_section("Test 9: Complex DOM Traversal");

    std::string test_complex_code = R"(
        var boxes = document.getElementsByClassName('box');
        if (boxes.length > 0) {
            console.log('Processing', boxes.length, 'boxes:');
            for (var i = 0; i < boxes.length; i++) {
                console.log('  Box', i, 'tagName:', boxes[i].tagName);
                console.log('  Box', i, 'textContent:', boxes[i].textContent);
            }
        }
    )";
    bool complex_ok = script_engine->eval(test_complex_code);
    print_test("Complex DOM traversal", complex_ok);

    // Test 10: Combined API Usage
    print_section("Test 10: Combined API Usage");

    std::string test_combined_code = R"(
        function processElements() {
            // Find all elements with class 'box'
            var boxes = document.getElementsByClassName('box');
            console.log('Found', boxes.length, 'boxes');
            
            // For each box, get properties
            for (var i = 0; i < boxes.length; i++) {
                var box = boxes[i];
                var tag = box.tagName;
                var text = box.textContent;
                var style = box.getComputedStyle();
                
                console.log('Box', i);
                console.log('  Tag:', tag);
                console.log('  Text:', text);
                console.log('  Display:', style.display);
            }
        }
        
        processElements();
    )";
    bool combined_ok = script_engine->eval(test_combined_code);
    print_test("Combined API usage", combined_ok);

    // Summary
    print_section("Test Summary");
    
    std::vector<std::pair<std::string, bool>> tests = {
        {"console.log", console_log_ok},
        {"console.warn", console_warn_ok},
        {"console.error", console_error_ok},
        {"getElementById", id_ok},
        {"getElementsByTagName", tag_ok},
        {"getElementsByClassName", class_ok},
        {"querySelector", query_ok},
        {"querySelectorAll", query_all_ok},
        {"Element properties", elem_props_ok},
        {"getAttribute/setAttribute", attr_ok},
        {"textContent", text_ok},
        {"getChildren", children_ok},
        {"getComputedStyle", style_ok},
        {"Styled elements", styled_ok},
        {"Event objects", event_ok},
        {"Error handling", error_ok},
        {"Complex traversal", complex_ok},
        {"Combined API", combined_ok}
    };
    
    int passed = 0;
    for (const auto& test : tests) {
        if (test.second) passed++;
    }
    
    std::cout << "\nPassed: " << passed << "/" << tests.size() << " tests" << std::endl;
    
    if (passed == tests.size()) {
        std::cout << "✓ All tests passed!" << std::endl;
    } else {
        std::cout << "✗ Some tests failed" << std::endl;
    }

    // Cleanup
    dong_view_free(view);
    dong_destroy_context(ctx);

    return passed == tests.size() ? 0 : 1;
}
