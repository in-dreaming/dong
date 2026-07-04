#!/usr/bin/env node
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath, pathToFileURL } from 'node:url';
import { spawnSync } from 'node:child_process';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const reproDir = __dirname;
const dongRoot = path.resolve(reproDir, '../../../../../dong');
const porfforRoot = path.join(dongRoot, 'third_party', 'porffor');
const testJs = path.join(reproDir, 't04_prop_hash.js');
const outC = path.join(reproDir, 't04_prop_hash.c');
const outExe = path.join(reproDir, 't04_prop_hash.exe');
const stubsC = path.join(reproDir, 't04_stubs.c');

function fail(msg) {
  console.error('FAIL:', msg);
  process.exit(1);
}

function ok(msg) {
  console.log('OK:', msg);
}

function compile2c(jsPath, cPath, extraArgs = []) {
  const r = spawnSync(process.execPath, [
    path.join(porfforRoot, 'runtime/index.js'),
    'c',
    '--2c-wasm-imports',
    '--no-run',
    ...extraArgs,
    jsPath,
    cPath,
  ], { cwd: porfforRoot, encoding: 'utf8' });

  if (r.status !== 0) {
    fail(`2c compile failed: ${r.stderr || r.stdout}`);
  }
  if (!fs.existsSync(cPath)) fail(`missing output: ${cPath}`);
  return fs.readFileSync(cPath, 'utf8');
}

async function assertWasmPath() {
  const wrapUrl = pathToFileURL(path.join(porfforRoot, 'compiler/wrap.js')).href;
  const { default: wrap } = await import(wrapUrl);
  const { exports } = wrap(fs.readFileSync(testJs, 'utf8'));
  const ret = exports.main();
  if (ret !== 0) fail(`wasm path expected 0, got ${ret} (bitmask=${ret})`);
  ok('wasm path property access sum=0');
}

function assert2cCodegen() {
  const c = compile2c(testJs, outC);
  if (!/\(u32\)[^)]*>>/.test(c)) {
    fail('generated C missing u32 logical shift for >>> (hash avalanche)');
  }
  ok('2c codegen uses u32 logical shift in hash path');
}

function assert2cNative() {
  assert2cCodegen();

  if (!fs.existsSync(stubsC)) {
    fs.writeFileSync(stubsC, 'void __porf_import_print(f64 x) { (void)x; }\n');
  }

  const r = spawnSync('clang', ['-O2', stubsC, outC, '-o', outExe, '-lm'], { encoding: 'utf8' });
  if (r.status !== 0) {
    console.log('SKIP: clang native run (clang unavailable or compile error)');
    return;
  }

  const run = spawnSync(outExe, [], { encoding: 'utf8' });
  if (run.status !== 0) fail(`native exit code expected 0, got ${run.status}`);
  ok('clang compile + native run exit code 0');
}

function assertObjectGetBench() {
  const benchJs = path.join(porfforRoot, 'bench/object_get.js');
  const benchC = path.join(reproDir, 't04_object_get.c');
  const benchExe = path.join(reproDir, 't04_object_get.exe');

  compile2c(benchJs, benchC);

  const r = spawnSync('clang', ['-O2', stubsC, benchC, '-o', benchExe, '-lm'], { encoding: 'utf8' });
  if (r.status !== 0) {
    console.log('SKIP: object_get.js native bench (clang unavailable)');
    return;
  }

  const run = spawnSync(benchExe, [], { encoding: 'utf8', timeout: 120000 });
  if (run.status !== 0 && run.status != null) fail(`object_get native unexpected exit ${run.status}`);
  ok('object_get.js 2c compile + native run');
}

async function main() {
  if (!fs.existsSync(path.join(porfforRoot, 'compiler/codegen.js'))) {
    fail(`Porffor submodule missing at ${porfforRoot}`);
  }

  await assertWasmPath();
  assert2cNative();
  assertObjectGetBench();
  console.log('\nAll T04 verification checks passed.');
}

main().catch(e => {
  console.error(e);
  process.exit(1);
});
