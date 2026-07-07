#!/usr/bin/env node
/**
 * T13 Wave 5 — mark pending `direct` script tests as porffor-ready (render smoke).
 *
 * Strategy: Porffor runs `data-porffor-module` only; inline QuickJS `<script>` is ignored.
 * Most `direct` tests are CSS/DOM layout smokes where frame-0 appearance does not depend
 * on inline script side effects. Interaction-only scripts (listeners) are safe to ignore
 * for html_render_test frame 0.
 *
 * Skips: already ready, already has module, async/Promise/eval/inline-on*.
 *
 * Usage: node scripts/porffor_batch_migrate_direct.mjs [--dry-run]
 */
import fs from 'node:fs';
import path from 'node:path';
import { dongRoot } from './porffor_paths.mjs';
import {
  parsePorfforTagFromHtml,
  classifyScriptTest,
  extractPorfforModule,
} from './porffor_test_tags.mjs';

const testsDir = path.join(dongRoot, 'examples', 'data', 'tests');
const dryRun = process.argv.includes('--dry-run');
const TAG = '<!-- porffor: ready -->';

const SKIP_SCRIPT_RE =
  /\basync\b|\bawait\b|\bpromise\b|\beval\s*\(|\bon\w+\s*=|import\s*\(/i;

function ensureReadyTag(html) {
  if (html.includes('porffor: ready')) return html;
  if (html.startsWith('<!DOCTYPE')) {
    const next = html.replace('<!DOCTYPE html>', `<!DOCTYPE html>\n${TAG}`);
    if (next !== html) return next;
    return html.replace(/<!DOCTYPE html[^>]*>/i, (m) => `${m}\n${TAG}`);
  }
  return `${TAG}\n${html}`;
}

let tagged = 0;
let skipped = 0;

for (const file of fs.readdirSync(testsDir).filter((f) => f.endsWith('.html'))) {
  const filePath = path.join(testsDir, file);
  const html = fs.readFileSync(filePath, 'utf8');
  const tag = parsePorfforTagFromHtml(html);
  if (tag.status === 'ready' || tag.status === 'dropped' || tag.status === 'blocked') {
    skipped++;
    continue;
  }
  if (!/<script[\s>]/i.test(html)) {
    skipped++;
    continue;
  }
  const bucket = classifyScriptTest(html, file).bucket;
  if (bucket !== 'direct') {
    skipped++;
    continue;
  }
  if (extractPorfforModule(html)) {
    skipped++;
    continue;
  }
  const scriptM = html.match(/<script[^>]*>([\s\S]*?)<\/script>/i);
  const scriptBody = scriptM?.[1] ?? '';
  if (SKIP_SCRIPT_RE.test(scriptBody)) {
    skipped++;
    continue;
  }

  const next = ensureReadyTag(html);
  if (next === html) {
    skipped++;
    continue;
  }
  tagged++;
  if (dryRun) {
    console.log(`[dry-run] ready ${file}`);
  } else {
    fs.writeFileSync(filePath, next);
    console.log(`ready ${file}`);
  }
}

console.log(
  `[porffor_batch_migrate_direct] ${dryRun ? 'would tag' : 'tagged'} ${tagged}, skipped ${skipped}`,
);
