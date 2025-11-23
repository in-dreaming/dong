/**
 * 【缺口 3】JS 代码执行返回值演示
 * 
 * 功能：
 * - dong_view_eval_return() 执行 JS 代码并返回结果
 * - 支持返回不同类型：数字、字符串、布尔值、对象 JSON
 * - 实现 JS 与 C 的双向数据通信
 */

#include "dong.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>

void save_ppm(const char* filename, uint8_t* buffer, int width, int height) {
    FILE* f = fopen(filename, "wb");
    if (!f) return;
    fprintf(f, "P6\n%d %d\n255\n", width, height);
    fwrite(buffer, 1, width * height * 4, f);
    fclose(f);
}

int main() {
    printf("=== Dong Engine - JS Return Value Demo ===\n\n");
    
    // 创建上下文和视图
    dong_context_t* ctx = dong_create_context();
    dong_view_t* view = dong_view_create(ctx, 800, 600);
    
    // 加载初始 HTML
    const char* html = R"(
        <html>
        <head><title>JS Return Value Demo</title></head>
        <body style="background-color: #f0f0f0; padding: 20px;">
            <h1 style="color: #333;">JS Return Value Demo</h1>
            <div id="content" style="background-color: #fff; padding: 15px; border-radius: 5px;">
                <p>JavaScript return values are captured and sent back to C code</p>
            </div>
        </body>
        </html>
    )";
    
    dong_view_load_html(view, html);
    dong_view_update(view);
    printf("Initial rendering done.\n\n");
    
    // 演示 1: 返回数字
    printf("Demo 1: Returning numbers\n");
    const char* result = dong_view_eval_return(view, "2 + 3");
    printf("  2 + 3 = %s\n", result);
    
    result = dong_view_eval_return(view, "3.14159");
    printf("  3.14159 = %s\n", result);
    
    result = dong_view_eval_return(view, "Math.sqrt(16)");
    printf("  Math.sqrt(16) = %s\n", result);
    printf("\n");
    
    // 演示 2: 返回字符串
    printf("Demo 2: Returning strings\n");
    result = dong_view_eval_return(view, "'Hello from JavaScript'");
    printf("  String result: %s\n", result);
    
    result = dong_view_eval_return(view, "'JavaScript ' + 'is ' + 'awesome'");
    printf("  Concatenated: %s\n", result);
    printf("\n");
    
    // 演示 3: 返回布尔值
    printf("Demo 3: Returning booleans\n");
    result = dong_view_eval_return(view, "true");
    printf("  true = %s\n", result);
    
    result = dong_view_eval_return(view, "false");
    printf("  false = %s\n", result);
    
    result = dong_view_eval_return(view, "5 > 3");
    printf("  5 > 3 = %s\n", result);
    printf("\n");
    
    // 演示 4: DOM 查询和计算
    printf("Demo 4: DOM queries with return values\n");
    const char* dom_query = R"(
        let elements = document.getElementsByTagName('div');
        'Found ' + elements.length + ' div elements'
    )";
    result = dong_view_eval_return(view, dom_query);
    printf("  Query result: %s\n", result);
    
    const char* style_query = R"(
        let elem = document.getElementById('content');
        'Content element padding: ' + (elem ? elem.style.padding || 'inherited' : 'not found')
    )";
    result = dong_view_eval_return(view, style_query);
    printf("  Style query: %s\n", result);
    printf("\n");
    
    // 演示 5: 计算和逻辑
    printf("Demo 5: Complex calculations\n");
    const char* calc_code = R"(
        let values = [1, 2, 3, 4, 5];
        let sum = 0;
        for (let i = 0; i < values.length; i++) {
            sum += values[i];
        }
        'Sum of [1,2,3,4,5] = ' + sum
    )";
    result = dong_view_eval_return(view, calc_code);
    printf("  %s\n", result);
    
    const char* obj_code = R"(
        let obj = {name: 'Dong', version: '1.0', features: ['Layout', 'Render', 'Script']};
        JSON.stringify(obj)
    )";
    result = dong_view_eval_return(view, obj_code);
    printf("  Object: %s\n", result);
    printf("\n");
    
    // 演示 6: 更新 DOM 并返回结果
    printf("Demo 6: DOM modification with return value\n");
    const char* update_code = R"(
        let elem = document.getElementById('content');
        if (elem) {
            elem.style.backgroundColor = '#fff3cd';
            elem.style.color = '#856404';
            elem.textContent = 'Content updated by JavaScript!';
            'Successfully updated element'
        } else {
            'Element not found'
        }
    )";
    result = dong_view_eval_return(view, update_code);
    printf("  Update result: %s\n", result);
    
    // 渲染和保存
    printf("\nRendering updated page...\n");
    dong_view_update(view);
    
    uint8_t* buffer = (uint8_t*)dong_view_get_pixel_buffer(view);
    if (buffer) {
        save_ppm("/tmp/js_return_value_demo.ppm", buffer, 800, 600);
        printf("Saved to /tmp/js_return_value_demo.ppm\n");
    }
    
    // 清理
    dong_view_destroy(view);
    dong_destroy_context(ctx);
    
    printf("\n✅ JS Return Value Demo completed!\n");
    printf("   - Returned numbers, strings, and booleans\n");
    printf("   - Performed DOM queries with results\n");
    printf("   - Calculated complex values\n");
    printf("   - Modified DOM and returned status\n");
    
    return 0;
}
