#!/usr/bin/env node
/**
 * CI-ready Porffor test runner: runs all <!-- porffor: ready --> HTML tests via html_render_test.
 *
 * Usage (from dong/):
 *   node scripts/run-porffor-tests.mjs [--html-render-test PATH] [--verbose]
 */
import fs from 'node:fs';
import path from 'node:path';
import { spawnSync } from 'node:child_process';
import { fileURLToPath } from 'node:url';
import { dongRoot } from './porffor_paths.mjs';
import { parsePorfforTagFromHtml, extractPorfforModule } from './porffor_test_tags.mjs';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const testsDir = path.join(dongRoot, 'examples', 'data', 'tests');
const dataDir = path.join(dongRoot, 'examples', 'data');

function discoverReadyHtml() {
  const entries = [];

  const scanFile = (htmlPath, id) => {
    const html = fs.readFileSync(htmlPath, 'utf8');
    const tag = parsePorfforTagFromHtml(html);
    entries.push({ id, htmlPath, tag, module: extractPorfforModule(html) });
  };

  for (const file of fs.readdirSync(testsDir).filter((f) => f.endsWith('.html'))) {
    scanFile(path.join(testsDir, file), file);
  }

  if (fs.existsSync(dataDir)) {
    for (const name of fs.readdirSync(dataDir)) {
      if (!name.startsWith('porf-')) continue;
      const indexHtml = path.join(dataDir, name, 'index.html');
      if (fs.existsSync(indexHtml)) {
        scanFile(indexHtml, path.join(name, 'index.html').replace(/\\/g, '/'));
      }
    }
  }

  return entries;
}
const defaultExe = process.platform === 'win32'
  ? path.join(dongRoot, 'zig-out', 'bin', 'html_render_test.exe')
  : path.join(dongRoot, 'zig-out', 'bin', 'html_render_test');

const args = process.argv.slice(2);
let exe = defaultExe;
let verbose = false;
for (let i = 0; i < args.length; i++) {
  if (args[i] === '--html-render-test' && args[i + 1]) {
    exe = args[++i];
  } else if (args[i] === '--verbose') {
    verbose = true;
  }
}

if (!fs.existsSync(exe)) {
  console.error(`[run-porffor-tests] html_render_test not found: ${exe}`);
  console.error('  Build first: cd dong && zig build');
  process.exit(2);
}

const outDir = path.join(dongRoot, 'zig-out', 'tmp', 'porffor-tests');
fs.mkdirSync(outDir, { recursive: true });

const discovered = discoverReadyHtml();
const inventory = { total: discovered.length, ready: 0, pending: 0, blocked: 0, dropped: 0 };
const readyTests = [];

for (const item of discovered) {
  const { tag } = item;
  inventory[tag.status] = (inventory[tag.status] ?? 0) + 1;
  if (tag.status === 'ready') {
    readyTests.push(item);
  }
}

console.log(
  `[run-porffor-tests] coverage: ready=${inventory.ready}/${inventory.total} ` +
    `(${(inventory.ready / inventory.total * 100).toFixed(1)}%)`,
);

let failed = 0;
let passed = 0;

for (const { id, htmlPath, module } of readyTests) {
  const safeName = id.replace(/[\\/]/g, '_').replace(/\.html$/, '');
  const outBmp = path.join(outDir, `${safeName}.bmp`);
  const cmd = [exe, htmlPath, outBmp, '800', '600', '1'];
  const env = { ...process.env };
  if (module) {
    env.DONG_PORFFOR_MODULE = module;
  } else {
    delete env.DONG_PORFFOR_MODULE;
  }

  const baseName = path.basename(htmlPath, '.html');
  const parentDir = path.basename(path.dirname(htmlPath));
  const mfCandidates = [
    path.join(path.dirname(htmlPath), `${baseName}.mf.json`),
    path.join(path.dirname(htmlPath), 'mf.json'),
    path.join(dongRoot, 'examples', 'porffor', 'tests', `${parentDir}.mf.json`),
    path.join(dongRoot, 'examples', 'porffor', 'tests', `${path.basename(id, '.html')}.mf.json`),
  ];
  for (const mfMetaPath of mfCandidates) {
    if (!fs.existsSync(mfMetaPath)) continue;
    const meta = JSON.parse(fs.readFileSync(mfMetaPath, 'utf8'));
    if (meta.frames) cmd[5] = String(meta.frames);
    if (meta.callExportAfterFrame0) {
      cmd.push('--call-export-after-frame0', meta.callExportAfterFrame0);
    }
    break;
  }

  if (verbose) console.log('+', cmd.join(' '));
  const r = spawnSync(cmd[0], cmd.slice(1), { env, stdio: verbose ? 'inherit' : 'pipe', cwd: dongRoot });
  if (r.status !== 0) {
    failed++;
    console.error(`FAIL ${id}`);
    if (!verbose && r.stderr) process.stderr.write(r.stderr);
  } else {
    passed++;
    if (verbose) console.log(`ok ${id}`);
  }
}

console.log(`[run-porffor-tests] passed=${passed} failed=${failed} skipped=${inventory.total - inventory.ready}`);
if (failed > 0) process.exit(1);
