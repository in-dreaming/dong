/**
 * 【缺口 2】样式动态修改演示
 * 
 * 功能：
 * - element.style.property 读写样式属性
 * - element.classList.add/remove/toggle() 操作 CSS 类
 * - 动态改变元素外观
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
    printf("=== Dong Engine - Style Modification Demo ===\n\n");
    
    // 创建上下文和视图
    dong_context_t* ctx = dong_create_context();
    dong_view_t* view = dong_view_create(ctx, 800, 600);
    
    // 加载初始 HTML
    const char* html = R"(
        <html>
        <head><title>Style Modification Demo</title></head>
        <body style="background-color: #f0f0f0; padding: 20px;">
            <h1 style="color: #333;">Style Modification Demo</h1>
            <div id="box1" style="width: 150px; height: 100px; background-color: #ff6b6b; margin: 10px; padding: 10px;">
                Box 1
            </div>
            <div id="box2" style="width: 150px; height: 100px; background-color: #4ecdc4; margin: 10px; padding: 10px;">
                Box 2
            </div>
        </body>
        </html>
    )";
    
    dong_view_load_html(view, html);
    dong_view_update(view);
    printf("Initial rendering done.\n\n");
    
    // 演示 1: element.style 属性修改
    printf("Demo 1: Modifying element.style properties\n");
    const char* style_code = R"(
        let box1 = document.getElementById('box1');
        console.log('Box1 original color:', box1.style.backgroundColor);
        
        // 修改背景色
        box1.style.backgroundColor = '#4ecdc4';
        console.log('Box1 new color:', box1.style.backgroundColor);
        
        // 修改字体大小
        box1.style.fontSize = 18;
        
        // 修改文本颜色
        box1.style.color = '#ffffff';
        
        'Style properties updated'
    )";
    
    const char* result = dong_view_eval_return(view, style_code);
    printf("Result: %s\n\n", result);
    
    // 演示 2: classList 操作
    printf("Demo 2: Using classList.add/remove/toggle\n");
    const char* classlist_code = R"(
        let box2 = document.getElementById('box2');
        console.log('Box2 initial class:', box2.className);
        
        // 添加类
        box2.classList.add('highlight');
        console.log('Added highlight class');
        
        // 添加另一个类
        box2.classList.add('active');
        console.log('Added active class');
        
        // 移除类
        box2.classList.remove('highlight');
        console.log('Removed highlight class');
        
        // 切换类
        let wasToggled = box2.classList.toggle('active');
        console.log('Toggle returned:', wasToggled);
        
        console.log('Final class:', box2.className);
        
        'ClassList operations completed'
    )";
    
    result = dong_view_eval_return(view, classlist_code);
    printf("Result: %s\n\n", result);
    
    // 演示 3: 组合修改
    printf("Demo 3: Combined style and class modifications\n");
    const char* combined_code = R"(
        let box1 = document.getElementById('box1');
        let box2 = document.getElementById('box2');
        
        // 修改多个属性
        box1.style.opacity = 0.7;
        box1.style.borderRadius = 10;
        box1.classList.add('modified');
        
        box2.style.display = 'block';
        box2.style.fontSize = 20;
        box2.classList.add('styled');
        
        console.log('Box1 opacity:', box1.style.opacity);
        console.log('Box2 fontSize:', box2.style.fontSize);
        
        'Combined modifications complete'
    )";
    
    result = dong_view_eval_return(view, combined_code);
    printf("Result: %s\n\n", result);
    
    // 渲染和保存
    printf("Rendering updated page...\n");
    dong_view_update(view);
    
    uint8_t* buffer = (uint8_t*)dong_view_get_pixel_buffer(view);
    if (buffer) {
        save_ppm("/tmp/style_modification_demo.ppm", buffer, 800, 600);
        printf("Saved to /tmp/style_modification_demo.ppm\n");
    }
    
    // 清理
    dong_view_destroy(view);
    dong_destroy_context(ctx);
    
    printf("\n✅ Style Modification Demo completed!\n");
    printf("   - Modified element.style properties\n");
    printf("   - Used classList.add/remove/toggle\n");
    printf("   - Applied combined style and class changes\n");
    
    return 0;
}
