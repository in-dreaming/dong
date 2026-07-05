#!/usr/bin/env node
import assert from 'node:assert/strict';
import {
  parsePorfforTagFromHtml,
  classifyScriptTest,
  detectEvalSnippetRef,
  extractPorfforModule,
} from './porffor_test_tags.mjs';

function test(name, fn) {
  try {
    fn();
    console.log(`ok ${name}`);
  } catch (e) {
    console.error(`FAIL ${name}`);
    throw e;
  }
}

test('explicit ready', () => {
  const r = parsePorfforTagFromHtml('<!-- porffor: ready --><html></html>');
  assert.equal(r.status, 'ready');
  assert.equal(r.explicit, true);
});

test('explicit pending', () => {
  const r = parsePorfforTagFromHtml('<!-- porffor: pending --><html><script></script></html>');
  assert.equal(r.status, 'pending');
});

test('blocked with task id', () => {
  const r = parsePorfforTagFromHtml('<!-- porffor: blocked(T18) --><html></html>');
  assert.equal(r.status, 'blocked');
  assert.equal(r.reason, 'T18');
});

test('dropped with reason', () => {
  const r = parsePorfforTagFromHtml('<!-- porffor: dropped(legacy demo) --><html></html>');
  assert.equal(r.status, 'dropped');
  assert.equal(r.reason, 'legacy demo');
});

test('default ready without script', () => {
  const r = parsePorfforTagFromHtml('<html><body>static</body></html>');
  assert.equal(r.status, 'ready');
  assert.equal(r.explicit, false);
});

test('default pending with script', () => {
  const r = parsePorfforTagFromHtml('<html><script>x=1</script></html>');
  assert.equal(r.status, 'pending');
  assert.equal(r.explicit, false);
});

test('explicit tag overrides script default', () => {
  const r = parsePorfforTagFromHtml('<!-- porffor: ready --><script></script>');
  assert.equal(r.status, 'ready');
});

test('extract porffor module', () => {
  assert.equal(extractPorfforModule('<html data-porffor-module="hello_dom">'), 'hello_dom');
});

test('detect eval snippet in comment', () => {
  const d = detectEvalSnippetRef(
    '<p>use --eval-after-frame0-file snippets/ce_bold_after_frame0.js</p>',
  );
  assert.equal(d.usesEvalSnippet, true);
  assert.equal(d.snippetPath, 'snippets/ce_bold_after_frame0.js');
});

test('classify CE as T20', () => {
  const c = classifyScriptTest('<script>document.execCommand("bold")</script>', 'test_ce.html');
  assert.equal(c.bucket, 'blocked(T20)');
});

test('classify simple DOM as direct', () => {
  const c = classifyScriptTest(
    '<script>console.log(document.getElementById("x").textContent)</script>',
    'test_simple.html',
  );
  assert.equal(c.bucket, 'direct');
});

console.log('porffor_test_tags: all tests passed');
