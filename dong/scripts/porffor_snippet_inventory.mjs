#!/usr/bin/env node
/**
 * Scan --eval-after-frame0-file usages and snippet files; emit T14-snippet-inventory.md
 */
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { dongRoot } from './porffor_paths.mjs';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const repoRoot = path.resolve(dongRoot, '..');
const snippetsDir = path.join(dongRoot, 'examples', 'data', 'tests', 'snippets');
const toolsDir = path.join(dongRoot, 'scripts', 'tools');
const outPath = path.join(repoRoot, 'docs', 'developer', 'porffor', 'tasks', 'T14-snippet-inventory.md');

function classifySnippet(source, name) {
  const lower = source.toLowerCase();
  if (/execcommand|getselection|createrange|contenteditable/.test(lower)) {
    return { plan: 'B', note: 'CE/selection — C++ input or T20' };
  }
  if (/fetch\s*\(|async|await|promise/.test(lower)) {
    return { plan: 'B', note: 'async/platform API' };
  }
  if (/getelementbyid|setattribute|classlist|textcontent|focus\s*\(/.test(lower)) {
    return { plan: 'A', note: 'precompile to export (prelude rewrite)' };
  }
  return { plan: 'review', note: 'manual triage' };
}

const snippetFiles = fs.existsSync(snippetsDir)
  ? fs.readdirSync(snippetsDir).filter((f) => f.endsWith('.js'))
  : [];

const rows = [];
for (const file of snippetFiles) {
  const src = fs.readFileSync(path.join(snippetsDir, file), 'utf8');
  const { plan, note } = classifySnippet(src, file);
  rows.push({ file: `snippets/${file}`, plan, note, bytes: src.length });
}

const refs = [];
function scanDir(dir, rel = '') {
  if (!fs.existsSync(dir)) return;
  for (const ent of fs.readdirSync(dir, { withFileTypes: true })) {
    const p = path.join(dir, ent.name);
    const r = rel ? `${rel}/${ent.name}` : ent.name;
    if (ent.isDirectory()) scanDir(p, r);
    else if (ent.name.endsWith('.py') || ent.name.endsWith('.html') || ent.name.endsWith('.md')) {
      const text = fs.readFileSync(p, 'utf8');
      const re = /--eval-after-frame0-file\s+(\S+)/g;
      let m;
      while ((m = re.exec(text)) !== null) {
        refs.push({ from: r, snippet: m[1] });
      }
    }
  }
}
scanDir(toolsDir, 'scripts/tools');
scanDir(path.join(dongRoot, 'examples', 'data', 'tests'), 'examples/data/tests');

let md = `# T14 — eval snippet 盘点

> 自动生成：\`node dong/scripts/porffor_snippet_inventory.mjs\`

## 摘要

| 指标 | 值 |
|------|-----|
| snippet 文件数 | ${rows.length} |
| 方案 A（可预编译 export） | ${rows.filter((r) => r.plan === 'A').length} |
| 方案 B（C++ 驱动 / T20） | ${rows.filter((r) => r.plan === 'B').length} |
| 待评审 | ${rows.filter((r) => r.plan === 'review').length} |

## snippet 文件分类

| 文件 | 方案 | 说明 | 大小 |
|------|------|------|------|
${rows.map((r) => `| ${r.file} | ${r.plan} | ${r.note} | ${r.bytes}B |`).join('\n')}

## 引用位置（--eval-after-frame0-file）

| 来源 | snippet 路径 |
|------|-------------|
${refs.map((r) => `| ${r.from} | ${r.snippet} |`).join('\n') || '| (none) | |'}

## 试点（Porffor ready）

| 测试 HTML | 模块 | export | 说明 |
|-----------|------|--------|------|
| test_porffor_mf_text.html | test_mf_text | afterFrame0 | 帧后改 textContent |
| test_porffor_mf_class.html | test_mf_class | afterFrame0 | 帧后 classAdd |
| test_porffor_mf_style.html | test_mf_style | afterFrame0 | 帧后 setStyle |

CE 类 snippet（\`ce_bold_*\`）归方案 B，待 T20；不阻塞 T14 runner 基建验收。
`;

fs.mkdirSync(path.dirname(outPath), { recursive: true });
fs.writeFileSync(outPath, md);
console.log(`[porffor_snippet_inventory] ${rows.length} snippets, ${refs.length} refs -> ${outPath}`);
