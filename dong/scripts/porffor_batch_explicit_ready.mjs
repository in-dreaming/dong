#!/usr/bin/env node
/**
 * T13 Wave 5 — add explicit <!-- porffor: ready --> to implicit-ready static HTML tests.
 * Does not change status counts; makes inventory explicit column accurate.
 *
 * Usage: node scripts/porffor_batch_explicit_ready.mjs [--dry-run]
 */
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { dongRoot } from './porffor_paths.mjs';
import { parsePorfforTagFromHtml } from './porffor_test_tags.mjs';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const testsDir = path.join(dongRoot, 'examples', 'data', 'tests');
const dryRun = process.argv.includes('--dry-run');

const TAG_LINE = '<!-- porffor: ready -->';

let tagged = 0;
for (const file of fs.readdirSync(testsDir).filter((f) => f.endsWith('.html'))) {
  const filePath = path.join(testsDir, file);
  const html = fs.readFileSync(filePath, 'utf8');
  const tag = parsePorfforTagFromHtml(html);
  if (tag.explicit || tag.status !== 'ready') continue;

  let next;
  if (html.startsWith('<!DOCTYPE')) {
    next = html.replace('<!DOCTYPE html>', `<!DOCTYPE html>\n${TAG_LINE}`);
    if (next === html) {
      next = html.replace(/<!DOCTYPE html[^>]*>/i, (m) => `${m}\n${TAG_LINE}`);
    }
  } else {
    next = `${TAG_LINE}\n${html}`;
  }

  if (next === html) continue;
  tagged++;
  if (dryRun) {
    console.log(`[dry-run] would tag ${file}`);
  } else {
    fs.writeFileSync(filePath, next);
    console.log(`tagged ${file}`);
  }
}

console.log(
  `[porffor_batch_explicit_ready] ${dryRun ? 'would tag' : 'tagged'} ${tagged} static-ready tests`,
);
