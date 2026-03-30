注意
- 完成的任务需要标注完成。
- 需要在examples/data/tests下添加测试用例。使用dong/scripts/tools/run_baseline_compare.py进行基准对比验证feature实现是否正确。


# Dong Engine - Web标准对齐 TODO

基于 gap analysis 整理的待办事项，按优先级分组。

定位校准：本引擎更偏**游戏/UI 导向的 Web-like 子集**，优先级以 ROI 为导向——先做会影响主流 UI 框架的 **layout / paint 正确性、表单可用性、滚动/定位、伪元素管线、IME**；高成本低收益/浏览器长尾特性集中放到“暂缓/冻结”。




CSS 单位补全 (em/rem/calc)
JavaScript API 增强 (fetch/async)
伪元素支持 (::before/::after)
外部样式表加载