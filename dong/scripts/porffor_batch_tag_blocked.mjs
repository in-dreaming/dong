#!/usr/bin/env node
/**
 * Tag all implicit `pending` script tests with blocked(Txx) / dropped / ready.
 *
 * Usage:
 *   node scripts/porffor_batch_tag_blocked.mjs [--dry-run]
 */
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { dongRoot } from './porffor_paths.mjs';
import { parsePorfforTagFromHtml } from './porffor_test_tags.mjs';
import {
  resolvePendingDisposition,
  formatPorfforTag,
  summarizeBatches,
} from './porffor_blocked_reasons.mjs';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const repoRoot = path.resolve(dongRoot, '..');
const testsDir = path.join(dongRoot, 'examples', 'data', 'tests');
const outBatches = path.join(
  repoRoot,
  'docs',
  'developer',
  'porffor',
  'tasks',
  'T13-pending-batches.md',
);
const dryRun = process.argv.includes('--dry-run');

const TAG_RE = /<!--\s*porffor:\s*(?:ready|pending|blocked\([^)]+\)|dropped\([^)]+\))\s*-->\s*\n?/gi;

function stripExistingTag(html) {
  return html.replace(TAG_RE, '');
}

function insertTag(html, tag) {
  const body = stripExistingTag(html);
  if (body.startsWith('<!DOCTYPE')) {
    const next = body.replace('<!DOCTYPE html>', `<!DOCTYPE html>\n${tag}`);
    if (next !== body) return next;
    return body.replace(/<!DOCTYPE html[^>]*>/i, (m) => `${m}\n${tag}`);
  }
  return `${tag}\n${body}`;
}

/** @type {Array<{ file: string, disposition: import('./porffor_blocked_reasons.mjs').resolvePendingDisposition extends (...args: any) => infer R ? R : never }>} */
const processed = [];
let updated = 0;
let skipped = 0;

for (const file of fs.readdirSync(testsDir).filter((f) => f.endsWith('.html')).sort()) {
  const filePath = path.join(testsDir, file);
  const html = fs.readFileSync(filePath, 'utf8');
  const tag = parsePorfforTagFromHtml(html);
  if (tag.status !== 'pending') {
    skipped++;
    continue;
  }
  if (!/<script[\s>]/i.test(html)) {
    skipped++;
    continue;
  }

  const disposition = resolvePendingDisposition(html, file);
  const newTag = formatPorfforTag(disposition);
  const next = insertTag(html, newTag);
  processed.push({ file, disposition });

  if (next === html && tag.explicit) {
    skipped++;
    continue;
  }

  updated++;
  const label = `${disposition.status}${disposition.reason ? `(${disposition.reason})` : ''}`;
  if (dryRun) {
    console.log(`[dry-run] ${file} -> ${label}`);
  } else {
    fs.writeFileSync(filePath, next);
    console.log(`${file} -> ${label}`);
  }
}

const batches = summarizeBatches(processed);
let md = `# T13 — Pending 分批处理（自动生成）

> \`node dong/scripts/porffor_batch_tag_blocked.mjs\` 后生成；与 \`porffor_blocked_reasons.mjs\` 同步。

## 摘要

| 指标 | 值 |
|------|-----|
| 本次处理 | ${processed.length} |
| 写入标记 | ${updated} |
| 跳过 | ${skipped} |

## 波次（按阻塞任务）

处理顺序建议：**T12 内联提取** → **T14 多帧/snippet** → **T06 字符串/DOM 解析** → **T10 fetch 模块** → **T11 平台 API** → **T19 Promise** → **T20 长尾** → **dropped**

`;

for (const [batch, files] of batches) {
  md += `### ${batch}（${files.length}）\n\n`;
  for (const f of files.sort()) {
    md += `- ${f}\n`;
  }
  md += '\n';
}

md += `## 相关脚本

| 脚本 | 用途 |
|------|------|
| \`porffor_batch_tag_blocked.mjs\` | 本表：pending → blocked/dropped/ready |
| \`porffor_batch_migrate_t12.mjs\` | blocked(T12) → T12 内联 handler 提取 |
| \`porffor_batch_migrate_direct.mjs\` | direct 类 → ready（已完成） |
| \`porffor_test_inventory.mjs\` | 全量盘点 |
`;

if (!dryRun) {
  fs.mkdirSync(path.dirname(outBatches), { recursive: true });
  fs.writeFileSync(outBatches, md);
}

console.log(
  `[porffor_batch_tag_blocked] ${dryRun ? 'would update' : 'updated'} ${updated}, skipped ${skipped}`,
);
if (!dryRun) {
  console.log(`[porffor_batch_tag_blocked] batches -> ${outBatches}`);
}
