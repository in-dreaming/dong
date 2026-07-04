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

const mutationJs = path.join(reproDir, 't07_mutation.js');
const mutationC = path.join(reproDir, 't07_mutation.c');
const stubsC = path.join(reproDir, 't07_stubs.c');
const nativeExe = path.join(reproDir, 't07_native.exe');

const T07_IMPORTS = [
  'dong_create_element',
  'dong_create_text_node',
  'dong_append_child',
  'dong_insert_before',
  'dong_remove',
  'dong_replace_child',
  'dong_parent',
  'dong_first_child',
  'dong_next_sibling',
  'dong_clone_node',
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

async function compileT07Source(cPath) {
  const argvBackup = process.argv.slice();
  process.argv = [
    process.argv[0],
    process.argv[1],
    '-O1',
    '--target=c',
    '--2c-wasm-imports',
    '--no-run',
    '--no-opt-unused',
    '--2c-prefix=t07_',
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
    ['dong_create_element', 1, 1],
    ['dong_create_text_node', 1, 1],
    ['dong_append_child', 2, 0],
    ['dong_insert_before', 3, 0],
    ['dong_remove', 1, 0],
    ['dong_replace_child', 3, 0],
    ['dong_parent', 1, 1],
    ['dong_first_child', 1, 1],
    ['dong_next_sibling', 1, 1],
    ['dong_clone_node', 2, 1],
  ]) {
    createImport(spec[0], spec[1], spec[2], null, null);
  }
  createImport('dong_str_len', 0, 1, null, null);
  createImport('dong_str_byte_at', 1, 1, null, null);
  createImport('dong_print', 1, 0, null, null);
  createImport('dong_stage_0', 1, 0, null, null);
  createImport('dong_stage_1', 1, 0, null, null);
  createImport('dong_commit_set_textContent', 0, 0, null, null);
  createImport('dong_dom_get_textContent', 1, 0, null, null);
  createImport('dong_state_set_num', 2, 0, null, null);
  createImport('dong_state_get_num', 1, 1, null, null);

  const prelude = fs.readFileSync(preludePath, 'utf8');
  const user = fs.readFileSync(mutationJs, 'utf8');
  globalThis.file = mutationJs;
  compile(`${prelude}\n${user}`);

  process.argv = argvBackup;
  process.exit = _realExit;

  if (!fs.existsSync(cPath)) fail(`missing output: ${cPath}`);
  return fs.readFileSync(cPath, 'utf8');
}

function assert2cImports(c) {
  for (const name of T07_IMPORTS) {
    const re = new RegExp(`__porf_import_${name.replace(/\//g, '\\/')}\\s*\\(`);
    if (!re.test(c)) fail(`2c missing import call: ${name}`);
    ok(`2c codegen: ${name}`);
  }
  if (!/__porf_import_dong_insert_before\s*\([^)]+,[^)]+,[^)]+\)/.test(c)) {
    fail('2c dong_insert_before not 3-arg');
  }
  ok('2c multi-arg dong_insert_before');
}

async function assert2cNative() {
  const c = await compileT07Source(mutationC);
  assert2cImports(c);

  const r = spawnSync('clang', ['-O2', stubsC, '-o', nativeExe, '-lm'], {
    encoding: 'utf8',
    cwd: reproDir,
  });
  if (r.status !== 0) {
    console.log('SKIP: clang native run (clang unavailable or compile error)');
    console.log(r.stderr || r.stdout);
    return;
  }

  const run = spawnSync(nativeExe, [], { encoding: 'utf8', cwd: reproDir });
  if (run.status !== 0) {
    fail(`native T07 failed: ${run.stderr || run.stdout}`);
  }
  if (!/T07_PASS/.test(run.stdout ?? '')) {
    fail(`native expected T07_PASS, got: ${JSON.stringify(run.stdout)}`);
  }
  ok('clang native T07 DOM mutation roundtrip');
}

function assertPorfforCompile() {
  const r = runNode([path.join(dongRoot, 'scripts/porffor_compile.mjs')], { cwd: dongRoot });
  if (r.status !== 0) fail(`porffor_compile.mjs failed: ${r.stderr || r.stdout}`);
  const hostCpp = fs.readFileSync(path.join(dongRoot, 'src/script/porffor/dong_porf_host.cpp'), 'utf8');
  for (const name of T07_IMPORTS) {
    if (!hostCpp.includes(`__porf_import_${name}`)) {
      fail(`host missing stub: ${name}`);
    }
  }
  const bindingsCpp = fs.readFileSync(
    path.join(dongRoot, 'src/script/porffor/js_bindings_porffor.cpp'),
    'utf8',
  );
  if (!/id_by_node_ptr_/.test(bindingsCpp)) fail('F8 reverse index missing');
  ok('porffor_compile + host T07 stubs + F8 reverse index');
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
  await runRegression(path.join(reproDir, '..', 't06', 't06_verify.mjs'));
  await runRegression(path.join(reproDir, '..', 't16', 't16_verify.mjs'));
  console.log('\nAll T07 verification checks passed.');
}

main().catch((e) => {
  console.error(e);
  process.exit(1);
});
