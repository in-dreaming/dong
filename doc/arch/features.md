

# direct 
dong.draw*系列
dong.renderText() + dong.drawRect() overlay API 绕过 DOM，但如果要让 DOM 路径也跑得快，引擎侧可以优化：

setInlineStyleProperty 延迟重建 style 字符串 — 只在 getAttribute("style") 时才拼接
inline-style-only 的脏标记 — 跳过 CSS 选择器匹配，只重应用内联样式
setTextContent 复用已有文本节点 — 避免 shared_ptr alloc/dealloc

# scene graph
所有节点使用 position: absolute + 显式像素坐标，去掉 width: 100%、margin、display: inline-block、position: relative 等 Scene Compiler 无法处理的特性。
meta name="dong-render-mode" content="scene">


# commands_regenerated_this_tick_ 