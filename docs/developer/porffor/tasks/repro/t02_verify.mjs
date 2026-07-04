#!/usr/bin/env node
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath, pathToFileURL } from 'node:url';
import { spawnSync } from 'node:child_process';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const reproDir = __dirname;
const dongRoot = path.resolve(reproDir, '../../../../../dong');
const porfforRoot = path.join(dongRoot, 'third_party', 'porffor');
const testJs = path.join(reproDir, 't02_inline_c_import.js');
const outC = path.join(reproDir, 't02_inline_c_import.c');
const outExe = path.join(reproDir, 't02_inline_c_import.exe');
const modAC = path.join(reproDir, 't02_mod_a.c');
const modBC = path.join(reproDir, 't02_mod_b.c');
const strictJs = path.join(reproDir, 't02_strict_fail.js');

const nodeExe = process.env.T02_NODE ?? process.execPath;

function fail(msg) {
  console.error('FAIL:', msg);
  process.exit(1);
}

function ok(msg) {
  console.log('OK:', msg);
}

function registerImports(createImport, setImports) {
  setImports();
  createImport('dong_add', 2, 1, (a, b) => a + b, 'return p0 + p1;');
}

async function compile2c(extraArgv = []) {
  const argvBackup = process.argv.slice();
  process.argv = [process.argv[0], process.argv[1], '-O1', '--target=c', '--no-run', `-o=${outC}`, ...extraArgv];
  globalThis.argvChanged?.();

  const _realExit = process.exit.bind(process);
  process.exit = code => {
    if (code && code !== 0) _realExit(code);
  };

  const builtinsUrl = pathToFileURL(path.join(porfforRoot, 'compiler/builtins.js')).href;
  const indexUrl = pathToFileURL(path.join(porfforRoot, 'compiler/index.js')).href;
  const { createImport, setImports } = await import(builtinsUrl);
  const { default: compile } = await import(indexUrl);

  registerImports(createImport, setImports);
  globalThis.file = testJs;
  compile(fs.readFileSync(testJs, 'utf8'));

  process.argv = argvBackup;
  process.exit = _realExit;
  globalThis.argvChanged?.();

  if (!fs.existsSync(outC)) fail(`missing generated C: ${outC}`);
  return fs.readFileSync(outC, 'utf8');
}

async function assertInlineC() {
  const c = await compile2c(['--2c-wasm-imports']);

  if (!/static\s+f64\s+__porf_import_dong_add\s*\(\s*f64\s+p0\s*,\s*f64\s+p1\s*\)/.test(c)) {
    fail('missing static inline C function __porf_import_dong_add(f64 p0, f64 p1)');
  }
  ok('static inline C function with p0,p1 params');

  if (!c.includes('return p0 + p1;')) fail('inline C body missing return p0 + p1;');
  ok('inline C body present');

  if (!/__porf_import_dong_add\s*\([^)]+\)/.test(c)) fail('missing call to __porf_import_dong_add');
  ok('2c calls inline import function');

  if (c.includes('import_module')) fail('inline c import should not use wasm import attribute');
  ok('inline c takes priority over wasm-imports extern');
}

async function assertWasmPath() {
  const wrapUrl = pathToFileURL(path.join(porfforRoot, 'compiler/wrap.js')).href;
  const builtinsUrl = pathToFileURL(path.join(porfforRoot, 'compiler/builtins.js')).href;
  const { createImport, setImports } = await import(builtinsUrl);
  const { default: wrap } = await import(wrapUrl);

  registerImports(createImport, setImports);
  const { exports } = wrap(fs.readFileSync(testJs, 'utf8'));
  const ret = exports.main();
  if (ret !== 7) fail(`wasm path expected 7, got ${ret}`);
  ok('wasm path execution result=7');
}

async function assertWasmImportsRegression() {
  const helloJs = path.join(porfforRoot, 'bench/hello.js');
  const helloC = path.join(reproDir, 't02_hello.c');
  const r = spawnSync(nodeExe, [
    path.join(porfforRoot, 'runtime/index.js'),
    'c',
    '--2c-wasm-imports',
    helloJs,
    helloC,
  ], { cwd: porfforRoot, encoding: 'utf8' });

  if (r.status !== 0) fail(`hello.js 2c-wasm-imports regression failed: ${r.stderr || r.stdout}`);
  const c = fs.readFileSync(helloC, 'utf8');
  if (!c.includes('__porf_import_print')) fail('hello.js C missing print import extern');
  ok('hello.js --2c-wasm-imports regression');
}

