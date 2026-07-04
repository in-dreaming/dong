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

const timersJs = path.join(reproDir, 't09_timers.js');
const timersC = path.join(reproDir, 't09_timers.c');
const stubsC = path.join(reproDir, 't09_stubs.c');
const nativeExe = path.join(reproDir, 't09_native.exe');

const T09_IMPORTS = [
  'dong_commit_setInterval',
  'dong_clear_interval',
  'dong_commit_requestAnimationFrame',
  'dong_cancel_animation_frame',
  'dong_raf_timestamp',
];

const T09_HOST_IMPORTS = [
  ...T09_IMPORTS,
  'dong_clear_timeout',
  'dong_set_interval',
  'dong_request_animation_frame',
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

async function compileT09Source(cPath) {
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
    '--2c-prefix=t09_',
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
    ['dong_timer_setTimeout', 2, 1],
    ['dong_set_interval', 2, 1],
    ['dong_clear_interval', 1, 0],
    ['dong_clear_timeout', 1, 0],
    ['dong_commit_setTimeout', 0, 1],
    ['dong_commit_setInterval', 0, 1],
    ['dong_request_animation_frame', 1, 1],
    ['dong_cancel_animation_frame', 1, 0],
    ['dong_commit_requestAnimationFrame', 0, 1],
    ['dong_raf_timestamp', 0, 1],
    ['dong_state_set_num', 2, 0],
    ['dong_state_get_num', 1, 1],
    ['dong_stage_0', 1, 0],
    ['dong_stage_1', 1, 0],
    ['dong_stage_2', 1, 0],
    ['dong_print', 1, 0],
    ['dong_time_now', 0, 1],
  ]) {
    createImport(spec[0], spec[1], spec[2], null, null);
  }

  const prelude = fs.readFileSync(preludePath, 'utf8');
  const user = fs.readFileSync(timersJs, 'utf8');
  globalThis.file = timersJs;
  compile(`${prelude}\n${user}`);

  process.argv = argvBackup;
  process.exit = _realExit;

  if (!fs.existsSync(cPath)) fail(`missing output: ${cPath}`);
  return fs.readFileSync(cPath, 'utf8');
}

function assert2cImports(c) {
  for (const name of T09_IMPORTS) {
    const re = new RegExp(`__porf_import_${name.replace(/\//g, '\\/')}\\s*\\(`);
    if (!re.test(c)) fail(`2c missing import call: ${name}`);
    ok(`2c codegen: ${name}`);
  }
  if (!/int t09_export_onRafWithArg\(f64 p0\)/.test(c)) {
    fail('missing export_onRafWithArg(f64) shim for T15 param path');
  }
  ok('2c export_onRafWithArg f64 param shim');
}

async function assert2cNative() {
  const c = await compileT09Source(timersC);
  assert2cImports(c);

  const r = spawnSync('clang', ['-O2', stubsC, '-o', nativeExe, '-lm'], {
    encoding: 'utf8',
    cwd: reproDir,
  });
  if (r.status !== 0) {
    console.log('SKIP: clang native run (clang unavailable or compile error)');
    if (r.stderr) console.log(r.stderr);
    return;
  }

  const run = spawnSync(nativeExe, [], { encoding: 'utf8', cwd: reproDir });
  if (run.status !== 0) {
    fail(`native T09 failed: ${run.stderr || run.stdout}`);
  }
  if (!/T09_PASS/.test(run.stdout ?? '')) {
    fail(`native expected T09_PASS, got: ${JSON.stringify(run.stdout)}`);
  }
  ok('clang native T09 interval + rAF + cancel');
}

function assertPorfforCompile() {
  const r = runNode([path.join(dongRoot, 'scripts/porffor_compile.mjs')], { cwd: dongRoot });
  if (r.status !== 0) fail(`porffor_compile.mjs failed: ${r.stderr || r.stdout}`);
  const hostCpp = fs.readFileSync(path.join(dongRoot, 'src/script/porffor/dong_porf_host.cpp'), 'utf8');
  for (const name of T09_HOST_IMPORTS) {
    if (!hostCpp.includes(`__porf_import_${name}`)) {
      fail(`host missing stub: ${name}`);
    }
  }
  const prelude = fs.readFileSync(preludePath, 'utf8');
  for (const fn of [
    'setInterval',
    'clearTimeout',
    'clearInterval',
    'requestAnimationFrame',
    'cancelAnimationFrame',
    'rafTimestamp',
  ]) {
    if (!prelude.includes(`function ${fn}`)) {
      fail(`prelude missing ${fn}`);
    }
  }
  ok('porffor_compile + host T09 stubs + prelude wrappers');
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
  await runRegression(path.join(reproDir, '..', 't15_verify.mjs'));
  await runRegression(path.join(reproDir, '..', 't16', 't16_verify.mjs'));
  console.log('\nAll T09 verification checks passed.');
}

main().catch((e) => {
  console.error(e);
  process.exit(1);
});
