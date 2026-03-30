注意
- 完成的任务需要标注完成。
- 需要在examples/data/tests下添加测试用例。使用dong/scripts/tools/run_baseline_compare.py进行基准对比验证feature实现是否正确。


# Dong Engine - Web标准对齐 TODO

基于 gap analysis 整理的待办事项，按优先级分组。

定位校准：本引擎更偏**游戏/UI 导向的 Web-like 子集**，优先级以 ROI 为导向——先做会影响主流 UI 框架的 **layout / paint 正确性、表单可用性、滚动/定位、伪元素管线、IME**；高成本低收益/浏览器长尾特性集中放到“暂缓/冻结”。




- [x] CSS 单位补全 (em/rem/calc) — 已实现：css_value_parser 支持 em/rem/calc/min/max/clamp/env/vw/vh/ch 等单位，css_value 中有 resolvePixels 求值。少数边缘路径（如 sticky 定位的 calc 单位）仍有 TODO。
- [x] 伪元素支持 (::before/::after) — 已实现：style_engine 处理 processPseudoElements，painter_text 渲染 ::before/::after 生成内容。
- [x] 外部样式表加载 — 已实现：`<link rel="stylesheet">` 支持本地相对路径与 HTTP(S) 远程 CSS 加载。通过 resource_root + DongFileSystem/DongHttpClient 统一解析。层叠顺序与内联 `<style>` 一致。文件不存在或网络失败时降级并产生 WARN 日志。测试：test_external_stylesheet_basic/order/missing.html。
- [x] JavaScript API 增强 (fetch/async) — 已实现最小子集：`fetch(url)` 返回 Promise，Response 对象提供 ok/status/text()/json()。本地文件同步解析，HTTP(S) 请求通过工作线程异步执行并在主线程 tick 回投 resolve/reject。测试：test_fetch_text_basic/json_basic/reject/timer_order.html。
  - 未实现：POST/body 上传、Headers/Request 完整类、redirect、cookie、CORS、streaming body。