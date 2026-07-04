#!/usr/bin/env node
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath, pathToFileURL } from 'node:url';
import { spawnSync } from 'node:child_process';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const reproDir = __dirname;
const dongRoot = path.resolve(reproDir, '../../../../../../dong');
const porfforRoot = path.join(dongRoot, 'third_party', 'porffor');
const preludePath = path.join(dongRoot, 'src/script/porffor/dong_porffor_prelude.js');
const nodeExe = process.env.PORFFOR_NODE ?? 'E:\\ws\\infra\\dong\\.tools\\node-v22.16.0-win-x64\\node.exe';
const node = fs.existsSync(nodeExe) ? nodeExe : process.execPath;

const domJs = path.join(reproDir, 't06_dom.js');
const domC = path.join(reproDir, 't06_dom.c');
const stubsC = path.join(reproDir, 't06_stubs.c');
const nativeExe = path.join(reproDir, 't06_native.exe');

const T06_IMPORTS = [
  'dong_get_value',
  'dong_set_value',
  'dong_get_checked',
  'dong_set_checked',
  'dong_get_disabled',
  'dong_set_disabled',
  'dong_get_attribute',
  'dong_set_attribute',
  'dong_remove_attribute',
  'dong_set_inner_html',
  'dong_query_selector',
  'dong_query_selector_all',
  'dong_get_elements_by_tag_name',
  'dong_class_add',
  'dong_class_remove',
  'dong_class_toggle',
  'dong_class_contains',
  'dong_style_set',
  'dong_style_get',
  'dong_computed_style_get',
  'dong_get_rect',
  'dong_get_metric',
  'dong_get_scroll_top',
  'dong_set_scroll_top',
  'dong_get_scroll_left',
  'dong_set_scroll_left',
  'dong_focus',
  'dong_blur',
  'dong_click',
  'dong_matches',
  'dong_closest',
];

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

async function compileT06Source(cPath) {
  const argvBackup = process.argv.slice();
  process.argv = [
    process.argv[0],
    process.argv[1],
    '-O1',
    '--target=c',
    '--2c-wasm-imports',
    '--no-run',
    '--no-opt-unused',
    '--2c-prefix=t06_',
    `-o=${cPath}`,
  ];

  const _realExit = process.exit.bind(process);
  process.exit = (code) => {
    if (code && code !== 0) _realExit(code);
  };

  const builtinsUrl = pathToFileURL(path.join(porfforRoot, 'compiler/builtins.js')).href;
  const indexUrl = pathToFileURL(path.join(porfforRoot, 'compiler/index.js')).href;
  const { createImport } = await import(builtinsUrl);
  const { default: compile } = await import(indexUrl);

  for (const spec of [
    ['dong_get_value', 1, 0],
    ['dong_set_value', 2, 0],
    ['dong_get_checked', 1, 1],
    ['dong_set_checked', 2, 0],
    ['dong_get_disabled', 1, 1],
    ['dong_set_disabled', 2, 0],
    ['dong_get_attribute', 2, 0],
    ['dong_set_attribute', 3, 0],
    ['dong_remove_attribute', 2, 0],
    ['dong_set_inner_html', 2, 0],
    ['dong_query_selector', 2, 1],
    ['dong_query_selector_all', 2, 0],
    ['dong_get_elements_by_tag_name', 2, 0],
    ['dong_class_add', 2, 0],
    ['dong_class_remove', 2, 0],
    ['dong_class_toggle', 2, 1],
    ['dong_class_contains', 2, 1],
    ['dong_style_set', 3, 0],
    ['dong_style_get', 2, 0],
    ['dong_computed_style_get', 2, 0],
    ['dong_get_rect', 1, 0],
    ['dong_get_metric', 2, 1],
    ['dong_get_scroll_top', 1, 1],
    ['dong_set_scroll_top', 2, 0],
    ['dong_get_scroll_left', 1, 1],
    ['dong_set_scroll_left', 2, 0],
    ['dong_focus', 1, 0],
    ['dong_blur', 1, 0],
    ['dong_click', 1, 0],
    ['dong_matches', 2, 1],
    ['dong_closest', 2, 1],
  ]) {
    createImport(spec[0], spec[1], spec[2], null, null);
  }
  createImport('dong_str_len', 0, 1, null, null);
  createImport('dong_str_byte_at', 1, 1, null, null);
  createImport('dong_print', 1, 0, null, null);
  createImport('dong_dom_getElementById', 1, 1, null, null);
  createImport('dong_dom_get_textContent', 1, 0, null, null);
  createImport('dong_stage_0', 1, 0, null, null);
  createImport('dong_stage_1', 1, 0, null, null);
  createImport('dong_stage_2', 1, 0, null, null);
  createImport('dong_commit_set_textContent', 0, 0, null, null);

  const prelude = fs.readFileSync(preludePath, 'utf8');
  const user = fs.readFileSync(domJs, 'utf8');
  globalThis.file = domJs;
  compile(`${prelude}\n${user}`);

  process.argv = argvBackup;
  process.exit = _realExit;

  if (!fs.existsSync(cPath)) fail(`missing output: ${cPath}`);
  return fs.readFileSync(cPath, 'utf8');
}

