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

const EACH_PORF = `---
module: porf_each
---

<template>
<div id="list"></div>
<!-- porf-each items as item key id -->
<div><span>{itemText}</span></div>
<!-- /porf-each -->
</template>

<script>
var itemCount = 2;
var itemId0 = 1;
var itemText0 = 'Alpha';
var itemId1 = 2;
var itemText1 = 'Beta';
</script>
`;

test('compiles each block rebuild', () => {
  const r = compilePorfSource(EACH_PORF);
  assert.match(r.js, /function porfRebuild_items/);
  assert.match(r.js, /function itemTextAt\(i\)/);
  assert.match(r.js, /var html = '<div>'/);
  assert.match(r.js, /html = html \+ /);
  assert.match(r.js, /html = html \+ '<\/div>'/);
  assert.match(r.js, /setInnerHTML\(listId, html\)/);
  assert.doesNotMatch(r.js, /appendChild\(listId, parseHtml/);
  assert.match(r.html, /id="list"/);
  assert.ok(!r.html.includes('porf-each'));
});

const EACH_FILTER_PORF = `---
module: porf_each_filter
---

<template>
<div id="list"></div>
<!-- porf-each items as item key id container=list filter=show(i) row-fn=buildRow -->
<span>{itemText}</span>
<!-- /porf-each -->
</template>

<script>
var itemCount = 1;
var itemId0 = 1;
var itemText0 = 'x';
function show(i) { return 1; }
function buildRow(i) { return '<b>' + itemTextAt(i) + '</b>'; }
</script>
`;

test('compiles each with filter and row-fn', () => {
  const r = compilePorfSource(EACH_FILTER_PORF);
  assert.match(r.js, /if \(show\(i\)\)/);
  assert.match(r.js, /html = html \+ buildRow\(i\)/);
  assert.match(r.js, /setInnerHTML\(listId, html\)/);
});

const PARTIAL_PORF = `---
module: porf_partial
---

<template>
<!-- porf-partial name="Btn" -->
<button id="{{id}}" style="background:{{color}}">{{label}}</button>
<!-- /porf-partial -->
<div>
  <!-- porf-use partial="Btn" props="id=go color=#27ae60 label=Go" -->
</div>
</template>

<script>
var x = 0;
</script>
`;

test('expands porf-partial / porf-use', () => {
  const r = compilePorfSource(PARTIAL_PORF);
  assert.match(r.html, /id="go"/);
  assert.match(r.html, /background:#27ae60/);
  assert.match(r.html, />Go</);
  assert.ok(!r.html.includes('porf-partial'));
  assert.ok(!r.html.includes('porf-use'));
});

console.log('porffor_framework_compile: all tests passed');
