# T16 — Host↔Wasm 字符串通道协议（定稿）

> **状态**: 已定稿（2026-07-04）  
> **对标**: T05 UTF-8 约定；T08 事件槽 / T10 fetch 槽生命周期  
> **实现**: `dong/src/script/porffor/dong_porf_host.cpp`、`dong_porffor_prelude.js`

## 1. 选型结论

| 方案 | 结论 | 理由 |
|------|------|------|
| **A 拉取式** | **采用** | host 不写模块内存；无布局假设；2c 下无 fork 依赖 |
| B prelude 定长 buffer | **否决** | spike：Porffor 2c 全局字面量地址虽稳定，但容量固定、无法承载 T10 JSON / 长 fetch body；与「host 写固定区」同类风险 |
| C fork `__porf_alloc` | **defer** | 最干净但需 fork 改动；可与 T15 后续一并评估；当前 A 已满足 T06/T08 |

**选用 A**：host 持有 **result slot**（UTF-8 `std::string`），wasm 通过 `dong_str_len` + `dong_str_read` / `dong_str_byte_at` 拉取；prelude 组装 Porffor 字符串。

## 2. 内存与数据流

### 2.1 host→wasm（返回值 / `-> str` API）

```
  ┌─────────────┐     dong_dom_get_textContent(nodeId)      ┌──────────────────┐
  │  C++ Host   │ ────────────────────────────────────────►│ result_slot_     │
  │ PorfforHost │         (void, 无 f64 指针返回)            │ UTF-8 std::string│
  └─────────────┘                                          └────────┬─────────┘
        ▲                                                             │
        │ dong_str_len() → byte length                                │
        │ dong_str_read(dest, max) → memcpy into module memory        │
        │ dong_str_byte_at(i) → byte 0..255 (optional)                ▼
  ┌─────────────┐                                          ┌──────────────────┐
  │ wasm module │ ◄── pullHostString() in prelude ──────────│ Porffor string   │
  └─────────────┘     UTF-8 decode → string type           └──────────────────┘
```

- host **永不**向模块 memory 做 bump 分配或 `realloc` 写回。
- `result_slot_` 在每次「产出字符串的 host import」返回前写入；同 T08/T10：**本次 import 调用链 / 回调期间有效**，嵌套调用由 host 栈式保存（后续 T08 扩展）。

### 2.2 wasm→host（import 参数）

```
  JS 值 (string UTF-16 / bytestring latin1)
        │
        ▼  toUtf8(s)  [prelude 强制]
  UTF-8 bytestring  (u32 len @ ptr, bytes @ ptr+4)
        │
        ▼  f64(ptr)  — 类型 tag 丢失，host 按 UTF-8 解读
  readByteString(ptr) → std::string (UTF-8 bytes)
```

- **契约**：凡传入 host import 的字符串参数，prelude **必须**经 `toUtf8()`，保证 UTF-8 bytestring。
- host `readByteString`：校验 `ptr + 4 + len ≤ memory_pages × 65536`，`len ≤ DONG_PORF_MAX_IMPORT_STR`（默认 4 MiB）。

## 3. Import 表（新增 / 变更）

| Import | 参数 | 返回 | 说明 |
|--------|------|------|------|
| `dong_str_len` | — | f64 | `result_slot_` 字节长度 |
| `dong_str_read` | dest_ptr, max_len | f64 | 拷贝至多 `max_len` 字节到模块 memory；返回实际拷贝数 |
| `dong_str_byte_at` | index | f64 | 第 `index` 字节 0–255；越界返回 -1 |
| `dong_dom_get_textContent` | node_id | **void**（原 1 返回已废弃） | 写入 `result_slot_` |

## 4. Prelude 公共 helper

| 函数 | 方向 | 说明 |
|------|------|------|
| `toUtf8(s)` / `toBytes(s)` | wasm→host | UTF-16/bytestring → UTF-8 bytestring |
| `pullHostString()` | host→wasm | `dong_str_len` + `dong_str_byte_at` 拉取并 UTF-8 解码为 `string` |
| `getTextContent(nodeId)` | 封装 | `dong_dom_get_textContent` + `pullHostString()` |

`setTextContent` / `getElementById` / `dongLog` / `addEventListener` 等现有 helper 在传 host 前对字符串参数调用 `toUtf8()`。

## 5. 编码约定（与 T05 对齐）

- **边界编码**：UTF-8。
- **Porffor 内部**：UTF-16 `string` / latin1 `bytestring` 不变。
- **中文 / emoji**：脚本侧用 `string`；过 import 前 `toUtf8`；host 读回后 `pullHostString` 还原为 `string`。

## 6. 生命周期（与 T08/T10 统一）

| 槽 | 写入时机 | 有效范围 |
|----|----------|----------|
| `result_slot_` | 任意 `-> str` host import | 从该 import 返回到 wasm 侧 `pullHostString()` 完成；嵌套 import 时 host 压栈 |
| stage 槽 | `dong_stage_*` | commit 前（既有约定） |

## 7. F3 / F4 处置

| 事实 | 处置 |
|------|------|
| F3 `makeByteString` static bump / 覆盖数据 / 不写回 memory | **停用**于 `dong_dom_get_textContent`；`makeByteString` 改为写 `result_slot_` 并返回 0（过渡 stub，禁止写模块 memory） |
| F4 import 丢类型 / latin1 误读 | prelude 强制 `toUtf8`；`readByteString` 按 UTF-8 字节序列 + 边界校验 |

## 8. 后续任务引用

- **T06/T08/T10**：所有 `-> str` API 遵循 §2.1 result slot + prelude `pullHostString()`。
- **T15 `dong_state_set_str/get_str`**：字符串槽与 `result_slot_` 共用 pull 语义或独立 slot index（T08 定稿）。