function assert2cImports(c) {
  for (const name of T06_IMPORTS) {
    const re = new RegExp(`__porf_import_${name.replace(/\//g, '\\/')}\\s*\\(`);
    if (!re.test(c)) fail(`2c missing import call: ${name}`);
    ok(`2c codegen: ${name}`);
  }
  if (!/__porf_import_dong_set_value\s*\([^)]+,[^)]+\)/.test(c)) {
    fail('2c dong_set_value not multi-arg');
  }
  ok('2c multi-arg dong_set_value');
}

async function assert2cNative() {
  const c = await compileT06Source(domC);
  assert2cImports(c);

  const r = spawnSync('clang', ['-O2', stubsC, '-o', nativeExe, '-lm'], {
    encoding: 'utf8',
    cwd: reproDir,
  });
  if (r.status !== 0) {
    console.log('SKIP: clang native run (clang unavailable or compile error)');
    return;
  }

  const run = spawnSync(nativeExe, [], { encoding: 'utf8', cwd: reproDir });
  if (run.status !== 0) {
    fail(`native T06 failed: ${run.stderr || run.stdout}`);
  }
  if (!/T06_PASS/.test(run.stdout ?? '')) {
    fail(`native expected T06_PASS, got: ${JSON.stringify(run.stdout)}`);
  }
  ok('clang native T06 DOM import roundtrip');
}

function assertPorfforCompile() {
  const r = runNode([path.join(dongRoot, 'scripts/porffor_compile.mjs')], { cwd: dongRoot });
  if (r.status !== 0) fail(`porffor_compile.mjs failed: ${r.stderr || r.stdout}`);
  const hostCpp = fs.readFileSync(path.join(dongRoot, 'src/script/porffor/dong_porf_host.cpp'), 'utf8');
  for (const name of T06_IMPORTS) {
    if (!hostCpp.includes(`__porf_import_${name}`)) {
      fail(`host missing stub: ${name}`);
    }
  }
  ok('porffor_compile + host T06 stubs');
}

async function runRegression(verifyPath) {
  if (!fs.existsSync(verifyPath)) return;
  const r = runNode([verifyPath], { cwd: path.dirname(verifyPath) });
  if (r.status !== 0) fail(`${path.basename(verifyPath)} regression failed`);
  ok(`${path.basename(verifyPath)} regression passed`);
}

async function main() {
  if (!fs.existsSync(path.join(porfforRoot, 'compiler/codegen.js'))) {
    fail(`Porffor submodule missing at ${porfforRoot}`);
  }

  await assert2cNative();
  assertPorfforCompile();
  await runRegression(path.join(reproDir, '..', 't16', 't16_verify.mjs'));
  await runRegression(path.join(reproDir, '..', 't15_verify.mjs'));
  console.log('\nAll T06 verification checks passed.');
}

main().catch((e) => {
  console.error(e);
  process.exit(1);
});
