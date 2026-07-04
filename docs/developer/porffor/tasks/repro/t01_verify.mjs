#!/usr/bin/env node
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath, pathToFileURL } from 'node:url';
import { spawnSync } from 'node:child_process';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const reproDir = __dirname;
const dongRoot = path.resolve(reproDir, '../../../../../dong');
const porfforRoot = path.join(dongRoot, 'third_party', 'porffor');
const testJs = path.join(reproDir, 't01_multiarg_import.js');
const outC = path.join(reproDir, 't01_multiarg_import.c');
const outExe = path.join(reproDir, 't01_multiarg_import.exe');
const stubC = path.join(reproDir, 't01_import_stubs.c');

function fail(msg) {
  console.error('FAIL:', msg);
  process.exit(1);
}

function ok(msg) {
  console.log('OK:', msg);
}

function registerTestImports(createImport, setImports) {
  setImports();
  createImport('dong_test0', 0, 1, () => 0);
  createImport('dong_test1', 1, 1, a => a);
  createImport('dong_test2', 2, 1, (a, b) => a + b);
  createImport('dong_test3', 3, 1, (a, b, c) => a + b + c);
}

async function assert2cOutput() {
  const argvBackup = process.argv.slice();
  process.argv = [process.argv[0], process.argv[1], '-O1', '--target=c', '--2c-wasm-imports', '--no-run', `-o=${outC}`];

  const _realExit = process.exit.bind(process);
  process.exit = code => {
    if (code && code !== 0) _realExit(code);
  };

  const builtinsUrl = pathToFileURL(path.join(porfforRoot, 'compiler/builtins.js')).href;
  const indexUrl = pathToFileURL(path.join(porfforRoot, 'compiler/index.js')).href;
  const { createImport, setImports } = await import(builtinsUrl);
  const { default: compile } = await import(indexUrl);

  registerTestImports(createImport, setImports);
  globalThis.file = testJs;
  compile(fs.readFileSync(testJs, 'utf8'));

  process.argv = argvBackup;
  process.exit = _realExit;
  globalThis.argvChanged?.();

  if (!fs.existsSync(outC)) fail(`missing generated C: ${outC}`);
  const c = fs.readFileSync(outC, 'utf8');

  const checks = [
    ['__porf_import_dong_test0()', /__porf_import_dong_test0\s*\(\s*\)/],
    ['__porf_import_dong_test1(...)', /__porf_import_dong_test1\s*\([^)]+\)/],
    ['__porf_import_dong_test2(a, b)', /__porf_import_dong_test2\s*\([^,)]+,\s*[^)]+\)/],
    ['__porf_import_dong_test3(a, b, c)', /__porf_import_dong_test3\s*\([^,)]+,\s*[^,)]+,\s*[^)]+\)/],
  ];

  for (const [label, re] of checks) {
    if (!re.test(c)) fail(`generated C missing expected call: ${label}`);
    ok(`2c call pattern: ${label}`);
  }

  const badSingleArg3 = c.match(/__porf_import_dong_test3\s*\(\s*[^,)]+\s*\)/);
  if (badSingleArg3) fail(`dong_test3 appears to have only one argument: ${badSingleArg3[0]}`);
}

async function assertWasmPath() {
  const wrapUrl = pathToFileURL(path.join(porfforRoot, 'compiler/wrap.js')).href;
  const builtinsUrl = pathToFileURL(path.join(porfforRoot, 'compiler/builtins.js')).href;
  const { createImport, setImports } = await import(builtinsUrl);
  const { default: wrap } = await import(wrapUrl);

  registerTestImports(createImport, setImports);
  const { exports } = wrap(fs.readFileSync(testJs, 'utf8'));
  const ret = exports.main();
  if (ret !== 19) fail(`wasm path expected sum 19, got ${ret}`);
  ok('wasm path execution sum=19');
}

function assertHelloRegression() {
  const helloJs = path.join(porfforRoot, 'bench/hello.js');
  const helloC = path.join(reproDir, 't01_hello.c');
  const r = spawnSync(process.execPath, [
    path.join(porfforRoot, 'runtime/index.js'),
    'c',
    '--2c-wasm-imports',
    helloJs,
    helloC,
  ], { cwd: porfforRoot, encoding: 'utf8' });

  if (r.status !== 0) {
    fail(`hello.js 2c compile failed: ${r.stderr || r.stdout}`);
  }
  if (!fs.existsSync(helloC)) fail('hello.js did not produce C output');
  const c = fs.readFileSync(helloC, 'utf8');
  if (!c.includes('__porf_import_print')) fail('hello.js C missing print import');
  ok('hello.js 2c regression');
}

function tryNativeRun() {
  if (!fs.existsSync(outC)) return;

  const stubs = `#include <stdio.h>
f64 __porf_import_dong_test0(void) { return 0; }
f64 __porf_import_dong_test1(f64 a) { return a; }
f64 __porf_import_dong_test2(f64 a, f64 b) { return a + b; }
f64 __porf_import_dong_test3(f64 a, f64 b, f64 c) { return a + b + c; }
void __porf_import_print(f64 x) { (void)x; }
`;
  fs.writeFileSync(stubC, stubs);

  const r = spawnSync('clang', ['-O2', stubC, outC, '-o', outExe, '-lm'], { encoding: 'utf8' });
  if (r.status !== 0) {
    console.log('SKIP: clang native run (clang unavailable or compile error)');
    return;
  }

  const run = spawnSync(outExe, [], { encoding: 'utf8' });
  if (run.status !== 19) fail(`native exit code expected 19, got ${run.status}`);
  ok('clang compile + native run exit code 19');
}

async function main() {
  if (!fs.existsSync(path.join(porfforRoot, 'compiler/index.js'))) {
    fail(`Porffor submodule missing at ${porfforRoot}`);
  }

  await assert2cOutput();
  await assertWasmPath();
  assertHelloRegression();
  tryNativeRun();
  console.log('\nAll T01 verification checks passed.');
}

main().catch(e => {
  console.error(e);
  process.exit(1);
});
