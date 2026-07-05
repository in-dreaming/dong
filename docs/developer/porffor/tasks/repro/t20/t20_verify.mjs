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

const T20_IMPORTS = ['dong_parse_html', 'dong_form_serialize', 'dong_selection_text'];
const T20_PRELUDE_FNS = ['parseHtml', 'formSerialize', 'selectionText'];

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

async function compileSnippet(jsName, cName, prefix, expectImports) {
  const jsPath = path.join(reproDir, jsName);
  const cPath = path.join(reproDir, cName);
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
    '--quiet',
    `--2c-prefix=${prefix}_`,
    `-o=${cPath}`,
  ];
  globalThis.argvChanged?.();

  const _realExit = process.exit.bind(process);
  process.exit = (code) => {
    if (code && code !== 0) _realExit(code);
  };

  try {
    const builtinsUrl = pathToFileURL(path.join(porfforRoot, 'compiler/builtins.js')).href;
    const indexUrl = pathToFileURL(path.join(porfforRoot, 'compiler/index.js')).href;
    const { createImport } = await import(builtinsUrl);
    const { default: compile } = await import(indexUrl);

    for (const name of T20_IMPORTS) {
      createImport(name, name === 'dong_selection_text' ? 0 : 1, 0, null, null);
    }
    for (const spec of [
      ['dong_dom_getElementById', 1, 1],
      ['dong_dom_get_textContent', 1, 0],
      ['dong_first_child', 1, 1],
      ['dong_str_len', 0, 1],
      ['dong_str_byte_at', 1, 1],
      ['dong_print', 1, 0],
    ]) {
      createImport(spec[0], spec[1], spec[2], null, null);
    }

    const prelude = fs.readFileSync(preludePath, 'utf8');
    const user = fs.readFileSync(jsPath, 'utf8');
    globalThis.file = jsPath;
    compile(`${prelude}\n${user}`);
  } finally {
    process.argv = argvBackup;
    process.exit = _realExit;
  }

  if (!fs.existsSync(cPath)) fail(`missing ${cPath}`);
  const c = fs.readFileSync(cPath, 'utf8');
  for (const name of expectImports) {
    if (!new RegExp(`__porf_import_${name}`).test(c)) fail(`2c missing ${name} in ${jsName}`);
  }
  ok(`2c imports for ${jsName}`);
}

function assertHostAndPrelude() {
  const hostCpp = fs.readFileSync(path.join(dongRoot, 'src/script/porffor/dong_porf_host.cpp'), 'utf8');
  for (const name of T20_IMPORTS) {
    if (!hostCpp.includes(`__porf_import_${name}`)) fail(`host missing ${name}`);
  }
  const prelude = fs.readFileSync(preludePath, 'utf8');
  for (const fn of T20_PRELUDE_FNS) {
    if (!prelude.includes(`function ${fn}`)) fail(`prelude missing ${fn}`);
  }
  ok('host stubs + prelude wrappers');
}

async function main() {
  await compileSnippet('t20_parse.js', 't20_parse.c', 't20_parse', ['dong_parse_html']);
  await compileSnippet('t20_form.js', 't20_form.c', 't20_form', ['dong_form_serialize']);
  await compileSnippet('t20_selection.js', 't20_selection.c', 't20_selection', ['dong_selection_text']);
  assertHostAndPrelude();
  const r = runNode([path.join(dongRoot, 'scripts/porffor_compile.mjs')], { cwd: dongRoot });
  if (r.status !== 0) fail(`porffor_compile: ${r.stderr || r.stdout}`);
  ok('porffor_compile with T20 imports');
  console.log('\nAll T20 verification checks passed.');
}

main().catch((e) => {
  console.error(e);
  process.exit(1);
});
