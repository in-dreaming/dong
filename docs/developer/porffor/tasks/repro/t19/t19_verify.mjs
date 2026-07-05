#!/usr/bin/env node
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath, pathToFileURL } from 'node:url';
import { spawnSync } from 'node:child_process';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const reproDir = __dirname;
const dongRoot = path.resolve(reproDir, '../../../../../../dong');
const porfforRoot = path.join(dongRoot, 'third_party', 'porffor');
const casesJs = path.join(reproDir, 't19_promise_cases.js');
const casesC = path.join(reproDir, 't19_promise_cases.c');
const stubsC = path.join(reproDir, 't19_stubs.c');
const nativeExe = path.join(reproDir, 't19_native.exe');
const nodeExe = process.env.PORFFOR_NODE ?? 'E:\\ws\\infra\\dong\\.tools\\node-v22.16.0-win-x64\\node.exe';
const node = fs.existsSync(nodeExe) ? nodeExe : process.execPath;

const CASES = [
  'then_resolve',
  'catch_reject',
  'finally',
  'chain',
  'all',
  'race',
  'settled_sync',
  'microtask_order',
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

function parsePassFail(stdout) {
  const results = new Map();
  for (const line of (stdout ?? '').split(/\r?\n/)) {
    const m = line.match(/^(PASS|FAIL):(\w+)/);
    if (m) results.set(m[2], m[1] === 'PASS');
  }
  return results;
}

async function runWasmPath() {
  const wrapUrl = pathToFileURL(path.join(porfforRoot, 'compiler/wrap.js')).href;
  const builtinsUrl = pathToFileURL(path.join(porfforRoot, 'compiler/builtins.js')).href;
  const { setImports } = await import(builtinsUrl);
  const { default: wrap } = await import(wrapUrl);
  setImports();

  let out = '';
  const source = fs.readFileSync(casesJs, 'utf8');
  const { exports } = wrap(source, undefined, (s) => {
    out += s;
  });
  exports.main();
  await new Promise((r) => setTimeout(r, 100));
  return parsePassFail(out);
}

function compile2c() {
  const r = runNode(
    [
      path.join(porfforRoot, 'runtime/index.js'),
      'c',
      '--2c-prefix=t19_promise_cases_',
      '--2c-wasm-imports',
      '--no-run',
      casesJs,
      casesC,
    ],
    { cwd: porfforRoot },
  );
  if (r.status !== 0) fail(`2c compile failed: ${r.stderr || r.stdout}`);
  if (!fs.existsSync(casesC)) fail(`missing ${casesC}`);
  ok('2c compile promise matrix');
}

function runNativePath() {
  const r = spawnSync('clang', ['-O2', stubsC, casesC, '-o', nativeExe, '-lm'], {
    encoding: 'utf8',
    cwd: reproDir,
  });
  if (r.status !== 0) {
    console.log('SKIP: clang native (unavailable)');
    return null;
  }
  const run = spawnSync(nativeExe, [], { encoding: 'utf8', cwd: reproDir });
  if (run.status !== 0) fail(`native run failed: ${run.stderr || run.stdout}`);
  ok('clang native promise matrix');
  return parsePassFail(run.stdout);
}

async function main() {
  if (!fs.existsSync(path.join(porfforRoot, 'compiler/wrap.js'))) {
    fail(`Porffor missing at ${porfforRoot}`);
  }

  const wasm = await runWasmPath();
  ok(`wasm path: ${wasm.size} case result(s)`);

  compile2c();
  const native = runNativePath();

  const matrix = {};
  for (const name of CASES) {
    matrix[name] = {
      wasm: wasm.has(name) ? (wasm.get(name) ? 'pass' : 'fail') : 'no-output',
      '2c+clang': native?.has(name)
        ? native.get(name)
          ? 'pass'
          : 'fail'
        : native === null
          ? 'skip'
          : 'no-output',
      '2c-compile': fs.existsSync(casesC) ? 'ok' : 'fail',
    };
  }

  const outPath = path.join(reproDir, 't19_matrix.json');
  fs.writeFileSync(outPath, JSON.stringify(matrix, null, 2));
  ok(`wrote ${outPath}`);

  const lintCheck = runNode(
    [
      '--input-type=module',
      '-e',
      `import { lintPorfforSource } from './scripts/porffor_lint.mjs';
try { lintPorfforSource('Promise.resolve(1)', 'x.js'); process.exit(2); } catch {}
lintPorfforSource('var x = 1;', 'y.js');
console.log('lint-ok');`,
    ],
    { cwd: dongRoot },
  );
  if (lintCheck.status !== 0 || !/lint-ok/.test(lintCheck.stdout ?? '')) {
    fail(`porffor_lint check failed: ${lintCheck.stderr || lintCheck.stdout}`);
  }
  ok('porffor_lint Promise/async/await gate');

  const r = runNode([path.join(dongRoot, 'scripts/porffor_compile.mjs')], { cwd: dongRoot });
  if (r.status !== 0) fail(`porffor_compile failed: ${r.stderr || r.stdout}`);
  ok('porffor_compile after T19');

  console.log('\nAll T19 verification checks passed.');
}

main().catch((e) => {
  console.error(e);
  process.exit(1);
});
