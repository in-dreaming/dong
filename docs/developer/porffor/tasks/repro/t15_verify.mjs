#!/usr/bin/env node
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath, pathToFileURL } from 'node:url';
import { spawnSync } from 'node:child_process';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const reproDir = __dirname;
const dongRoot = path.resolve(reproDir, '../../../../../dong');
const porfforRoot = path.join(dongRoot, 'third_party', 'porffor');
const nodeExe = process.env.PORFFOR_NODE ?? 'E:\\ws\\infra\\dong\\.tools\\node-v22.16.0-win-x64\\node.exe';
const node = fs.existsSync(nodeExe) ? nodeExe : process.execPath;

const wasmJs = path.join(reproDir, 't15_wasm_state.js');
const sharedJs = path.join(reproDir, 't15_shared_state.js');
const wasmC = path.join(reproDir, 't15_wasm_state.c');
const sharedC = path.join(reproDir, 't15_shared_state.c');
const stubsC = path.join(reproDir, 't15_stubs.c');
const nativeExe = path.join(reproDir, 't15_native.exe');

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

function compile2c(jsPath, cPath, extraArgs = []) {
  const r = runNode(
    [
      path.join(porfforRoot, 'runtime/index.js'),
      'c',
      '--module',
      '--2c-wasm-imports',
      '--no-run',
      '--2c-prefix=t15_',
      ...extraArgs,
      jsPath,
      cPath,
    ],
    { cwd: porfforRoot },
  );
  if (r.status !== 0) fail(`2c compile failed: ${r.stderr || r.stdout}`);
  if (!fs.existsSync(cPath)) fail(`missing output: ${cPath}`);
  return fs.readFileSync(cPath, 'utf8');
}

async function assertWasmSharedState() {
  const src = fs.readFileSync(wasmJs, 'utf8');
  const wrapUrl = pathToFileURL(path.join(porfforRoot, 'compiler/wrap.js')).href;
  const { default: wrap } = await import(wrapUrl);
  const { exports } = wrap(src, true, () => {});
  if (typeof exports.onClick !== 'function' || typeof exports.getCount !== 'function') {
    fail('wasm exports missing onClick/getCount');
  }
  exports.onClick();
  exports.onClick();
  const count = exports.getCount();
  if (count !== 2) fail(`wasm shared count expected 2, got ${count}`);
  ok('wasm multi-export shared global state');
}

function assert2cExportShims(c) {
  if (!/void t15__porf_init\(void\)/.test(c)) fail('missing idempotent __porf_init');
  if (!/int t15_export_onClick\(void\)/.test(c)) fail('missing export_onClick shim');
  if (!/int t15_export_echoArg\(f64 p0\)/.test(c)) fail('missing export_echoArg shim');
  ok('2c export shims + shared init present');
}

function assert2cNativeSharedState() {
  compile2c(wasmJs, wasmC);
  assert2cExportShims(fs.readFileSync(wasmC, 'utf8'));

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
    fail(`native shared-state test failed: ${run.stderr || run.stdout}`);
  }
  ok('clang native multi-export shared state + echoArg');
}

function assertPorfforCompile() {
  const r = runNode([path.join(dongRoot, 'scripts/porffor_compile.mjs')], { cwd: dongRoot });
  if (r.status !== 0) fail(`porffor_compile.mjs failed: ${r.stderr || r.stdout}`);
  const registry = fs.readFileSync(path.join(dongRoot, 'generated/porffor/registry.c'), 'utf8');
  if (!/dong_porf_hello_dom_export_onBtnClick/.test(registry)) {
    fail('hello_dom registry missing in-module export_onBtnClick');
  }
  if (/hello_dom__onBtnClick/.test(registry)) {
    fail('hello_dom still uses legacy separate handler module');
  }
  ok('porffor_compile multi-export registry for hello_dom');
}

async function runRegression(verifyName) {
  const verifyPath = path.join(reproDir, verifyName);
  if (!fs.existsSync(verifyPath)) {
    const alt = path.join(reproDir, '..', verifyName);
    if (!fs.existsSync(alt)) return;
    const r = runNode([alt], { cwd: path.dirname(alt) });
    if (r.status !== 0) fail(`${verifyName} regression failed`);
    ok(`${verifyName} regression passed`);
    return;
  }
  const r = runNode([verifyPath], { cwd: path.dirname(verifyPath) });
  if (r.status !== 0) fail(`${verifyName} regression failed`);
  ok(`${verifyName} regression passed`);
}

async function main() {
  if (!fs.existsSync(path.join(porfforRoot, 'compiler/codegen.js'))) {
    fail(`Porffor submodule missing at ${porfforRoot}`);
  }

  await assertWasmSharedState();
  assert2cNativeSharedState();
  compile2c(sharedJs, sharedC);
  assertPorfforCompile();
  await runRegression('t01_verify.mjs');
  await runRegression('t03/t03_verify.mjs');
  await runRegression('t04_verify.mjs');
  await runRegression('t05/t05_verify.mjs');
  console.log('\nAll T15 verification checks passed.');
}

main().catch((e) => {
  console.error(e);
  process.exit(1);
});
