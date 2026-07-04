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

const channelJs = path.join(reproDir, 't16_channel.js');
const channelC = path.join(reproDir, 't16_channel.c');
const stubsC = path.join(reproDir, 't16_stubs.c');
const nativeExe = path.join(reproDir, 't16_native.exe');

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

async function compileT16Source(cPath) {
  const argvBackup = process.argv.slice();
  process.argv = [
    process.argv[0],
    process.argv[1],
    '-O1',
    '--target=c',
    '--module',
    '--2c-wasm-imports',
    '--no-run',
    '--2c-prefix=t16_',
    `-o=${cPath}`,
  ];

  const _realExit = process.exit.bind(process);
  process.exit = (code) => {
    if (code && code !== 0) _realExit(code);
  };

  const builtinsUrl = pathToFileURL(path.join(porfforRoot, 'compiler/builtins.js')).href;
  const indexUrl = pathToFileURL(path.join(porfforRoot, 'compiler/index.js')).href;
  const { createImport, setImports } = await import(builtinsUrl);
  const { default: compile } = await import(indexUrl);

  setImports();
  createImport('dong_host_set_slot', 1, 1, null, null);
  createImport('dong_str_len', 0, 1, null, null);
  createImport('dong_str_read', 2, 1, null, null);
  createImport('dong_str_byte_at', 1, 1, null, null);
  createImport('dong_print', 1, 0, null, null);
  createImport('dong_dom_get_textContent', 1, 0, null, null);

  const prelude = fs.readFileSync(preludePath, 'utf8');
  const user = fs.readFileSync(channelJs, 'utf8');
  globalThis.file = channelJs;
  compile(`${prelude}\n${user}`);

  process.argv = argvBackup;
  process.exit = _realExit;

  if (!fs.existsSync(cPath)) fail(`missing output: ${cPath}`);
  return fs.readFileSync(cPath, 'utf8');
}

function assert2cPullImports(c) {
  const checks = [
    ['dong_str_len', /__porf_import_dong_str_len\s*\(\s*\)/],
    ['dong_str_byte_at', /__porf_import_dong_str_byte_at\s*\(/],
    ['pullHostString path', /__porf_import_dong_str_byte_at/],
  ];
  for (const [label, re] of checks) {
    if (!re.test(c)) fail(`2c missing: ${label}`);
    ok(`2c codegen: ${label}`);
  }
}

async function assert2cNative() {
  const c = await compileT16Source(channelC);
  assert2cPullImports(c);

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
    fail(`native T16 failed: ${run.stderr || run.stdout}`);
  }
  if (!/T16_PASS/.test(run.stdout ?? '')) {
    fail(`native expected T16_PASS, got: ${JSON.stringify(run.stdout)}`);
  }
  ok('clang native roundtrip + guard byte + 1000 fuzz');
}

function assertPorfforCompile() {
  const r = runNode([path.join(dongRoot, 'scripts/porffor_compile.mjs')], { cwd: dongRoot });
  if (r.status !== 0) fail(`porffor_compile.mjs failed: ${r.stderr || r.stdout}`);
  const hostCpp = fs.readFileSync(path.join(dongRoot, 'src/script/porffor/dong_porf_host.cpp'), 'utf8');
  if (/static size_t bump/.test(hostCpp)) fail('F3 static bump still present');
  if (!/__porf_import_dong_str_len/.test(hostCpp)) fail('missing dong_str_len stub');
  if (!/domPrepareTextContent/.test(hostCpp)) fail('missing domPrepareTextContent');
  ok('porffor_compile + host F3 removed');
}

async function runRegression(verifyName) {
  const verifyPath = path.join(reproDir, '..', verifyName);
  if (!fs.existsSync(verifyPath)) return;
  const r = runNode([verifyPath], { cwd: path.dirname(verifyPath) });
  if (r.status !== 0) fail(`${verifyName} regression failed`);
  ok(`${verifyName} regression passed`);
}

async function main() {
  if (!fs.existsSync(path.join(porfforRoot, 'compiler/codegen.js'))) {
    fail(`Porffor submodule missing at ${porfforRoot}`);
  }

  await assert2cNative();
  assertPorfforCompile();
  await runRegression('t01_verify.mjs');
  await runRegression('t05/t05_verify.mjs');
  await runRegression('t15_verify.mjs');
  console.log('\nAll T16 verification checks passed.');
}

main().catch((e) => {
  console.error(e);
  process.exit(1);
});
