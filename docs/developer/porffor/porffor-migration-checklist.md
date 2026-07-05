# 如何迁移一个测试到 Porffor

迁移判定 checklist（T13 定稿）：

## 前置条件

- [ ] 无跨作用域闭包，或已按 T15 模型（同模块多 export / 状态槽）改写
- [ ] 无 eval / 动态 import / Promise / async（Promise 待 T19）
- [ ] 用到的 API 均已有 host import（对照 T06–T11 清单与 `dong_porffor_prelude.js`）
- [ ] 无 React/Preact（归 T18）
- [ ] 无内联 `onclick=` 等（归 T12，或改 export handler）
- [ ] 无 `execCommand` / Selection API（归 T20 或方案 B C++ 驱动）

## 静态测试（无 `<script>`）

1. 确认纯 CSS/HTML 渲染在 Porffor 构建下通过：`zig build run-html-test -- examples/data/tests/xxx.html`
2. 可选：在 HTML 顶部加 `<!-- porffor: ready -->`（无标记时默认已是 ready）
3. 加入 CI：`node scripts/run-porffor-tests.mjs`

## 带 script 测试

1. 在 `examples/porffor/tests/` 编写 prelude 风格模块（见 `hello_dom.js`）
2. 在 `scripts/porffor_manifest.json` 的 `scripts` 数组注册模块
3. HTML 根元素加 `data-porffor-module="模块名"` 与 `<!-- porffor: ready -->`
4. 移除或保留原 `<script>`（Porffor 路径不执行 QuickJS script；保留可作迁移对照）
5. `node scripts/porffor_compile.mjs` 后 `zig build`
6. 验证：`node scripts/run-porffor-tests.mjs --verbose`

## 多帧 + snippet（T14）

1. 将 snippet 改写为 prelude 风格 `export function afterFrame0() { ... }`
2. 注册 manifest；`html_render_test` 使用 `--call-export-after-frame0 module::afterFrame0`
3. 在 `examples/porffor/tests/<name>.mf.json` 记录 frames / export（供 runner）
4. CE/execCommand 类 snippet 标 `blocked(T20)`，用 C++ 输入注入或延后

## 标记约定

| 标记 | 含义 |
|------|------|
| `<!-- porffor: ready -->` | CI 必须过 |
| `<!-- porffor: pending -->` | 待迁移，不计入失败 |
| `<!-- porffor: blocked(Txx) -->` | 被任务阻塞 |
| `<!-- porffor: dropped(原因) -->` | 永久跳过 |

默认：无 script → ready；有 script → pending。

## 工具

```bash
cd dong
node scripts/porffor_test_tags.test.mjs      # 标记解析单测
node scripts/porffor_test_inventory.mjs      # 生成 T13-test-inventory.md
node scripts/porffor_snippet_inventory.mjs   # 生成 T14-snippet-inventory.md
node scripts/run-porffor-tests.mjs           # CI Porffor job
```
