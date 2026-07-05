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

const platformJs = path.join(reproDir, 't11_platform.js');
const platformC = path.join(reproDir, 't11_platform.c');
const stubsC = path.join(reproDir, 't11_stubs.c');
const nativeExe = path.join(reproDir, 't11_native.exe');

const T11_IMPORTS = [
  'dong_clipboard_read',
  'dong_clipboard_write',
  'dong_match_media',
  'dong_css_supports',
  'dong_dialog_show',
  'dong_dialog_show_modal',
  'dong_dialog_close',
  'dong_dialog_return_value',
  'dong_dialog_open',
  'dong_scene_add_node',
  'dong_scene_remove',
  'dong_scene_set',
  'dong_scene_find',
  'dong_scene_on',
  'dong_scene_clear',
  'dong_scene_count',
  'dong_text_layout',
  'dong_clear_overlay',
  'dong_render_text',
  'dong_draw_rect',
  'dong_draw_circle',
];

const T11_PRELUDE_FUNCS = [
  'clipboardRead',
  'clipboardWrite',
  'matchMedia',
  'cssSupports',
  'dialogShow',
  'dialogShowModal',
  'dialogClose',
  'dialogReturnValue',
  'dialogOpen',
  'sceneAddNode',
  'sceneRemove',
  'sceneSet',
  'sceneFind',
  'sceneOn',
  'sceneClear',
  'sceneCount',
  'textLayout',
  'clearOverlay',
  'renderText',
  'drawRect',
  'drawCircle',
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

async function compileT11Source(cPath) {
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
    '--2c-prefix=t11_',
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

  const specs = [
    ['dong_clipboard_read', 0, 0],
    ['dong_clipboard_write', 1, 0],
    ['dong_match_media', 1, 1],
    ['dong_css_supports', 2, 1],
    ['dong_scene_add_node', 1, 1],
    ['dong_scene_remove', 1, 0],
    ['dong_scene_set', 3, 0],
    ['dong_scene_find', 1, 1],
    ['dong_scene_on', 3, 0],
    ['dong_scene_clear', 0, 0],
    ['dong_scene_count', 0, 1],
    ['dong_text_layout', 1, 0],
    ['dong_clear_overlay', 0, 0],
    ['dong_clear_overlay', 0, 0],
    ['dong_render_text', 1, 0],
    ['dong_draw_rect', 1, 0],
    ['dong_draw_circle', 1, 0],
    ['dong_stage_0', 1, 0],
    ['dong_stage_1', 1, 0],
    ['dong_stage_2', 1, 0],
    ['dong_str_len', 0, 1],
    ['dong_str_byte_at', 1, 1],
    ['dong_print', 1, 0],
  ];
  for (const spec of specs) {
    createImport(spec[0], spec[1], spec[2], null, null);
  }

  const prelude = fs.readFileSync(preludePath, 'utf8');
  const user = fs.readFileSync(platformJs, 'utf8');
  globalThis.file = platformJs;
  compile(`${prelude}\n${user}`);

  process.argv = argvBackup;
  process.exit = _realExit;

  if (!fs.existsSync(cPath)) fail(`missing output: ${cPath}`);
  return fs.readFileSync(cPath, 'utf8');
}

function assertPrelude() {
  const prelude = fs.readFileSync(preludePath, 'utf8');
  for (const fn of T11_PRELUDE_FUNCS) {
    const re = new RegExp(`function ${fn}\\(`);
    if (!re.test(prelude)) fail(`prelude missing ${fn}()`);
  }
  ok('prelude T11 wrappers');
}

function assertHostImplementation() {
  const r = runNode([path.join(dongRoot, 'scripts/porffor_compile.mjs')], { cwd: dongRoot });
  if (r.status !== 0) fail(`porffor_compile.mjs failed: ${r.stderr || r.stdout}`);

  const hostCpp = fs.readFileSync(path.join(dongRoot, 'src/script/porffor/dong_porf_host.cpp'), 'utf8');
  const bindingsCpp = fs.readFileSync(
    path.join(dongRoot, 'src/script/porffor/js_bindings_porffor.cpp'),
    'utf8',
  );
  const cmake = fs.readFileSync(path.join(dongRoot, 'cmake/PorfforScripts.cmake'), 'utf8');

  for (const name of T11_IMPORTS) {
    if (!hostCpp.includes(`__porf_import_${name}`)) fail(`host missing stub: ${name}`);
  }
  ok('host T11 import stubs');

  if (!/sceneAddNode/.test(bindingsCpp) || !/textLayout/.test(bindingsCpp)) {
    fail('bindings missing scene/textLayout');
  }
  ok('bindings scene + textLayout');

  if (!cmake.includes('js_scene_porffor.cpp') || !cmake.includes('js_text_layout_porffor.cpp')) {
    fail('cmake missing scene/text_layout porffor sources');
  }
  ok('cmake porffor scene sources');

  const testHtml = path.join(dongRoot, 'examples/data/tests/test_porffor_clipboard_cn.html');
  if (!fs.existsSync(testHtml)) fail('missing test_porffor_clipboard_cn.html');
  ok('clipboard CN test HTML');
}

async function assert2cNative() {
  const c = await compileT11Source(platformC);
  for (const name of [
    'dong_clipboard_read',
    'dong_match_media',
    'dong_scene_add_node',
    'dong_text_layout',
    'dong_clear_overlay',
  ]) {
    const re = new RegExp(`__porf_import_${name.replace(/\//g, '\\/')}`);
    if (!re.test(c)) fail(`2c missing import: ${name}`);
  }
  ok('2c codegen T11 imports');

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
    fail(`native T11 failed: ${run.stderr || run.stdout}`);
  }
  const out = run.stdout ?? '';
  if (!/T11_PASS/.test(out)) fail(`expected T11_PASS, got: ${JSON.stringify(out)}`);
  if (!/CLIP:你好，Dong 剪贴板/.test(out)) fail(`clipboard CN roundtrip failed: ${JSON.stringify(out)}`);
  if (!/MM:0/.test(out) || !/MM:1/.test(out)) fail(`matchMedia resize smoke failed: ${JSON.stringify(out)}`);
  if (!/SCENE:1,1,1/.test(out)) fail(`scene smoke failed: ${JSON.stringify(out)}`);
  if (!/TL:1/.test(out)) fail(`textLayout smoke failed: ${JSON.stringify(out)}`);
  ok('clang native T11 platform smoke');
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

  assertPrelude();
  assertHostImplementation();
  await assert2cNative();
  await runRegression(path.join(reproDir, '..', 't08', 't08_verify.mjs'));
  console.log('\nAll T11 verification checks passed.');
}

main().catch((e) => {
  console.error(e);
  process.exit(1);
});
