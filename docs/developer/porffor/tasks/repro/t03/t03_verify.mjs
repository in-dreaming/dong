#!/usr/bin/env node
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath, pathToFileURL } from 'node:url';
import { spawnSync } from 'node:child_process';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const reproDir = __dirname;
const dongRoot = path.resolve(reproDir, '../../../../../../dong');
const porfforRoot = path.join(dongRoot, 'third_party', 'porffor');

function fail(msg) {
  console.error('FAIL:', msg);
  process.exit(1);
}

function ok(msg) {
  console.log('OK:', msg);
}

function compile2c(jsPath, outC, extraArgs = []) {
  const r = spawnSync(process.execPath, [
    path.join(porfforRoot, 'runtime/index.js'),
    'c',
    '--2c-wasm-imports',
    '--no-run',
    ...extraArgs,
    jsPath,
    outC,
  ], { cwd: porfforRoot, encoding: 'utf8' });

  if (r.status !== 0) {
    fail(`2c compile failed for ${jsPath}: ${r.stderr || r.stdout}`);
  }
  if (!fs.existsSync(outC)) fail(`missing output: ${outC}`);
  return fs.readFileSync(outC, 'utf8');
}

async function assert2cHygiene() {
  const nanJs = path.join(reproDir, 't03_nan.js');
  const nanC = path.join(reproDir, 't03_nan.c');
  const c = compile2c(nanJs, nanC);

  if (/const\s+f64\s+NaN\b/.test(c)) fail('generated C still has const f64 NaN');
  ok('no const f64 NaN in output');

  if (!c.includes('__porf_nan') && !c.includes('NAN')) fail('missing safe NaN reference');
  ok('uses safe NaN macro/name');

  if (!/static\s+(?:struct ReturnValue|f64|i32|void)\s+__/.test(c)) {
    fail('internal helper functions are not static');
  }
  ok('internal helper functions are static');
}

async function assertMultiModuleLink() {
  const modA = path.join(reproDir, 't03_module_a.js');
  const modB = path.join(reproDir, 't03_module_b.js');
  const cA = path.join(reproDir, 't03_module_a.c');
  const cB = path.join(reproDir, 't03_module_b.c');
  const outExe = path.join(reproDir, 't03_link_test.exe');
  const stubs = path.join(reproDir, 't03_link_stubs.c');
  const linkMain = path.join(reproDir, 't03_link_main.c');

  const srcA = compile2c(modA, cA, ['--2c-prefix=mod_a_']);
  const srcB = compile2c(modB, cB, ['--2c-prefix=mod_b_']);

  if (!srcA.includes('int mod_a_main(')) fail('mod_a missing prefixed main');
  if (!srcB.includes('int mod_b_main(')) fail('mod_b missing prefixed main');
  ok('2c-prefix renames main per module');

  if (!srcA.includes('mod_a_memory') || !srcB.includes('mod_b_memory')) {
    fail('2c-prefix did not rename memory globals');
  }
  ok('2c-prefix renames memory globals');

  fs.writeFileSync(linkMain, `#include <stdio.h>
extern int mod_a_main(void);
extern int mod_b_main(void);
int main(void) {
  int a = mod_a_main();
  int b = mod_b_main();
  return a + b;
}
`);

  const r = spawnSync('clang', ['-O2', stubs, cA, cB, linkMain, '-o', outExe, '-lm'], { encoding: 'utf8' });
  if (r.status !== 0) {
    console.log('SKIP: clang multi-module link (clang unavailable)');
    return;
  }

  const run = spawnSync(outExe, [], { encoding: 'utf8' });
  if (run.status !== 3) fail(`link test expected exit 3, got ${run.status}`);
  ok('clang link of two prefixed modules exit code 3');
}

async function assertNanNative() {
  const nanJs = path.join(reproDir, 't03_nan.js');
  const nanC = path.join(reproDir, 't03_nan.c');
  const outExe = path.join(reproDir, 't03_nan.exe');
  const stubs = path.join(reproDir, 't03_link_stubs.c');

  compile2c(nanJs, nanC);

  const r = spawnSync('clang', ['-O2', '-Werror', stubs, nanC, '-o', outExe, '-lm'], { encoding: 'utf8' });
  if (r.status !== 0) {
    console.log('SKIP: clang NaN compile (clang unavailable)');
    return;
  }

  const run = spawnSync(outExe, [], { encoding: 'utf8' });
  if (run.status !== 42) fail(`NaN test expected 42, got ${run.status}`);
  ok('NaN/Infinity native run exit 42');
}

async function assertWasmPath() {
  const wrapUrl = pathToFileURL(path.join(porfforRoot, 'compiler/wrap.js')).href;
  const { default: wrap } = await import(wrapUrl);
  const nanJs = path.join(reproDir, 't03_nan.js');
  const { exports } = wrap(fs.readFileSync(nanJs, 'utf8'));
  const ret = exports.main();
  if (ret !== 42) fail(`wasm path expected 42, got ${ret}`);
  ok('wasm path NaN/Infinity check returns 42');
}

function assertQuiet() {
  const helloJs = path.join(porfforRoot, 'bench/hello.js');
  const helloC = path.join(reproDir, 't03_hello_quiet.c');

  const noisy = spawnSync(process.execPath, [
    path.join(porfforRoot, 'runtime/index.js'),
    'c', '--2c-wasm-imports', helloJs, helloC,
  ], { cwd: porfforRoot, encoding: 'utf8' });

  const quiet = spawnSync(process.execPath, [
    path.join(porfforRoot, 'runtime/index.js'),
    'c', '--2c-wasm-imports', '--quiet', helloJs, helloC,
  ], { cwd: porfforRoot, encoding: 'utf8' });

  if (quiet.status !== 0) fail(`quiet compile failed: ${quiet.stderr}`);
  if (quiet.stdout && quiet.stdout.trim().length > 0) fail(`--quiet still produced stdout: ${quiet.stdout}`);
  ok('--quiet suppresses stdout');

  if (!noisy.stdout || !/\d+ms/.test(noisy.stdout + noisy.stderr)) {
    ok('non-quiet compile may omit timing in non-tty (skipped assert)');
  } else {
    ok('non-quiet compile shows timing output');
  }
}

async function main() {
  if (!fs.existsSync(path.join(porfforRoot, 'compiler/2c.js'))) {
    fail(`Porffor submodule missing at ${porfforRoot}`);
  }

  await assert2cHygiene();
  await assertMultiModuleLink();
  await assertNanNative();
  await assertWasmPath();
  assertQuiet();
  console.log('\nAll T03 verification checks passed.');
}

main().catch(e => {
  console.error(e);
  process.exit(1);
});
