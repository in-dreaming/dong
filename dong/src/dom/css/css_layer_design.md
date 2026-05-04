# CSS @layer 级联层设计文档

## 数据结构设计

### LayerRule 结构
```cpp
// @layer 规则表示
struct LayerRule {
    std::string name;        // 层名称（空字符串表示匿名层）
    std::vector<CSSRule> rules; // 层内的CSS规则
    int declaration_order;  // 声明顺序（用于优先级排序）
    bool is_predeclared;     // 是否为预声明层（没有规则）
};
```

### LayerContext 结构
```cpp
// 层上下文管理
struct LayerContext {
    std::vector<LayerRule> layers; // 所有层（按声明顺序）
    std::unordered_map<std::string, int> layer_order_map; // 层名到优先级的映射

    // 未分层规则的优先级最高
    std::vector<CSSRule> unlayered_rules;
};
```

## 层优先级规则

根据CSS Cascading Level 5规范：

1. **优先级顺序**：未分层样式 > 后声明的层 > 先声明的层
2. **层声明顺序**：
   - `@layer A, B, C;` 声明A < B < C优先级
   - 后续的`@layer B { ... }`会添加到已声明的层B中
3. **匿名层**：每个匿名层都是独立的，按声明顺序排序

## 级联排序算法

### 层优先级计算
```
LayerPriority = (layer_order, specificity, source_order)
```

### 规则排序流程
1. 将所有规则按层分组
2. 按层优先级排序：未分层 > 高优先级层 > 低优先级层
3. 在每个层内按传统级联排序（specificity + source_order）

## 集成到现有架构

### CSSParser 扩展
- 添加 `parseLayerRules()` 方法
- 修改 `parse()` 方法处理@layer规则
- 支持层内嵌套其他@规则（@media、@keyframes等）

### StyleEngine 扩展
- 在 `applyMatchingRules()` 中集成层优先级
- 修改规则排序逻辑以考虑层优先级
- 保持向后兼容性（无层时使用传统级联）

## 测试用例设计

### 基本层优先级测试
```css
@layer base { div { color: red; } }
@layer theme { div { color: blue; } }
div { color: green; } /* 未分层，优先级最高 */
```

### 层声明顺序测试
```css
@layer A, B, C;  /* 声明顺序：A < B < C */
@layer C { div { color: red; } }  /* 最高优先级 */
@layer A { div { color: blue; } } /* 最低优先级 */
```

### 匿名层测试
```css
@layer { div { color: red; } }  /* 匿名层1 */
@layer { div { color: blue; } } /* 匿名层2 */
```