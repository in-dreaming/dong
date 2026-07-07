#!/usr/bin/env node
/**
 * Wave: blocked(T14) — CE/selection/IME 分批处理。
 *
 * - frame0-smoke: 布局/静态 DOM 不依赖脚本副作用 → porffor: ready
 * - exec-on-load: 解析期 execCommand 改 innerHTML → blocked(T20)（C++ 输入 / snippet Plan B）
 * - multiframe-snippet: 文档引用 eval snippet → 写 .mf.json 占位 + blocked(T20)
 *
 * Usage: node scripts/porffor_batch_migrate_t14.mjs [--dry-run]
 */
import fs from 'node:fs';
import path from 'node:path';
import { dongRoot } from './porffor_paths.mjs';
import { parsePorfforTagFromHtml, detectEvalSnippetRef } from './porffor_test_tags.mjs';

const testsDir = path.join(dongRoot, 'examples', 'data', 'tests');
const mfDir = path.join(dongRoot, 'examples', 'porffor', 'tests');
const dryRun = process.argv.includes('--dry-run');

/** @type {Record<string, { action: 'ready'|'blocked-t20'|'multiframe-stub', note?: string, snippet?: string }>} */
const T14_DISPOSITION = {
  // frame-0 布局冒烟（Porffor 忽略 inline script / 仅监听器）
  'test_selection_api.html': { action: 'ready', note: 'selection UI; interaction via click only' },
  'test_ime_composition.html': { action: 'ready', note: 'IME fields layout; events on user input' },
  'test_contenteditable_basic.html': { action: 'ready', note: 'CE toolbar + editor layout' },
  'test_contenteditable_prebolded.html': { action: 'ready', note: 'pre-bolded span in static HTML' },
  'test_contenteditable_typing.html': { action: 'ready', note: 'CE editor + instruction list' },
  'test_ce_debug.html': { action: 'ready', note: 'CE debug buttons layout' },
  'test_ce_enter_offset.html': { action: 'ready', note: 'static CE blocks + log placeholder' },
  'test_ce_mixed_multiline.html': { action: 'ready', note: 'mixed CE sections layout' },

  // 解析期 execCommand — 已迁移为 Porffor host import（T20）
  'test_contenteditable_bold_auto.html': {
    action: 'multiframe-stub',
    note: 'auto bold on parse; needs C++ drive or ce_bold snippet (Plan B)',
    snippet: 'snippets/ce_bold_after_frame0.js',
  },
  'test_contenteditable_auto.html': { action: 'blocked-t20', note: 'execCommand suite on parse' },
  'test_contenteditable_features.html': { action: 'blocked-t20', note: 'execCommand feature matrix on parse' },
  'test_ce_simulate.html': { action: 'blocked-t20', note: 'simulated execCommand flow on parse' },
  'test_ce_interactive_sim.html': { action: 'blocked-t20', note: 'simulated button+execCommand on parse' },
};

function replacePorfforTag(html, tag) {
  const stripped = html.replace(
    /<!--\s*porffor:\s*(?:ready|pending|blocked\([^)]+\)|dropped\([^)]+\))\s*-->\s*\n?/gi,
    '',
  );
  const line = `<!-- porffor: ${tag} -->`;
  if (stripped.startsWith('<!DOCTYPE')) {
    const next = stripped.replace('<!DOCTYPE html>', `<!DOCTYPE html>\n${line}`);
    if (next !== stripped) return next;
    return stripped.replace(/<!DOCTYPE html[^>]*>/i, (m) => `${m}\n${line}`);
  }
  return `${line}\n${stripped}`;
}

function snippetModuleName(snippetPath) {
  const base = path.basename(snippetPath, '.js');
  return `qj_${base}`;
}

let ready = 0;
let t20 = 0;
let mfStub = 0;
let skip = 0;

const report = [];

