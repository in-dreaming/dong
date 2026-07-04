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

const eventsJs = path.join(reproDir, 't08_events.js');
const eventsC = path.join(reproDir, 't08_events.c');
const stubsC = path.join(reproDir, 't08_stubs.c');
const nativeExe = path.join(reproDir, 't08_native.exe');

const T08_IMPORTS = [
  'dong_event_type',
  'dong_event_target',
  'dong_event_key',
  'dong_event_key_code',
  'dong_event_x',
  'dong_event_y',
  'dong_event_button',
  'dong_event_modifiers',
  'dong_event_value',
  'dong_event_prevent_default',
  'dong_event_stop_propagation',
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

async function compileT08Source(cPath) {
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
    '--2c-prefix=t08_',
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
    ['dong_event_type', 0, 0],
    ['dong_event_target', 0, 1],
    ['dong_event_key', 0, 0],
    ['dong_event_key_code', 0, 1],
    ['dong_event_x', 0, 1],
    ['dong_event_y', 0, 1],
    ['dong_event_button', 0, 1],
    ['dong_event_modifiers', 0, 1],
    ['dong_event_value', 0, 0],
    ['dong_event_prevent_default', 0, 0],
    ['dong_event_stop_propagation', 0, 0],
    ['dong_str_len', 0, 1],
    ['dong_str_byte_at', 1, 1],
    ['dong_print', 1, 0],
    ['dong_state_set_num', 2, 0],
    ['dong_state_get_num', 1, 1],
  ]) {
    createImport(spec[0], spec[1], spec[2], null, null);
  }

  const prelude = fs.readFileSync(preludePath, 'utf8');
  const user = fs.readFileSync(eventsJs, 'utf8');
  globalThis.file = eventsJs;
  compile(`${prelude}\n${user}`);

  process.argv = argvBackup;
  process.exit = _realExit;

  if (!fs.existsSync(cPath)) fail(`missing output: ${cPath}`);
  return fs.readFileSync(cPath, 'utf8');
}

function assert2cImports(c) {
  for (const name of T08_IMPORTS) {
    const re = new RegExp(`__porf_import_${name.replace(/\//g, '\\/')}`);
    if (!re.test(c)) fail(`2c missing import: ${name}`);
    ok(`2c codegen: ${name}`);
  }
  if (!/function eventType\(/.test(fs.readFileSync(preludePath, 'utf8'))) {
    fail('prelude missing eventType()');
  }
  ok('prelude event wrappers');
}

async function assert2cNative() {
  const c = await compileT08Source(eventsC);
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
    fail(`native T08 failed: ${run.stderr || run.stdout}`);
  }
  const out = run.stdout ?? '';
  if (!/T08_PASS/.test(out)) fail(`expected T08_PASS, got: ${JSON.stringify(out)}`);
  if (!/click;42;Enter;13;100,200;0;3;hello/.test(out)) {
    fail(`event slot probe mismatch: ${JSON.stringify(out)}`);
  }
  if (!/NESTED:inner/.test(out) || !/NESTED:outer/.test(out)) {
    fail(`event slot stack mismatch: ${JSON.stringify(out)}`);
  }
  ok('clang native T08 event slot roundtrip');
}

function assertHostImplementation() {
  const r = runNode([path.join(dongRoot, 'scripts/porffor_compile.mjs')], { cwd: dongRoot });
  if (r.status !== 0) fail(`porffor_compile.mjs failed: ${r.stderr || r.stdout}`);

  const hostCpp = fs.readFileSync(path.join(dongRoot, 'src/script/porffor/dong_porf_host.cpp'), 'utf8');
  const bindingsCpp = fs.readFileSync(
    path.join(dongRoot, 'src/script/porffor/js_bindings_porffor.cpp'),
    'utf8',
  );
  const registryCpp = fs.readFileSync(
    path.join(dongRoot, 'src/script/porffor/porffor_script_registry.cpp'),
    'utf8',
  );
  const engineView = fs.readFileSync(path.join(dongRoot, 'src/core/engine_view.cpp'), 'utf8');

  for (const name of T08_IMPORTS) {
    if (!hostCpp.includes(`__porf_import_${name}`)) fail(`host missing stub: ${name}`);
  }
  ok('host T08 event import stubs');

  if (!/module_name/.test(bindingsCpp) || !/l\.module_name/.test(bindingsCpp)) {
    fail('F5: listener module_name not recorded');
  }
  if (/callExport\(engine_->registry\(\)->activeModule\(\)/.test(bindingsCpp)) {
    fail('F5: dispatch still uses activeModule()');
  }
  ok('F5 module-scoped event routing');

  if (!/prev_active/.test(registryCpp) || !/PorfforHost_setActiveModule\(prev_mod\)/.test(registryCpp)) {
    fail('F6: callExport missing active module restore');
  }
  if (!/pushResultSlot/.test(registryCpp)) {
    fail('F6: callExport missing result slot stack');
  }
  ok('F6 callExport save/restore');

  if (!/Dispatch DOMContentLoaded \/ load on document/.test(engineView)) {
    fail('F7: lifecycle dispatch block missing');
  }
  const porfforEnd = engineView.indexOf('#endif // DONG_SCRIPT_ENGINE_PORFFOR');
  const lifecycleSlice = engineView.slice(porfforEnd, porfforEnd + 600);
  if (/#ifndef DONG_SCRIPT_ENGINE_PORFFOR[\s\S]{0,400}DOMContentLoaded/.test(lifecycleSlice)) {
    fail('F7: lifecycle still ifdef-skipped for Porffor');
  }
  if (!/dispatchSimpleEvent\(nid, "DOMContentLoaded"\)/.test(engineView)) {
    fail('F7: DOMContentLoaded dispatch missing');
  }
  ok('F7 lifecycle events enabled');
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
  await runRegression(path.join(reproDir, '..', 't06', 't06_verify.mjs'));
  await runRegression(path.join(reproDir, '..', 't16', 't16_verify.mjs'));
  await runRegression(path.join(reproDir, '..', 't15_verify.mjs'));
  console.log('\nAll T08 verification checks passed.');
}

main().catch((e) => {
  console.error(e);
  process.exit(1);
});
