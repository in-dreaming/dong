#!/usr/bin/env node
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { spawnSync } from 'node:child_process';
import { extractInlineEntry } from '../../../../../../dong/scripts/porffor_inline_handlers.mjs';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const reproDir = __dirname;
const dongRoot = path.resolve(reproDir, '../../../../../../dong');
const nodeExe = process.env.PORFFOR_NODE ?? 'E:\\ws\\infra\\dong\\.tools\\node-v22.16.0-win-x64\\node.exe';
const node = fs.existsSync(nodeExe) ? nodeExe : process.execPath;

function fail(msg) {
  console.error('FAIL:', msg);
  process.exit(1);
}

function ok(msg) {
  console.log('OK:', msg);
}

function runNode(args, opts = {}) {
  return spawnSync(node, args, { encoding: 'utf8', ...opts });
}

async function assertExtraction() {
  const htmlPath = path.join(reproDir, 't12_inline.html');
  const result = await extractInlineEntry({
    name: 't12_inline',
    html: path.relative(dongRoot, htmlPath).replace(/\\/g, '/'),
    out_js: 'generated/porffor/t12_inline.js',
    out_html: 'generated/porffor/t12_inline.html',
  });
  if (!result.entry) fail('extraction produced no entry');
  if (result.entry.exports.length !== 2) {
    fail(`expected 2 exports, got ${result.entry.exports.length}`);
  }
  if (!result.entry.exports.includes('inc__onclick')) {
    fail('missing inc__onclick export');
  }
  if (!result.entry.exports.includes('__porf_auto_0__onclick')) {
    fail('missing auto-id export __porf_auto_0__onclick');
  }

  const js = fs.readFileSync(path.join(dongRoot, result.entry.path), 'utf8');
  if (!/var count = 0/.test(js)) fail('shared count global missing');
  if (!/export function inc__onclick/.test(js)) fail('inc__onclick export missing');
  if (!/addEventListener\(incId, 'click', 'inc__onclick'\)/.test(js)) {
    fail('inc listener registration missing');
  }

  const cleaned = fs.readFileSync(path.join(dongRoot, result.entry.html), 'utf8');
  if (/onclick=/i.test(cleaned)) fail('cleaned HTML still has onclick');
  if (!/id="__porf_auto_0"/.test(cleaned)) fail('auto id not injected into cleaned HTML');
  ok('inline handler extraction + HTML cleanup');
}

function assertDroppedSkip() {
  const droppedHtml = path.join(reproDir, 't12_dropped.html');
  fs.writeFileSync(
    droppedHtml,
    '<!-- porffor: dropped(test) --><button onclick="count++">x</button>',
  );
  return extractInlineEntry({
    name: 't12_dropped',
    html: path.relative(dongRoot, droppedHtml).replace(/\\/g, '/'),
  }).then((result) => {
    if (!result.skipped) fail('dropped HTML should be skipped');
    ok('dropped HTML skipped');
    fs.unlinkSync(droppedHtml);
  });
}

function assertCompileError() {
  const badHtml = path.join(reproDir, 't12_bad.html');
  fs.writeFileSync(
    badHtml,
    '<html data-porffor-module="t12_bad"><button id="b" onclick="let x = () => x;">bad</button></html>',
  );
  return extractInlineEntry({
    name: 't12_bad',
    html: path.relative(dongRoot, badHtml).replace(/\\/g, '/'),
  })
    .then(() => fail('closure handler should fail compile'))
    .catch((e) => {
      if (!/failed Porffor compile/.test(String(e.message))) {
        fail(`unexpected error: ${e.message}`);
      }
      ok('compile failure reports HTML location');
      fs.unlinkSync(badHtml);
    });
}

function assertPorfforCompile() {
  const r = runNode([path.join(dongRoot, 'scripts/porffor_compile.mjs')], {
    cwd: dongRoot,
    maxBuffer: 10 * 1024 * 1024,
  });
  if (r.status !== 0) {
    const tail = (r.stderr || r.stdout || '').slice(-2000);
    fail(`porffor_compile.mjs failed: ${tail}`);
  }
  const registry = fs.readFileSync(path.join(dongRoot, 'generated/porffor/registry.c'), 'utf8');
  if (!/dong_porf_t12_inline_export_inc__onclick/.test(registry)) {
    fail('registry missing t12_inline inc__onclick export');
  }
  if (!/dong_porf_t12_inline_export___porf_auto_0__onclick/.test(registry)) {
    fail('registry missing auto-id handler export');
  }
  ok('porffor_compile registry includes t12_inline exports');
}

function assertRuntimeStubRemoved() {
  const bindings = fs.readFileSync(
    path.join(dongRoot, 'src/script/porffor/js_bindings_porffor.cpp'),
    'utf8',
  );
  if (/inline event handlers not supported/.test(bindings)) {
    fail('Porffor inline handler warning stub still present');
  }
  const engineView = fs.readFileSync(path.join(dongRoot, 'src/core/engine_view.cpp'), 'utf8');
  const porfforBlock = engineView.match(
    /#ifdef DONG_SCRIPT_ENGINE_PORFFOR([\s\S]*?)#else/,
  )?.[1];
  if (porfforBlock && /scanAndRegisterInlineEventHandlers/.test(porfforBlock)) {
    fail('engine_view Porffor path still calls scanAndRegisterInlineEventHandlers');
  }
  ok('runtime inline scan stub removed (F12)');
}

async function main() {
  await assertExtraction();
  await assertDroppedSkip();
  await assertCompileError();
  assertPorfforCompile();
  assertRuntimeStubRemoved();
  console.log('\nAll T12 verification checks passed.');
}

main().catch((e) => {
  console.error(e);
  process.exit(1);
});