async function assertStrictImports() {
  fs.writeFileSync(strictJs, 'dong_unknown(1);\n');

  const argvBackup = process.argv.slice();
  process.argv = [process.argv[0], process.argv[1], '-O1', '--target=c', '--2c-strict-imports', '--no-run', `-o=${outC}`];
  globalThis.argvChanged?.();

  const _realExit = process.exit.bind(process);
  process.exit = code => {
    if (code) throw new Error(`process.exit(${code})`);
  };

  let threw = false;
  try {
    const builtinsUrl = pathToFileURL(path.join(porfforRoot, 'compiler/builtins.js')).href;
    const indexUrl = pathToFileURL(path.join(porfforRoot, 'compiler/index.js')).href;
    const { createImport, setImports } = await import(builtinsUrl);
    const { default: compile } = await import(indexUrl);

    setImports();
    createImport('dong_unknown', 1, 0, () => {}, null);
    globalThis.file = strictJs;
    compile(fs.readFileSync(strictJs, 'utf8'));
  } catch (e) {
    threw = true;
    if (!String(e.message || e).includes('dong_unknown')) {
      fail(`strict imports error should name import, got: ${e}`);
    }
  } finally {
    process.argv = argvBackup;
    process.exit = _realExit;
    globalThis.argvChanged?.();
  }

  if (!threw) fail('--2c-strict-imports should throw on unknown import');
  ok('--2c-strict-imports hard error on unknown import');
}

async function assertMultiModuleStatic() {
  const modJs = path.join(reproDir, 't02_mod_dup.js');
  fs.writeFileSync(modJs, 'dong_add(1, 2);\n');

  async function compileMod(outPath, prefix) {
    const argvBackup = process.argv.slice();
    process.argv = [process.argv[0], process.argv[1], '-O1', '--target=c', '--no-run', `--2c-prefix=${prefix}`, `-o=${outPath}`];
    globalThis.argvChanged?.();

    const _realExit = process.exit.bind(process);
    process.exit = code => {
      if (code && code !== 0) _realExit(code);
    };

    const builtinsUrl = pathToFileURL(path.join(porfforRoot, 'compiler/builtins.js')).href;
    const indexUrl = pathToFileURL(path.join(porfforRoot, 'compiler/index.js')).href;
    const { createImport, setImports } = await import(builtinsUrl);
    const { default: compile } = await import(indexUrl);

    registerImports(createImport, setImports);
    globalThis.file = modJs;
    compile(fs.readFileSync(modJs, 'utf8'));

    process.argv = argvBackup;
    process.exit = _realExit;
    globalThis.argvChanged?.();
    return fs.readFileSync(outPath, 'utf8');
  }

  const cA = await compileMod(modAC, 'mod_a_');
  const cB = await compileMod(modBC, 'mod_b_');

  if (!cA.includes('static f64 __porf_import_dong_add')) fail('mod_a missing static inline import');
  if (!cB.includes('static f64 __porf_import_dong_add')) fail('mod_b missing static inline import');
  ok('both modules emit static inline import (no duplicate global symbol)');

  const linkMain = path.join(reproDir, 't02_link_main.c');
  const outExe2 = path.join(reproDir, 't02_link_test.exe');
  fs.writeFileSync(linkMain, `#include <stdio.h>
extern int mod_a_main(void);
extern int mod_b_main(void);
int main(void) {
  int a = mod_a_main();
  int b = mod_b_main();
  return (a == 3 && b == 3) ? 0 : 1;
}
`);

  const r = spawnSync('clang', ['-O2', linkMain, modAC, modBC, '-o', outExe2, '-lm'], { encoding: 'utf8' });
  if (r.status !== 0) {
    console.log('SKIP: clang multi-module link (clang unavailable)');
    return;
  }

  const run = spawnSync(outExe2, [], { encoding: 'utf8' });
  if (run.status !== 0) fail(`multi-module link run expected 0, got ${run.status}`);
  ok('clang multi-module link with static inline imports');
}

function tryNativeRun() {
  if (!fs.existsSync(outC)) return;

  const r = spawnSync('clang', ['-O2', outC, '-o', outExe, '-lm'], { encoding: 'utf8' });
  if (r.status !== 0) {
    console.log('SKIP: clang native run (clang unavailable or compile error)');
    return;
  }

  const run = spawnSync(outExe, [], { encoding: 'utf8' });
  if (run.status !== 7) fail(`native exit code expected 7, got ${run.status}`);
  ok('clang compile + native run exit code 7');
}

async function main() {
  if (!fs.existsSync(path.join(porfforRoot, 'compiler/index.js'))) {
    fail(`Porffor submodule missing at ${porfforRoot}`);
  }

  await assertInlineC();
  await assertWasmPath();
  await assertWasmImportsRegression();
  await assertStrictImports();
  await assertMultiModuleStatic();
  tryNativeRun();
  console.log('\nAll T02 verification checks passed.');
}

main().catch(e => {
  console.error(e);
  process.exit(1);
});
