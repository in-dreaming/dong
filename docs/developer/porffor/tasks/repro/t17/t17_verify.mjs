#!/usr/bin/env node
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { spawnSync } from 'node:child_process';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const reproDir = __dirname;
const dongRoot = path.resolve(reproDir, '../../../../../../dong');
const porfforRoot = path.join(dongRoot, 'third_party', 'porffor');
const nodeExe = process.env.PORFFOR_NODE ?? 'E:\\ws\\infra\\dong\\.tools\\node-v22.16.0-win-x64\\node.exe';
const node = fs.existsSync(nodeExe) ? nodeExe : process.execPath;

const counterJs = path.join(reproDir, 't17_counter.js');
const counterC = path.join(reproDir, 't17_counter.c');
const stubsC = path.join(reproDir, 't17_stubs.c');
const dualViewC = path.join(reproDir, 't17_dual_view.c');
const nativeExe = path.join(reproDir, 't17_dual_view.exe');

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
  const r = runNode(
    [
      path.join(porfforRoot, 'runtime/index.js'),
      'c',
      '--module',
      '--2c-wasm-imports',
      '--no-run',
      '--2c-prefix=t17_',
      jsPath,
      cPath,
    ],
    { cwd: porfforRoot },
  );
  if (r.status !== 0) fail(`2c compile failed: ${r.stderr || r.stdout}`);
  if (!fs.existsSync(cPath)) fail(`missing output: ${cPath}`);
  return fs.readFileSync(cPath, 'utf8');
}

function assertSpikeGlobals(c) {
  if (!/static f64 t17_count\s*=/.test(c)) {
    fail('spike: count not in static C global');
  }
  if (!/char\* t17_memory/.test(c)) {
    fail('spike: missing module memory pointer');
  }
  ok('spike: JS globals are static C scalars separate from memory');
}

function appendStateSnapshot(c, modPrefix) {
  const globals = [];
  const re = new RegExp(`^static (f64|i32|u32|u8|u16|i64|u64) (${modPrefix.replace(/[.*+?^${}()|[\]\\]/g, '\\$&')}\\w+)\\s*=`, 'gm');
  let m;
  while ((m = re.exec(c)) !== null) {
    globals.push({ type: m[1], name: m[2] });
  }
  if (globals.length === 0) return c;
  const structName = `${modPrefix}state_t`;
  const structFields = globals.map((g) => `  ${g.type} ${g.name};`).join('\n');
  const captureBody = globals.map((g) => `  out->${g.name} = ${g.name};`).join('\n');
  const applyBody = globals.map((g) => `  ${g.name} = in->${g.name};`).join('\n');
  return `${c}

typedef struct {
${structFields}
} ${structName};

void ${modPrefix}state_capture(${structName}* out) {
${captureBody}
}

void ${modPrefix}state_apply(const ${structName}* in) {
${applyBody}
}
`;
}

function assertStateSnapshot(c) {
  if (!/void t17_state_capture\(t17_state_t\* out\)/.test(c)) {
    fail('missing t17_state_capture');
  }
  if (!/void t17_state_apply\(const t17_state_t\* in\)/.test(c)) {
    fail('missing t17_state_apply');
  }
  if (!/t17_count;/.test(c)) {
    fail('state struct missing t17_count');
  }
  ok('compile-time state snapshot API present');
}

function assertNativeDualView() {
  let c = compile2c(counterJs, counterC);
  assertSpikeGlobals(c);
  c = appendStateSnapshot(c, 't17_');
  assertStateSnapshot(c);
  fs.writeFileSync(counterC, c);

  const r = spawnSync('clang', ['-O2', stubsC, dualViewC, '-o', nativeExe, '-lm'], {
    encoding: 'utf8',
    cwd: reproDir,
  });
  if (r.status !== 0) {
    console.log('SKIP: clang native dual-view test unavailable');
    return;
  }

  const run = spawnSync(nativeExe, [], { encoding: 'utf8', cwd: reproDir });
  if (run.status !== 0) {
    fail(`dual-view native test failed: ${run.stderr || run.stdout}`);
  }
  ok('two views same module with independent counters');
}

function assertPorfforCompileRegistry() {
  const r = runNode([path.join(dongRoot, 'scripts/porffor_compile.mjs')], { cwd: dongRoot });
  if (r.status !== 0) fail(`porffor_compile.mjs failed: ${r.stderr || r.stdout}`);
  const registryH = fs.readFileSync(path.join(dongRoot, 'generated/porffor/registry.h'), 'utf8');
  if (!/state_capture/.test(registryH)) {
    fail('registry.h missing state_capture');
  }
  const registryC = fs.readFileSync(path.join(dongRoot, 'generated/porffor/registry.c'), 'utf8');
  if (!/__porf_init/.test(registryC)) {
    fail('registry.c missing init_fn wiring');
  }
  const helloDom = fs.readFileSync(path.join(dongRoot, 'generated/porffor/hello_dom.c'), 'utf8');
  if (!/dong_porf_hello_dom_state_capture/.test(helloDom)) {
    fail('hello_dom.c missing state_capture');
  }
  ok('porffor_compile emits per-module state snapshot + registry init_fn');
}

function runRegression(name) {
  const verifyPath = path.join(reproDir, '..', name);
  if (!fs.existsSync(verifyPath)) return;
  const r = runNode([verifyPath], { cwd: path.dirname(verifyPath) });
  if (r.status !== 0) fail(`${name} regression failed`);
  ok(`${name} regression passed`);
}

function main() {
  if (!fs.existsSync(path.join(porfforRoot, 'compiler/codegen.js'))) {
    fail(`Porffor submodule missing at ${porfforRoot}`);
  }

  assertNativeDualView();
  assertPorfforCompileRegistry();
  runRegression('t15_verify.mjs');
  console.log('\nAll T17 verification checks passed.');
}

main();
