
html
https://html.spec.whatwg.org/multipage/?utm_source=chatgpt.com
https://developer.mozilla.org/en-US/docs/Web

css
https://developer.mozilla.org/en-US/docs/Web/CSS/Reference?utm_source=chatgpt.com

dom
https://dom.spec.whatwg.org/


# 📘 **HTML 规范（元素 / 属性 / 全集）**

👉 **HTML Living Standard（权威规范）**

* WHATWG 官方 HTML 标准，是现代浏览器实现的基础规范
* 包含所有 HTML 元素、属性以及 DOM 语义
  🔗 [https://html.spec.whatwg.org/multipage/](https://html.spec.whatwg.org/multipage/)
  （完整标准文档，Living-Standard，即持续更新的规范）([HTML Living Standard][1])

👉 **MDN HTML 元素参考（易读版）**

* 列出标准 HTML 元素、属性说明和示例
* 适合对照实现支持项
  🔗 [https://developer.mozilla.org/en-US/docs/Web/HTML/Reference/Elements](https://developer.mozilla.org/en-US/docs/Web/HTML/Reference/Elements)
  （每个元素都有属性、用途、交互说明）([MDN Web Docs][2])

👉 **MDN HTML 主参考（整体入口）**
🔗 [https://developer.mozilla.org/en-US/docs/Web/HTML](https://developer.mozilla.org/en-US/docs/Web/HTML)
（左侧可以浏览所有元素分类 + 属性 + 语义）([MDN Web Docs][3])

💡 **备注**

* WHATWG 规范是 “official authoritative spec” — 浏览器以此为实现依据。([Stack Overflow][4])
* MDN 是社区整理的规范参考，适合人类阅读。

---

# 🎨 **CSS 规范（属性 / 选择器 / 约定）**

👉 **MDN CSS Reference（权威属性表）**

* 按字母索引所有标准 CSS 属性、伪类、伪元素、@规则
* 包含语义说明、值类型、示例等
  🔗 [https://developer.mozilla.org/en-US/docs/Web/CSS/Reference](https://developer.mozilla.org/en-US/docs/Web/CSS/Reference)
  （浏览所有可用 CSS 属性实现可能时非常有用）([MDN Web Docs][5])

👉 **MDN CSS 规范主页（模块入口）**
🔗 [https://developer.mozilla.org/zh-CN/docs/Web/CSS](https://developer.mozilla.org/zh-CN/docs/Web/CSS)
（按大模块浏览器布局、颜色、动画等文档）([MDN Web Docs][6])

💡 **注意**
CSS 标准不是单一文档，而是很多模块，比如布局、flexbox、grid、滤镜、动画等。MDN 的 Reference 链接就覆盖了全部。([MDN Web Docs][5])

---

# 🔧 **DOM（文档对象模型）规范**

👉 **DOM Living Standard（WHATWG）**

* 描述浏览器如何表示 HTML 文档结构 / 属性 / 事件 / API
* 这部分规范定义了 `document.getElementById` / `addEventListener` 等行为
  🔗 [https://dom.spec.whatwg.org/](https://dom.spec.whatwg.org/)
  （按照标准实现 DOM 树 & 事件模型）([W3C][7])

👉 **DOM 维基百科参考简介**

* 用来了解 DOM 标准的历史与演进
  🔗 [https://en.wikipedia.org/wiki/Document_Object_Model](https://en.wikipedia.org/wiki/Document_Object_Model)
  （有 DOM 版本演进的背景，可辅助理解为何 DOM API 这样设计）([维基百科][8])

---

# 🧠 **为什么这些标准是权威且必看的？**

✅ **WHATWG HTML Standard**
是当前浏览器实现中 HTML 的真实规范 — 比起旧的 “HTML5” 规范更现代，更详细。([HTML Living Standard][1])

✅ **MDN 作为辅助资料**
虽然不是官方标准源，但文档内容由浏览器厂商工程师维护，条目齐全且易于查阅。([MDN Web Docs][5])

✅ **CSS Reference** 覆盖所有标准 CSS 属性与语义，适合作为实现对照清单。([MDN Web Docs][5])

⚠️ **注意**
真正的规范文本很冗长（HTML + DOM + CSS 是多个大型规范），不适合全部原文照搬，但可以：

* 逐条对照元素/属性/事件是否需要支持
* 根据规范定义实现行为边界（例如事件冒泡机制）

---

## 📌 推荐用法

你可以把这些链接作为**权威规范对照清单**：

| 规范                   | 用途                         |
| -------------------- | -------------------------- |
| WHATWG HTML Standard | 全面标准 HTML 元素 + 属性 + DOM 行为 |
| MDN HTML Reference   | 人类可读的标签 & 属性解析             |
| MDN CSS Reference    | 全标准 CSS 属性清单               |
| DOM Standard         | 动态结构与事件 / JS 操作规范          |

每当你想判断某个机制“是否标准”，可以：

1. 查 HTML tag 与属性 → WHATWG / MDN
2. 查 DOM / JS 行为 → DOM spec
3. 查 CSS 属性 → MDN CSS Reference

这些链接一起就构成了“可以一一对照”的规范手册。

---

如需要，我也可以帮你 **把你上面的 UI 规范草案自动和这些标准建立对照表**，生成一个对照矩阵（比如：规范 property → 是否支持 → 备注）。只要你告诉我输出格式（表格 / JSON / 文档），我就可以帮你生成。

[1]: https://html.spec.whatwg.org/multipage/?utm_source=chatgpt.com "Living Standard - HTML - WhatWG"
[2]: https://developer.mozilla.org/en-US/docs/Web/HTML/Reference/Elements?utm_source=chatgpt.com "HTML elements reference - MDN Web Docs - Mozilla"
[3]: https://developer.mozilla.org/en-US/docs/Web/HTML?utm_source=chatgpt.com "HTML: HyperText Markup Language - MDN Web Docs"
[4]: https://stackoverflow.com/questions/6825713/html5-w3c-vs-whatwg-which-gives-the-most-authoritative-spec?utm_source=chatgpt.com "HTML5: W3C vs WHATWG. Which gives the most authoritative spec?"
[5]: https://developer.mozilla.org/en-US/docs/Web/CSS/Reference?utm_source=chatgpt.com "CSS reference - CSS | MDN"
[6]: https://developer.mozilla.org/zh-CN/docs/Web/CSS?utm_source=chatgpt.com "CSS：层叠样式表 | MDN"
[7]: https://www.w3.org/html/?utm_source=chatgpt.com "W3C HTML"
[8]: https://en.wikipedia.org/wiki/Document_Object_Model?utm_source=chatgpt.com "Document Object Model"
