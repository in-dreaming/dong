# T16 — host↔wasm 字符串通道与类型 / 编码边界

- **Area**: dong-host（必要时 porffor-fork 配合）
- **性质**: **两段式** — 阶段 0：三个候选协议 spike + **《字符串通道协议》定稿评审**（该文档是 T06/T08/T10/T22 的对标规格）；阶段 1：按协议实现与迁移。gate 未过，其他任务不得铺开新的 `-> str` API
- **优先级**: **P0**（T06/T08/T10 所有 `-> str` API 的前置）
- **依赖**: T05（编码结论；可并行推进，接口按 UTF-8 预设）
- **预估**: 2–3 人天
- **前置阅读**: `docs/developer/porffor/tasks/setup.md` §3（事实 F3/F4）§8

## 背景（事实 F3/F4）

host 向 wasm 返回字符串的现有通道 `PorfforHost::makeByteString`（`dong_porf_host.cpp` L49–71）有三个致命问题：

1. `static size_t bump = 4096`：**函数级 static bump 分配器**，跨模块共享、跨帧从不重置，只增不减；
2. 从固定偏移 4096 开始写，完全不知道 Porffor 模块自己的数据段 / 堆布局，**会覆盖模块数据**；
3. 扩容 `realloc` 后只更新了 host 本地指针，**没有写回模块的 `char** memory` 全局**——模块代码继续用旧指针（use-after-free）。

方向相反的边界同样有洞：Porffor 值是 (f64 value, i32 type) 对，但 **import 只传 f64，类型信息丢失**。`readByteString` 把一切字符串按 bytestring（latin1）解读；脚本里一旦出现中文字面量（Porffor 用 UTF-16 `string` 类型），host 读到的就是乱码。

当前只有 `dong_dom_get_textContent` 一个用户所以侥幸没炸；T06 铺开几十个字符串 API 后必炸。**本任务先定协议、再修实现**。

## 工作内容

### 1. host→wasm 返回协议定稿（三选一，先 spike 再定）

- **a) 拉取式（推荐 spike 首选）**：host 持有当前结果字符串，Porffor 侧通过 `dong_str_len() -> i32` + `dong_str_read(destPtr, maxLen) -> i32`（或逐字符 `dong_str_byte_at(i)`）拉取，prelude 里组装成 JS 字符串。host 完全不写模块内存，无布局假设；代价是一次拷贝 + prelude 样板。
- **b) prelude 预留 buffer**：prelude 声明一个全局定长数组作为接收区，编译期确定其地址；host 写入该固定区域。需验证 Porffor 下「稳定地址的全局 buffer」可行性。
- **c) fork 配合**：2c 暴露模块 allocator（如 `__porf_alloc(size) -> ptr`）供 host 调用，host 分配后写入。最干净但依赖 fork 改动（可与 T15 一并做）。

结果槽生命周期约定与 T08 事件槽 / T10 fetch 槽统一：「本次回调 / 调用期间有效」。

### 2. wasm→host 类型与编码边界

- prelude 保证传给 import 的字符串**一律 UTF-8 bytestring**：提供 `toBytes(s)` / `toUtf8(s)` helper（依赖 T05 确认 string→UTF-8 转换的 builtin 现状，缺则补）；
- host 侧文档明确「import 收到的 str 参数一律 UTF-8 bytestring 指针（len 前缀 + 数据）」，`readByteString` 加防御（长度上界校验）。

### 3. 止血与迁移

- 无论选哪条协议，先修 F3 的三个 bug（写回 memory 指针、去 static、越界防护），避免过渡期踩雷；
- 现有 `dong_dom_get_textContent` 迁移到新协议，作为样例。

## 验收标准

1. 协议定稿文档（含内存图、生命周期、编码约定），供 T06/T08/T10/T15(str 槽) 引用。
2. 往返 fuzz 用例：ASCII / 中文 / emoji / 空串 / 4KB 长串，连续 1000 次调用无内存踩踏（ASan 或 guard byte 校验）。
3. 中文往返：JS 设置中文 textContent → host → JS 读回逐字节一致。
4. F3 三个 bug 消除（代码 review + 用例），setup §3 事实清单 F3/F4 更新。

## 风险

- 方案 b 的「稳定地址全局 buffer」在 Porffor 的内存模型下可能不成立（GC / 内存整理），spike 阶段优先证伪；
- UTF-16 string → UTF-8 的转换成本在热路径（每帧事件字段读取）上要测一下，必要时事件槽字段用纯 ASCII 快路径。
