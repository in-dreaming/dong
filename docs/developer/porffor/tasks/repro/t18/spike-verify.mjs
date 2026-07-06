#!/usr/bin/env node
/** T18 spike — generated JS from framework compiler must pass `porf c`. */
import { spawnSync } from 'node:child_process';
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { compilePorfSource } from '../../../../../../dong/scripts/porffor_framework_compile.mjs';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const porfforRoot = path.resolve(__dirname, '../../../../../../dong/third_party/porffor');
const prelude = fs.readFileSync(
  path.resolve(__dirname, '../../../../../../dong/src/script/porffor/dong_porffor_prelude.js'),
  'utf8',
);

const samples = [
  {
    name: 'counter_bind',
    porf: `---
module: spike_counter
---
<template><div id="n" data-porf-bind="count">{count}</div></template>
<script>var count = 0;</script>`,
  },
  {
    name: 'if_block',
    porf: `---
module: spike_if
---
<template><div id="box" data-porf-if="show">x</div></template>
<script>var show = 1;</script>`,
  },
];

let ok = 0;
for (const s of samples) {
  const { js } = compilePorfSource(s.porf, { fileLabel: s.name });
  const tmpJs = path.join(__dirname, `_spike_${s.name}.js`);
  const tmpC = path.join(__dirname, `_spike_${s.name}.c`);
  fs.writeFileSync(tmpJs, `${prelude}\n${js}`);
  const r = spawnSync(process.execPath, ['runtime/index.js', 'c', tmpJs, tmpC], {
    cwd: porfforRoot,
    encoding: 'utf8',
  });
  if (r.status !== 0) {
    console.error(`FAIL ${s.name}:`, r.stderr || r.stdout);
    process.exit(1);
  }
  console.log('OK spike', s.name);
  ok++;
}
console.log(`t18 spike-verify: ${ok}/${samples.length} passed`);
