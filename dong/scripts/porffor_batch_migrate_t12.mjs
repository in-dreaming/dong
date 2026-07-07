#!/usr/bin/env node
/**
 * Wave: blocked(T12) — extract inline on* handlers via porffor_inline_handlers.
 *
 * On success: writes generated JS/HTML, tags test ready, patches manifest scripts[].
 *
 * Usage:
 *   node scripts/porffor_batch_migrate_t12.mjs [--dry-run] [--file test_foo.html]
 */
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { dongRoot } from './porffor_paths.mjs';
import { parsePorfforTagFromHtml } from './porffor_test_tags.mjs';
import { extractInlineEntry } from './porffor_inline_handlers.mjs';

const testsDir = path.join(dongRoot, 'examples', 'data', 'tests');
const manifestPath = path.join(dongRoot, 'scripts', 'porffor_manifest.json');
const dryRun = process.argv.includes('--dry-run');
const onlyFile = (() => {
  const i = process.argv.indexOf('--file');
  return i >= 0 ? process.argv[i + 1] : null;
})();

function moduleNameFromFile(file) {
  return path.basename(file, '.html').replace(/[^a-zA-Z0-9_]/g, '_');
}

function tagReady(html) {
  const stripped = html.replace(
    /<!--\s*porffor:\s*(?:ready|pending|blocked\([^)]+\)|dropped\([^)]+\))\s*-->\s*\n?/gi,
    '',
  );
  if (stripped.startsWith('<!DOCTYPE')) {
    const next = stripped.replace('<!DOCTYPE html>', '<!DOCTYPE html>\n<!-- porffor: ready -->');
    if (next !== stripped) return next;
    return stripped.replace(
      /<!DOCTYPE html[^>]*>/i,
      (m) => `${m}\n<!-- porffor: ready -->`,
    );
  }
  return `<!-- porffor: ready -->\n${stripped}`;
}

function addModuleAttr(html, moduleName) {
  if (/data-porffor-module=/i.test(html)) return html;
  if (/<html\b/i.test(html)) {
    return html.replace(/<html\b/i, `<html data-porffor-module="${moduleName}"`);
  }
  return html.replace(/<body\b/i, `<body data-porffor-module="${moduleName}"`);
}

let ok = 0;
let fail = 0;
let skip = 0;

async function main() {
const files = fs
  .readdirSync(testsDir)
  .filter((f) => f.endsWith('.html'))
  .filter((f) => !onlyFile || f === onlyFile);

for (const file of files) {
  const filePath = path.join(testsDir, file);
  const html = fs.readFileSync(filePath, 'utf8');
  const tag = parsePorfforTagFromHtml(html);
  if (tag.status !== 'blocked' || tag.reason !== 'T12') {
    skip++;
    continue;
  }

  const name = moduleNameFromFile(file);
  const relHtml = path.posix.join('examples/data/tests', file);
  const outJs = `generated/porffor/t12_${name}.js`;

  try {
    const result = await extractInlineEntry({
      name: `t12_${name}`,
      html: relHtml,
      out_js: outJs,
    });

    if (result.skipped || !result.entry || !result.cleanedHtml) {
      console.log(`skip ${file}: no extractable inline handlers`);
      skip++;
      continue;
    }

    let finalHtml = tagReady(addModuleAttr(result.cleanedHtml, result.entry.name));

    if (dryRun) {
      console.log(
        `[dry-run] ok ${file} -> ${result.entry.name} (${result.entry.exports.length} exports)`,
      );
      ok++;
      continue;
    }

    fs.writeFileSync(filePath, finalHtml);

    const manifest = JSON.parse(fs.readFileSync(manifestPath, 'utf8'));
    const scripts = manifest.scripts ?? [];
    const existing = scripts.findIndex((s) => s.name === result.entry.name);
    const entry = {
      name: result.entry.name,
      path: result.entry.path,
      exports: result.entry.exports,
    };
    if (existing >= 0) {
      scripts[existing] = entry;
    } else {
      scripts.push(entry);
    }
    manifest.scripts = scripts;
    fs.writeFileSync(manifestPath, `${JSON.stringify(manifest, null, 2)}\n`);

    console.log(`ok ${file} -> ${result.entry.name}`);
    ok++;
  } catch (e) {
    console.error(`FAIL ${file}: ${e.message}`);
    fail++;
  }
}

console.log(`[porffor_batch_migrate_t12] ok=${ok} fail=${fail} skip=${skip}`);
}

main().catch((e) => {
  console.error(e);
  process.exit(1);
});
