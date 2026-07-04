# T03 — 2c 输出卫生：符号冲突 / static / NaN / --quiet

- **Area**: porffor-fork（`dong/third_party/porffor` 子模块，分支 `enjin_porffor`）
- **性质**: 纯实现型 — 对标物：Dong 现有三个文本后处理的效果（`porffor_compile.mjs`）；修完后处理可删且行为不变
- **优先级**: P1
- **依赖**: 无（与 T01/T02 并行安全，改动区域小心重叠）
- **预估**: 1–2 人天
- **前置阅读**: `docs/developer/porffor/tasks/setup.md` §3 §5

## 背景

Dong 把多个 2c 产物（每个 Porffor 模块一个 .c）与引擎静态链接，目前靠**构建期文本后处理**修补 2c 输出的四类问题。本任务把修补上移到 2c 本身，删除后处理。

## 子任务

### 3.1 符号全局污染 / duplicate symbol

- 现象一：2c 生成的辅助函数（如名为 `log` 的函数）与 `math.h` 的 `log` 冲突（Dong 已改名 `dongLog` 绕过）。
- 现象二：多个 .c 产物含同名 helper（`prepend` 里的公共函数、`CMemFuncs` 等），链接时 duplicate symbol。
- 修复：2c 生成的所有非 export 函数与文件级变量一律 `static`；或提供 `--2c-prefix=<str>` 给所有全局符号加前缀。推荐两者都做（static 为主，prefix 兜底给必须外露的符号如 `main`/export）。
- 注意 `#main` 的输出（2c.js ~L354：生成 `int main(...)` 或 `user_main`），多模块场景每个模块都有 main —— 检查 fork 现有处理，必要时提供 `--2c-entry-name=<name>`。

### 3.2 `NaN` / `Infinity` 字面量

- 2c 头部已有 `const f64 NaN = 0e+0/0e+0;`、`const f64 Infinity = ...`（2c.js L32–33），但某些编译器/模式（如 MSVC、`-Werror`、C++ 编译单元）下该写法失败或与宏冲突，Dong 做了后处理替换。
- 修复：改为编译器无关写法——`#include <math.h>` 后用 `NAN`/`INFINITY` 宏定义内部名（如 `__porf_nan`），字面量输出处统一引用内部名；或至少把 `NaN`/`Infinity` 重命名为不易冲突的内部标识符。**先在 Dong 的实际工具链（clang / clang-cl / MSVC）上复现失败样例再修**。

### 3.3 编译 stdout 噪声

- `porf c` 会打印计时等日志，污染 Dong 构建输出。
- 修复：新增 `--quiet` pref（`compiler/prefs.js` 自动映射 `--quiet` → `Prefs.quiet`），在 `compiler/log.js` / 计时输出处统一 gate。保证 `porf c in.js out.c --quiet` 时 stdout 无任何输出、stderr 只留 error。

## 验收标准

1. 两个不同 JS 模块 `porf c` 后与一个 `main.c` 测试桩共同用 clang 编译链接，零 duplicate symbol、零与 libc/math.h 冲突。
2. 含 `NaN`/`Infinity`/`-Infinity` 字面量及运算的脚本，2c 产物在 clang 与 clang-cl（如可用）下编译通过、运行结果与 node wasm 路径一致。
3. `--quiet` 生效；不加时输出不变。
4. Dong 侧三个文本后处理（改名 log、替换 NaN、加 static）在验证后可删除（在完成记录中注明验证方式）。
5. 复现与验证脚本放 `docs/developer/porffor/tasks/repro/t03/`。

## 风险

- `static` 化要跳过真正需要导出的符号（export 函数、`main`）。
- 与 T02 都会改 `prepend` 相关代码，认领时相互知会，避免冲突。
