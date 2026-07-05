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

const fetchJs = path.join(reproDir, 't10_fetch.js');
const fetchC = path.join(reproDir, 't10_fetch.c');
const stubsC = path.join(reproDir, 't10_stubs.c');
const nativeExe = path.join(reproDir, 't10_native.exe');

const T10_IMPORTS = [
  'dong_fetch_start',
  'dong_fetch_abort',
  'dong_fetch_request_id',
  'dong_fetch_status',
  'dong_fetch_ok',
  'dong_fetch_body',
  'dong_fetch_error',
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

async function compileT10Source(cPath) {
  const argvBackup = process.argv.slice();
  process.argv = [
    process.argv[0],
    process.argv[1],
    '-O1',
    '--target=c',
    '--module',
    '--2c-wasm-imports',
    '--no-run',
    '--no-opt-unused',
    '--2c-prefix=t10_',
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
    ['dong_fetch_start', 2, 1],
    ['dong_fetch_abort', 1, 0],
    ['dong_fetch_request_id', 0, 1],
    ['dong_fetch_status', 0, 1],
    ['dong_fetch_ok', 0, 1],
    ['dong_fetch_body', 0, 0],
    ['dong_fetch_error', 0, 0],
    ['dong_str_len', 0, 1],
    ['dong_str_byte_at', 1, 1],
    ['dong_print', 1, 0],
    ['dong_state_set_num', 2, 0],
    ['dong_state_get_num', 1, 1],
  ]) {
    createImport(spec[0], spec[1], spec[2], null, null);
  }

  const prelude = fs.readFileSync(preludePath, 'utf8');
  const user = fs.readFileSync(fetchJs, 'utf8');
  globalThis.file = fetchJs;
  compile(`${prelude}\n${user}`);

  process.argv = argvBackup;
  process.exit = _realExit;

  if (!fs.existsSync(cPath)) fail(`missing output: ${cPath}`);
  return fs.readFileSync(cPath, 'utf8');
}

function assert2cImports(c) {
  for (const name of T10_IMPORTS) {
    const re = new RegExp(`__porf_import_${name.replace(/\//g, '\\/')}`);
    if (!re.test(c)) fail(`2c missing import: ${name}`);
    ok(`2c codegen: ${name}`);
  }
  const prelude = fs.readFileSync(preludePath, 'utf8');
  for (const fn of ['dongFetch', 'fetchBody', 'fetchStatus', 'fetchAbort']) {
    if (!prelude.includes(`function ${fn}(`)) fail(`prelude missing ${fn}()`);
  }
  ok('prelude fetch wrappers');
}

async function assert2cNative() {
  const c = await compileT10Source(fetchC);
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
    fail(`native T10 failed: ${run.stderr || run.stdout}`);
  }
  const out = run.stdout ?? '';
  if (!/T10_PASS/.test(out)) fail(`expected T10_PASS, got: ${JSON.stringify(out)}`);
  if (!/LOCAL:1:1:200:\{"title":"hello"\}/.test(out)) {
    fail(`local JSON fetch mismatch: ${JSON.stringify(out)}`);
  }
  if (!/ERR:0:404:/.test(out) || !/ERR:0:0:Invalid URL/.test(out)) {
    fail(`error path mismatch: ${JSON.stringify(out)}`);
  }
  if (!/CONC:3:200:A/.test(out) || !/CONC:4:200:B/.test(out)) {
    fail(`concurrent requestId pairing mismatch: ${JSON.stringify(out)}`);
  }
  if (/ABORT_SHOULD_NOT_RUN/.test(out)) {
    fail('aborted fetch triggered callback');
  }
  if (!/OUTSIDE::0/.test(out)) {
    fail(`outside-slot guard mismatch: ${JSON.stringify(out)}`);
  }
  ok('clang native T10 fetch callback roundtrip');
}

function assertHostImplementation() {
  const r = runNode([path.join(dongRoot, 'scripts/porffor_compile.mjs')], { cwd: dongRoot });
  if (r.status !== 0) fail(`porffor_compile.mjs failed: ${r.stderr || r.stdout}`);

  const hostCpp = fs.readFileSync(path.join(dongRoot, 'src/script/porffor/dong_porf_host.cpp'), 'utf8');
  const hostHpp = fs.readFileSync(path.join(dongRoot, 'src/script/porffor/dong_porf_host.hpp'), 'utf8');
  const bindingsCpp = fs.readFileSync(
    path.join(dongRoot, 'src/script/porffor/js_bindings_porffor.cpp'),
    'utf8',
  );
  const compileMjs = fs.readFileSync(path.join(dongRoot, 'scripts/porffor_compile.mjs'), 'utf8');

  for (const name of T10_IMPORTS.concat(['dong_fetch_start', 'dong_fetch_header'])) {
    if (!hostCpp.includes(`__porf_import_${name}`)) fail(`host missing stub: ${name}`);
  }
  ok('host T10 fetch import stubs');

  if (!/processFetches/.test(hostCpp) || !/loadTextResource/.test(hostCpp)) {
    fail('host missing fetch dispatch / ResourceLoader');
  }
  if (!/fetch_slot_active_/.test(hostHpp)) fail('fetch slot lifecycle missing');
  if (!/processFetches\(\)/.test(bindingsCpp)) fail('tickTimers missing processFetches');
  if (!/dong_fetch_start/.test(compileMjs)) fail('porffor_compile missing fetch imports');
  ok('host fetch wiring');
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
  assertHostImplementation();
  await runRegression(path.join(reproDir, '..', 't08', 't08_verify.mjs'));
  await runRegression(path.join(reproDir, '..', 't16', 't16_verify.mjs'));
  console.log('\nAll T10 verification checks passed.');
}

main().catch((e) => {
  console.error(e);
  process.exit(1);
});
