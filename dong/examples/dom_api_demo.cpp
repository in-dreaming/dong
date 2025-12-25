/**
 * 【缺口 1】DOM 创建 API 演示
 * 
 * 功能：
 * - document.createElement() 创建新元素
 * - element.appendChild() 添加子元素
 * - element.removeChild() 移除子元素
 * - 使用 JS 动态构建 DOM 树
 */

#include "dong.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>

void save_ppm(const char* filename, uint8_t* buffer, int width, int height) {
    FILE* f = fopen(filename, "wb");
    if (!f) return;
    fprintf(f, "P6\n%d %d\n255\n", width, height);
    fwrite(buffer, 1, width * height * 4, f);  // RGBA -> write as-is
    fclose(f);
}

int main() {
    printf("=== Dong Engine - DOM API Demo ===\n\n");
    
    // 创建上下文和视图
    dong_context_t* ctx = dong_create_context();
    dong_view_t* view = dong_view_create(ctx, 800, 600);
    
    // 1. 加载初始 HTML
    const char* html = R"(
        <html>
        <head><title>DOM API Demo</title></head>
        <body style="background-color: #f0f0f0; padding: 20px;">
            <h1 style="color: #333;">Dynamic DOM Creation</h1>
            <div id="container" style="background-color: #fff; padding: 15px; border-radius: 5px;">
            </div>
        </body>
        </html>
    )";
    
    dong_view_load_html(view, html);
    dong_view_update(view);
    
    // 2. 使用 JS 动态创建元素
    printf("Creating elements with JavaScript...\n");
    
    const char* js_code = R"(
        // 获取容器
        let container = document.getElementById('container');
        console.log('Container found:', container ? container.tagName : 'NOT FOUND');
        
        // 创建标题
        let title = document.createElement('h2');
        title.textContent = 'Items List';
        container.appendChild(title);
        console.log('Added title');
        
        // 创建列表项
        for (let i = 1; i <= 5; i++) {
            let item = document.createElement('div');
            item.setAttribute('class', 'list-item');
            item.textContent = 'Item #' + i;
            container.appendChild(item);
        }
        console.log('Added 5 items');
        
        // 验证子元素数量
        let children = container.getChildren();
        console.log('Container now has:', children.length, 'children');
        
        'Success: Created 6 elements (1 title + 5 items)'
    )";
    
    const char* result = dong_view_eval_return(view, js_code);
    printf("JS Result: %s\n\n", result);
    
    // 3. 测试 removeChild
    printf("Testing removeChild...\n");
    const char* remove_code = R"(
        let container = document.getElementById('container');
        let children = container.getChildren();
        console.log('Before remove:', children.length, 'children');
        
        if (children.length > 0) {
            let lastChild = children[children.length - 1];
            container.removeChild(lastChild);
            console.log('Removed last child');
        }
        
        'Removed 1 element'
    )";
    
    result = dong_view_eval_return(view, remove_code);
    printf("JS Result: %s\n\n", result);
    
    // 4. 渲染和保存
    printf("Rendering...\n");
    dong_view_update(view);
    
    uint8_t* buffer = (uint8_t*)dong_view_get_pixel_buffer(view);
    if (buffer) {
        save_ppm("/tmp/dom_api_demo.ppm", buffer, 800, 600);
        printf("Saved to /tmp/dom_api_demo.ppm\n");
    }
    
    // 清理
    dong_view_destroy(view);
    dong_destroy_context(ctx);
    
    printf("\n✅ DOM API Demo completed!\n");
    printf("   - Created elements with document.createElement()\n");
    printf("   - Added elements with appendChild()\n");
    printf("   - Removed elements with removeChild()\n");
    
    return 0;
}