for (const file of fs.readdirSync(testsDir).filter((f) => f.endsWith('.html'))) {
  const filePath = path.join(testsDir, file);
  const html = fs.readFileSync(filePath, 'utf8');
  const tag = parsePorfforTagFromHtml(html);
  if (tag.status !== 'blocked' || tag.reason !== 'T14') {
    skip++;
    continue;
  }

  let disp = T14_DISPOSITION[file];
  if (!disp) {
    const snippet = detectEvalSnippetRef(html);
    if (snippet.usesEvalSnippet) {
      disp = {
        action: 'multiframe-stub',
        note: 'eval snippet ref',
        snippet: snippet.snippetPath ?? undefined,
      };
    } else if (/document\.execCommand\s*\(/.test(html)) {
      disp = { action: 'blocked-t20', note: 'execCommand (auto-detect)' };
    } else {
      disp = { action: 'ready', note: 'default frame-0 smoke' };
    }
  }

  if (dryRun) {
    console.log(`[dry-run] ${file} -> ${disp.action} (${disp.note ?? ''})`);
    report.push({ file, ...disp });
    continue;
  }

  if (disp.action === 'ready') {
    fs.writeFileSync(filePath, replacePorfforTag(html, 'ready'));
    console.log(`ready ${file}`);
    ready++;
    report.push({ file, result: 'ready', note: disp.note });
    continue;
  }

  if (disp.action === 'blocked-t20') {
    fs.writeFileSync(filePath, replacePorfforTag(html, 'blocked(T20)'));
    console.log(`blocked(T20) ${file}`);
    t20++;
    report.push({ file, result: 'blocked(T20)', note: disp.note });
    continue;
  }

  if (disp.action === 'multiframe-stub') {
    fs.writeFileSync(filePath, replacePorfforTag(html, 'blocked(T20)'));
    const mfName = `${path.basename(file, '.html')}.mf.json`;
    const mfPath = path.join(mfDir, mfName);
    const mod = disp.snippet ? snippetModuleName(disp.snippet) : null;
    const mf = {
      frames: 2,
      _t14Note: disp.note,
      _snippetSource: disp.snippet ?? null,
      ...(mod
        ? {
            callExportAfterFrame0: `${mod}::__snippet_${path.basename(disp.snippet, '.js').replace(/[^a-zA-Z0-9_]/g, '_')}`,
            _status: 'plan_b_pending',
          }
        : {}),
    };
    fs.mkdirSync(mfDir, { recursive: true });
    fs.writeFileSync(mfPath, `${JSON.stringify(mf, null, 2)}\n`);
    console.log(`blocked(T20) + mf stub ${file} -> ${mfName}`);
    mfStub++;
    report.push({ file, result: 'blocked(T20)+mf', note: disp.note, mf: mfName });
  }
}

const outPath = path.join(
  dongRoot,
  '..',
  'docs',
  'developer',
  'porffor',
  'tasks',
  'T14-batch-result.md',
);

const md = `# T14 波次处理结果

> \`node dong/scripts/porffor_batch_migrate_t14.mjs\` 生成

## 摘要

| 动作 | 数量 |
|------|------|
| ready（frame-0 冒烟） | ${ready} |
| blocked(T20)（execCommand 解析期） | ${t20} |
| blocked(T20) + mf.json 占位 | ${mfStub} |
| 跳过 | ${skip} |

## 明细

| 文件 | 结果 | 说明 |
|------|------|------|
${report.map((r) => `| ${r.file} | ${r.result ?? r.action} | ${r.note ?? ''}${r.mf ? ` (${r.mf})` : ''} |`).join('\n')}

## 后续

- **ready** 集：\`run-porffor-tests.mjs\` frame-0 布局回归
- **blocked(T20)**：待 C++ 输入注入或 \`ce_bold_*\` snippet Porffor 化（Plan B）
- \`test_contenteditable_bold_auto.mf.json\` 已预留 \`callExportAfterFrame0\`，待 \`porffor_snippet_compile\` 完成 prelude 改写后启用
`;

fs.mkdirSync(path.dirname(outPath), { recursive: true });
if (!dryRun) {
  fs.writeFileSync(outPath, md);
}

console.log(
  `[porffor_batch_migrate_t14] ready=${ready} t20=${t20} mfStub=${mfStub} skip=${skip}` +
    (dryRun ? ' (dry-run)' : ` -> ${outPath}`),
);
