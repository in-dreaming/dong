#!/usr/bin/env node
/**
 * T22 golden snapshot tests for porffor_framework_compile.mjs
 */
import assert from 'node:assert/strict';
import { compilePorfSource } from './porffor_framework_compile.mjs';

function test(name, fn) {
  try {
    fn();
    console.log('ok', name);
  } catch (e) {
    console.error('FAIL', name);
    throw e;
  }
}

const COUNTER_PORF = `---
module: porf_counter
exports: onInc
---

<template>
<div id="count-display" data-porf-bind="count">{count}</div>
<button id="btn" data-porf-on="click:onInc">+</button>
</template>

<script>
var count = 0;
export function onInc() {
  count = count + 1;
  porfPatch_count();
}
</script>
`;

test('compiles text bind and event', () => {
  const r = compilePorfSource(COUNTER_PORF, { fileLabel: 'counter.porf' });
  assert.equal(r.module, 'porf_counter');
  assert.match(r.js, /function porfPatch_count/);
  assert.match(r.js, /addEventListener\(btnId, 'click', 'onInc'\)/);
  assert.match(r.html, /id="count-display"/);
  assert.ok(!r.html.includes('data-porf-on'));
});

test('rejects arrow functions', () => {
  const bad = COUNTER_PORF.replace('count = count + 1', 'count = (() => 1)()');
  assert.throws(() => compilePorfSource(bad), /arrow functions/);
});

test('rejects Promise', () => {
  const bad = COUNTER_PORF + '\nvar p = Promise.resolve(1);';
  assert.throws(() => compilePorfSource(bad), /Promise/);
});

const IF_PORF = `---
module: porf_if
---

<template>
<div id="box" data-porf-if="show">x</div>
</template>

<script>
var show = 1;
</script>
`;

test('compiles if block patch', () => {
  const r = compilePorfSource(IF_PORF);
  assert.match(r.js, /function porfPatchIf_box/);
  assert.match(r.js, /removeAttribute\(boxId, 'hidden'\)/);
});

console.log('porffor_framework_compile: all tests passed');
