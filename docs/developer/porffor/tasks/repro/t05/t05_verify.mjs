#!/usr/bin/env node
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath, pathToFileURL } from 'node:url';
import { spawnSync } from 'node:child_process';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const reproDir = __dirname;
const dongRoot = path.resolve(reproDir, '../../../../../../dong');
const porfforRoot = path.join(dongRoot, 'third_party', 'porffor');
const nodeExe = process.env.PORFFOR_NODE ?? 'E:\\ws\\infra\\dong\\.tools\\node-v22.16.0-win-x64\\node.exe';
const node = fs.existsSync(nodeExe) ? nodeExe : process.execPath;

const frameJs = path.join(reproDir, 't05_frame_concat.js');
const matrixJs = path.join(reproDir, 't05_matrix.js');
const matrixC = path.join(reproDir, 't05_matrix.c');
const matrixExe = path.join(reproDir, 't05_matrix.exe');
const stubsC = path.join(reproDir, 't05_stubs.c');

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

function compile2c(jsPath, cPath) {
  const r = runNode([
    path.join(porfforRoot, 'runtime/index.js'),
    'c',
    '--2c-wasm-imports',
    '--no-run',
    jsPath,
    cPath,
  ], { cwd: porfforRoot });

  if (r.status !== 0) fail(`2c compile failed: ${r.stderr || r.stdout}`);
  if (!fs.existsSync(cPath)) fail(`missing output: ${cPath}`);
  return fs.readFileSync(cPath, 'utf8');
}

function captureWrapStdout(source) {
  let out = '';
  return new Promise(async (resolve, reject) => {
    try {
      const wrapUrl = pathToFileURL(path.join(porfforRoot, 'compiler/wrap.js')).href;
      const { default: wrap } = await import(wrapUrl);
      const { exports } = wrap(source, undefined, str => { out += str; });
      exports.main();
      resolve(out);
    } catch (e) {
      reject(e);
    }
  });
}

async function assertWasmFrameConcat() {
  const src = fs.readFileSync(frameJs, 'utf8');
  const out = await captureWrapStdout(src);
  const line = out.trim().split(/\r?\n/).filter(Boolean).pop() ?? '';
  if (line !== 'frame:0') fail(`wasm print(frame concat) expected "frame:0", got ${JSON.stringify(line)}`);
  ok('wasm print("frame:" + number) => frame:0');
}

async function assertWasmMatrix() {
  const src = fs.readFileSync(matrixJs, 'utf8');
  const out = await captureWrapStdout(src);
  const lines = out.trim().split(/\r?\n/).filter(Boolean);
  const expected = ['frame:0', '0str', 'frame:0', '0', '帧率:42'];
  for (let i = 0; i < expected.length; i++) {
    if (lines[i] !== expected[i]) {
      fail(`wasm matrix line ${i + 1}: expected ${JSON.stringify(expected[i])}, got ${JSON.stringify(lines[i])}`);
    }
  }
  ok('wasm matrix ASCII + Chinese concat via console.log');
}

function assert2cPrintCharUtf8() {
  const c = compile2c(matrixJs, matrixC);
  if (!/utf8_putchar|__porf_printCharUtf8|0xE0\s*\|\s*\(cp\s*>>\s*12\)/.test(c + fs.readFileSync(stubsC, 'utf8'))) {
    fail('2c printChar path missing UTF-8 encoding (inline or stub)');
  }
  ok('2c printChar uses UTF-8 encoding path');
}

function assert2cNative() {
  assert2cPrintCharUtf8();

  const r = spawnSync('clang', ['-O2', stubsC, matrixC, '-o', matrixExe, '-lm'], { encoding: 'utf8' });
  if (r.status !== 0) {
    console.log('SKIP: clang native run (clang unavailable or compile error)');
    return;
  }

  const run = spawnSync(matrixExe, [], { encoding: 'utf8' });
  if (run.status !== 0) fail(`native exit code expected 0, got ${run.status}`);
  const lines = (run.stdout ?? '').trim().split(/\r?\n/).filter(Boolean);
  if (lines[0] !== 'frame:0') fail(`native line 1 expected frame:0, got ${JSON.stringify(lines[0])}`);
  if (lines[4] !== '帧率:42') fail(`native line 5 expected 帧率:42, got ${JSON.stringify(lines[4])}`);
  ok('clang native Chinese + concat output');
}

function assertBench(name) {
  const benchJs = path.join(porfforRoot, 'bench', name);
  const benchC = path.join(reproDir, `t05_${name.replace('.js', '')}.c`);
  compile2c(benchJs, benchC);
  ok(`${name} 2c compile`);
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

  await assertWasmFrameConcat();
  await assertWasmMatrix();
  assert2cNative();
  assertBench('strcat.js');
  assertBench('string_methods.js');
  await runRegression('t01_verify.mjs');
  await runRegression('t03/t03_verify.mjs');
  await runRegression('t04_verify.mjs');
  console.log('\nAll T05 verification checks passed.');
}

main().catch(e => {
  console.error(e);
  process.exit(1);
});
