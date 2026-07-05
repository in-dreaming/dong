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

const allHtml = fs.readdirSync(testsDir).filter((f) => f.endsWith('.html'));
const inventory = { total: allHtml.length, ready: 0, pending: 0, blocked: 0, dropped: 0 };
const readyTests = [];

for (const file of allHtml) {
  const html = fs.readFileSync(path.join(testsDir, file), 'utf8');
  const tag = parsePorfforTagFromHtml(html);
  inventory[tag.status] = (inventory[tag.status] ?? 0) + 1;
  if (tag.status === 'ready') {
    readyTests.push({ file, module: extractPorfforModule(html) });
  }
}

console.log(
  `[run-porffor-tests] coverage: ready=${inventory.ready}/${inventory.total} ` +
    `(${(inventory.ready / inventory.total * 100).toFixed(1)}%)`,
);

let failed = 0;
let passed = 0;

for (const { file, module } of readyTests) {
  const htmlPath = path.join(testsDir, file);
  const outBmp = path.join(outDir, `${path.basename(file, '.html')}.bmp`);
  const cmd = [exe, htmlPath, outBmp, '800', '600', '1'];
  const env = { ...process.env };
  if (module) env.DONG_PORFFOR_MODULE = module;

  const mfMetaPath = path.join(dongRoot, 'examples', 'porffor', 'tests', `${path.basename(file, '.html')}.mf.json`);
  if (fs.existsSync(mfMetaPath)) {
    const meta = JSON.parse(fs.readFileSync(mfMetaPath, 'utf8'));
    if (meta.frames) cmd[5] = String(meta.frames);
    if (meta.callExportAfterFrame0) {
      cmd.push('--call-export-after-frame0', meta.callExportAfterFrame0);
    }
  }

  if (verbose) console.log('+', cmd.join(' '));
  const r = spawnSync(cmd[0], cmd.slice(1), { env, stdio: verbose ? 'inherit' : 'pipe', cwd: dongRoot });
  if (r.status !== 0) {
    failed++;
    console.error(`FAIL ${file}`);
    if (!verbose && r.stderr) process.stderr.write(r.stderr);
  } else {
    passed++;
    if (verbose) console.log(`ok ${file}`);
  }
}

console.log(`[run-porffor-tests] passed=${passed} failed=${failed} skipped=${inventory.total - inventory.ready}`);
if (failed > 0) process.exit(1);
