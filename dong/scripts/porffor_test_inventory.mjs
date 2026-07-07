#!/usr/bin/env node
/**
 * Generate docs/developer/porffor/tasks/T13-test-inventory.md from examples/data/tests/*.html
 */
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { dongRoot } from './porffor_paths.mjs';
import {
  parsePorfforTagFromHtml,
  classifyScriptTest,
  extractPorfforModule,
  detectEvalSnippetRef,
} from './porffor_test_tags.mjs';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const repoRoot = path.resolve(dongRoot, '..');
const testsDir = path.join(dongRoot, 'examples', 'data', 'tests');
const outPath = path.join(repoRoot, 'docs', 'developer', 'porffor', 'tasks', 'T13-test-inventory.md');

const files = fs.readdirSync(testsDir).filter((f) => f.endsWith('.html')).sort();

const rows = [];
const counts = { ready: 0, pending: 0, blocked: 0, dropped: 0 };
const buckets = {};

for (const file of files) {
  const html = fs.readFileSync(path.join(testsDir, file), 'utf8');
  const tag = parsePorfforTagFromHtml(html);
  counts[tag.status] = (counts[tag.status] ?? 0) + 1;
  const hasScript = /<script[\s>]/i.test(html);
  const mod = extractPorfforModule(html);
  let migration = hasScript ? classifyScriptTest(html, file).bucket : 'static-render';
  if (detectEvalSnippetRef(html).usesEvalSnippet) migration = 'T14-snippet';
  buckets[migration] = (buckets[migration] ?? 0) + 1;
  rows.push({
    file,
    status: tag.status,
    explicit: tag.explicit ? 'yes' : 'no',
    reason: tag.reason ?? '',
    hasScript: hasScript ? 'yes' : 'no',
    module: mod ?? '',
    migration,
  });
}

const total = files.length;
const readyPct = ((counts.ready ?? 0) / total * 100).toFixed(1);

let md = `# T13 — 全量测试迁移盘点

> 自动生成：\`node dong/scripts/porffor_test_inventory.mjs\`（勿手改统计表，改生成器或 HTML 标记后重跑）

## 摘要

| 指标 | 值 |
|------|-----|
| 测试总数 | ${total} |
| ready | ${counts.ready ?? 0} (${readyPct}%) |
| pending | ${counts.pending ?? 0} |
| blocked | ${counts.blocked ?? 0} |
| dropped | ${counts.dropped ?? 0} |
| 无 script（静态渲染） | ${rows.filter((r) => r.hasScript === 'no').length} |
| 有 script | ${rows.filter((r) => r.hasScript === 'yes').length} |

## blocked 分批（显式标记）

| 阻塞任务 | 数量 |
|----------|------|
${Object.entries(
  rows
    .filter((r) => r.status === 'blocked' && r.reason)
    .reduce((acc, r) => {
      acc[r.reason] = (acc[r.reason] ?? 0) + 1;
      return acc;
    }, {}),
)
  .sort((a, b) => b[1] - a[1])
  .map(([k, v]) => `| blocked(${k}) | ${v} |`)
  .join('\n')}

| dropped 原因 | 数量 |
|-------------|------|
${Object.entries(
  rows
    .filter((r) => r.status === 'dropped' && r.reason)
    .reduce((acc, r) => {
      acc[r.reason] = (acc[r.reason] ?? 0) + 1;
      return acc;
    }, {}),
)
  .map(([k, v]) => `| ${k} | ${v} |`)
  .join('\n') || '| — | 0 |'}

详见 [T13-pending-batches.md](./T13-pending-batches.md)。

## 构建组织选型（阶段 0 定稿）

**选定方案：a) 单 test runner 链接全部 Porffor 模块**

| 方案 | 说明 | 结论 |
|------|------|------|
| a) 单 runner + 全量 registry | manifest → \`porffor_compile.mjs\` → \`generated/porffor/registry.c\`；\`html_render_test\` / CI 一次链接 | **采用** — 实现简单，当前模块数 <20，链接增量可接受 |
| b) 按目录分组多 registry | 多二进制，按 tests 子集跑 | 暂缓 — 模块上百后再评估 |
| c) 每测试独立编译 + 缓存 | 增量友好，管线复杂 | 不采用 — 维护成本高 |

**实测（本机 node 计时，仅 compile 步）**：\`node scripts/porffor_compile.mjs\` 在 ~10 个模块时 <3s；全量 150 模块预估 <30s compile + 链接时间随模块线性增长，CI 可接受。

**合入门槛**：CI Porffor job 只跑 \`ready\` 集；\`pending\`/\`blocked\` 计入覆盖率摘要，不阻塞合入。

## 迁移分类（script 测试）

| 分类 | 数量 |
|------|------|
${Object.entries(buckets)
  .sort((a, b) => b[1] - a[1])
  .map(([k, v]) => `| ${k} | ${v} |`)
  .join('\n')}

## 全量清单

| 文件 | porffor 状态 | 显式标记 | 阻塞原因 | script | 模块 | 迁移分类 |
|------|-------------|---------|---------|--------|------|---------|
${rows.map((r) => `| ${r.file} | ${r.status} | ${r.explicit} | ${r.reason} | ${r.hasScript} | ${r.module} | ${r.migration} |`).join('\n')}

## dropped 候选（待评审）

| 文件 | 建议理由 |
|------|---------|
| complex.html | 综合演示页，非回归粒度 |
| skin_test.html | 皮肤/主题实验，非引擎契约 |

## T14 snippet 交叉引用

见 [T14-snippet-inventory.md](./T14-snippet-inventory.md)（\`porffor_snippet_inventory.mjs\` 生成）。
`;

fs.mkdirSync(path.dirname(outPath), { recursive: true });
fs.writeFileSync(outPath, md);
console.log(`[porffor_test_inventory] ${total} tests -> ${outPath}`);
console.log(`  ready=${counts.ready ?? 0} pending=${counts.pending ?? 0} blocked=${counts.blocked ?? 0}`);
