#!/usr/bin/env node
/**
 * Final wave: migrate all remaining blocked(T06|T10|T11|T12|T19|T20) tests.
 *
 * Actions: ready | blocked(T20) | dropped(reason) | fetch-module | exec-module
 *
 * Usage: node scripts/porffor_batch_migrate_remaining.mjs [--dry-run]
 */
import fs from 'node:fs';
import path from 'node:path';
import { dongRoot } from './porffor_paths.mjs';
import { parsePorfforTagFromHtml } from './porffor_test_tags.mjs';

const testsDir = path.join(dongRoot, 'examples', 'data', 'tests');
const porfforTestsDir = path.join(dongRoot, 'examples', 'porffor', 'tests');
const manifestPath = path.join(dongRoot, 'scripts', 'porffor_manifest.json');
const dryRun = process.argv.includes('--dry-run');

const FETCH_MODULES = {
  'test_fetch_json_basic.html': {
    name: 't10_fetch_json',
    js: `var resultId = getElementById('result');

export function onFetchDone() {
  var ok = fetchOk();
  var status = fetchStatus();
  var body = fetchBody();
  if (ok === 0) {
    setTextContent(resultId, 'FAIL: ' + fetchError());
    classAdd(resultId, 'fail');
    return;
  }
  var expect = '{"greeting": "hello from dong", "version": 1, "items": ["alpha", "beta", "gamma"]}';
  if (status === 200 && body === expect) {
    setTextContent(resultId, 'PASS: greeting=hello from dong, items=alpha,beta,gamma');
    classAdd(resultId, 'pass');
  } else {
    setTextContent(resultId, 'FAIL: unexpected response');
    classAdd(resultId, 'fail');
  }
}

dongFetch('test_fetch_local.json', 'onFetchDone');
`,
    mf: { frames: 2 },
  },
  'test_fetch_text_basic.html': {
    name: 't10_fetch_text',
    js: `var resultId = getElementById('result');

export function onFetchDone() {
  var ok = fetchOk();
  var body = fetchBody();
  if (ok === 0) {
    setTextContent(resultId, 'FAIL: ' + fetchError());
    classAdd(resultId, 'fail');
    return;
  }
  var expect = 'This is a local text file loaded via fetch().\\nIt should appear in the DOM after the fetch completes.';
  if (body === expect) {
    setTextContent(resultId, 'PASS: ' + body);
    classAdd(resultId, 'pass');
  } else {
    setTextContent(resultId, 'FAIL: unexpected content');
    classAdd(resultId, 'fail');
  }
}

dongFetch('test_fetch_local.txt', 'onFetchDone');
`,
    mf: { frames: 2 },
  },
  'test_fetch_reject.html': {
    name: 't10_fetch_reject',
    js: `var resultId = getElementById('result');

export function onFetchDone() {
  var ok = fetchOk();
  var err = fetchError();
  if (ok === 0 && err !== '') {
    setTextContent(resultId, 'PASS: fetch correctly rejected: ' + err);
    classAdd(resultId, 'pass');
  } else {
    setTextContent(resultId, 'FAIL: expected rejection');
    classAdd(resultId, 'fail');
  }
}

dongFetch('nonexistent_file_xyz.txt', 'onFetchDone');
`,
    mf: { frames: 2 },
  },
};

const EXEC_MODULES = {
  'test_bold_dynamic.html': {
    name: 't20_bold_dynamic',
    js: `var editorA = getElementById('editorA');
var logId = getElementById('log');

if (editorA !== 0) {
  focusNode(editorA);
  selectAll();
  var ok = execCommand('bold', '');
  setTextContent(logId, 'auto bold(dynamic): ' + (ok ? 'true' : 'false'));
}`,
    exports: [],
  },
  'test_bold_layout_debug.html': {
    name: 't20_bold_layout_debug',
    js: `var editor = getElementById('editor');
var logId = getElementById('log');

if (editor !== 0) {
  focusNode(editor);
  selectAll();
  var ok = execCommand('bold', '');
  setTextContent(logId, 'Bold result: ' + (ok ? 'true' : 'false'));
}`,
    exports: [],
  },
  'test_ce_interactive_sim.html': {
    name: 't20_ce_interactive_sim',
    js: `var editor = getElementById('editor');
var logId = getElementById('log');
var btnBold = getElementById('btnBold');
var btnItalic = getElementById('btnItalic');
var btnUnderline = getElementById('btnUnderline');

function appendLog(msg) {
  var prev = getTextContent(logId);
  setTextContent(logId, prev + msg + '\\n');
}

export function onBold() {
  focusNode(editor);
  selectAll();
  appendLog('bold: ' + (execCommand('bold', '') ? 'true' : 'false'));
}

export function onItalic() {
  focusNode(editor);
  selectAll();
  appendLog('italic: ' + (execCommand('italic', '') ? 'true' : 'false'));
}

export function onUnderline() {
  focusNode(editor);
  selectAll();
  appendLog('underline: ' + (execCommand('underline', '') ? 'true' : 'false'));
}

addEventListener(btnBold, 'click', 'onBold');
addEventListener(btnItalic, 'click', 'onItalic');
addEventListener(btnUnderline, 'click', 'onUnderline');
onBold();
onItalic();
onUnderline();`,
    exports: ['onBold', 'onItalic', 'onUnderline'],
  },
  'test_ce_simulate.html': {
    name: 't20_ce_simulate',
    js: `var editor = getElementById('editor');
var logId = getElementById('log');

function appendLog(msg) {
  var prev = getTextContent(logId);
  setTextContent(logId, prev + msg + '\\n');
}

focusNode(editor);
selectAll();
appendLog('bold: ' + (execCommand('bold', '') ? 'true' : 'false'));
setTextContent(editor, 'Hello World Test');
focusNode(editor);
selectAll();
appendLog('italic: ' + (execCommand('italic', '') ? 'true' : 'false'));
setTextContent(editor, 'Hello World Test');
focusNode(editor);
selectAll();
appendLog('underline: ' + (execCommand('underline', '') ? 'true' : 'false'));`,
    exports: [],
  },
  'test_contenteditable_auto.html': {
    name: 't20_contenteditable_auto',
    js: `var editor = getElementById('editor');
var logId = getElementById('log');

function appendLog(msg) {
  var prev = getTextContent(logId);
  setTextContent(logId, prev + msg + '\\n');
}

focusNode(editor);
appendLog('selectAll: ' + (selectAll() ? 'true' : 'false'));
appendLog('bold: ' + (execCommand('bold', '') ? 'true' : 'false'));
appendLog('italic: ' + (execCommand('italic', '') ? 'true' : 'false'));
selectAll();
appendLog('underline: ' + (execCommand('underline', '') ? 'true' : 'false'));
selectAll();
appendLog('bold toggle: ' + (execCommand('bold', '') ? 'true' : 'false'));`,
    exports: [],
  },
  'test_contenteditable_bold_auto.html': {
    name: 't20_contenteditable_bold_auto',
    js: `var editor = getElementById('editor');
var logId = getElementById('log');

if (editor !== 0) {
  focusNode(editor);
  selectAll();
  var ok = execCommand('bold', '');
  setTextContent(logId, 'auto: selectAll + bold -> ' + (ok ? 'true' : 'false'));
}`,
    exports: [],
  },
  'test_contenteditable_features.html': {
    name: 't20_contenteditable_features',
    js: `var editor = getElementById('editor');
var logId = getElementById('log');

function appendLog(msg) {
  var prev = getTextContent(logId);
  setTextContent(logId, prev + msg + '\\n');
}

focusNode(editor);
selectAll();
appendLog('bold: ' + (execCommand('bold', '') ? 'true' : 'false'));
setTextContent(editor, 'Hello World Test');
selectAll();
appendLog('italic: ' + (execCommand('italic', '') ? 'true' : 'false'));
setTextContent(editor, 'Hello World Test');
selectAll();
appendLog('underline: ' + (execCommand('underline', '') ? 'true' : 'false'));
setTextContent(editor, 'Hello World Test');
selectAll();
appendLog('insertText: ' + (execCommand('insertText', 'Replaced') ? 'true' : 'false'));
setTextContent(editor, 'Hello World Test');
selectAll();
appendLog('delete: ' + (execCommand('delete', '') ? 'true' : 'false'));`,
    exports: [],
  },
};

/** @type {Record<string, { action: string, note?: string }>} */
const DISPOSITION = {
  // T06
  'innerhtml_test.html': { action: 'ready', note: 'layout smoke' },
  'innerhtml_auto_test.html': { action: 'ready', note: 'layout smoke' },
  'test_domparser_api.html': { action: 'ready', note: 'static form layout' },
  'test_formdata_name_serialization.html': { action: 'ready', note: 'form field layout' },
  'test_formdata_api.html': { action: 'dropped', note: 'FormData object API per T20' },

  // T10 — handled via fetch-module
  'test_fetch_json_basic.html': { action: 'fetch-module' },
  'test_fetch_text_basic.html': { action: 'fetch-module' },
  'test_fetch_reject.html': { action: 'fetch-module' },

  // T11
  'test_clipboard_api.html': { action: 'ready', note: 'clipboard UI layout' },
  'test_dialog_modal.html': { action: 'ready', note: 'dialog layout (closed)' },
  'test_dialog_show.html': { action: 'ready', note: 'dialog layout (closed)' },
  'test_cssom_matchmedia.html': { action: 'ready', note: 'matchMedia page layout' },
  'test_cssom_supports.html': { action: 'ready', note: 'CSS.supports page layout' },
  'test_event_clipboard.html': { action: 'ready', note: 'clipboard event UI' },
  'test_porffor_clipboard_cn.html': { action: 'ready', note: 'clipboard CN UI' },

  // T12
  'test_inert_attribute.html': { action: 'ready', note: 'inert section layout' },
  'test_optgroup_basic.html': { action: 'ready', note: 'optgroup visual layout' },

  // T19
  'test_fetch_timer_order.html': { action: 'dropped', note: 'Promise microtask ordering' },

  // T20 — execCommand seven cases (Porffor host imports)
  'test_contenteditable_auto.html': { action: 'exec-module', note: 'execCommand on parse -> porffor module' },
  'test_contenteditable_features.html': { action: 'exec-module', note: 'execCommand matrix -> porffor module' },
  'test_contenteditable_bold_auto.html': { action: 'exec-module', note: 'auto bold -> porffor module' },
  'test_ce_simulate.html': { action: 'exec-module', note: 'execCommand simulate -> porffor module' },
  'test_ce_interactive_sim.html': { action: 'exec-module', note: 'execCommand interactive sim -> porffor module' },
  'test_bold_dynamic.html': { action: 'exec-module', note: 'dynamic bold -> porffor module' },
  'test_bold_layout_debug.html': { action: 'exec-module', note: 'bold layout debug -> porffor module' },

  // T20 — dropped (engine-specific API)
  'test_scene_graph.html': { action: 'dropped', note: 'dong.scene API' },

  // T20 — frame-0 ready
  'tabindex_test.html': { action: 'ready', note: 'tabindex focus order UI' },
  'test_font_weight_mapping.html': { action: 'ready', note: 'static font-weight CSS' },
  'test_form_validation_range.html': { action: 'ready', note: 'range input layout' },
  'test_img_load_error.html': { action: 'ready', note: 'image load/error UI' },
  'test_img_events_simple.html': { action: 'ready', note: 'image events UI' },
  'test_pointer_properties.html': { action: 'ready', note: 'pointer events UI' },
  'test_scene_compiler.html': { action: 'ready', note: 'scene compiler static layout' },
  'queryselector_complex_test.html': { action: 'ready', note: 'selector layout (gray items)' },
  'stylesheets_deleterule_test.html': { action: 'ready', note: 'stylesheet box layout' },
  'stylesheets_insertrule_test.html': { action: 'ready', note: 'stylesheet box layout' },
  'test_event_currenttarget.html': { action: 'ready', note: 'event UI layout' },
  'test_event_features.html': { action: 'ready', note: 'event features UI' },
  'test_event_features_p2.html': { action: 'ready', note: 'event features UI p2' },
  'test_element_click_and_capture.html': { action: 'ready', note: 'capture phase UI (initial)' },
  'test_scroll_autobottom_blocks.html': { action: 'ready', note: 'scroll container layout' },
  'test_scroll_top_clamp_scrollheight.html': { action: 'ready', note: 'scroll clamp layout' },
  'test_select_option_click.html': { action: 'ready', note: 'select dropdown layout' },
  'test_sticky_scroll_top_autoscroll.html': { action: 'ready', note: 'sticky scroll layout' },
};

function replacePorfforTag(html, tag) {
  const stripped = html.replace(
    /<!--\s*porffor:\s*(?:ready|pending|blocked\([^)]+\)|dropped\([^)]+\))\s*-->\s*\n?/gi,
    '',
  );
  const line = `<!-- porffor: ${tag} -->`;
  if (stripped.startsWith('<!DOCTYPE') || stripped.startsWith('<!doctype')) {
    const next = stripped.replace(/<!DOCTYPE html>/i, `<!DOCTYPE html>\n${line}`);
    if (next !== stripped) return next;
    return stripped.replace(/<!DOCTYPE html[^>]*>/i, (m) => `${m}\n${line}`);
  }
  if (stripped.startsWith('<!-- porffor:')) return stripped;
  return `${line}\n${stripped}`;
}

function stripScripts(html) {
  return html.replace(/<script\b[^>]*>[\s\S]*?<\/script>\s*/gi, '');
}

function addModuleAttr(html, moduleName) {
  if (/data-porffor-module=/i.test(html)) return html;
  if (/<html\b/i.test(html)) {
    return html.replace(/<html\b/i, `<html data-porffor-module="${moduleName}"`);
  }
  return html.replace(/<body\b/i, `<body data-porffor-module="${moduleName}"`);
}

const stats = { ready: 0, exec: 0, dropped: 0, t20: 0, fetch: 0, skip: 0 };
const report = [];

function processFile(file) {
  const filePath = path.join(testsDir, file);
  const html = fs.readFileSync(filePath, 'utf8');
  const tag = parsePorfforTagFromHtml(html);
  if (tag.status !== 'blocked') {
    stats.skip++;
    return;
  }

  const disp = DISPOSITION[file];
  if (!disp) {
    stats.skip++;
    report.push({ file, result: 'skip', note: 'no disposition' });
    return;
  }

  if (disp.action === 'fetch-module') {
    const spec = FETCH_MODULES[file];
    const relJs = `examples/porffor/tests/${spec.name}.js`;
    const jsPath = path.join(dongRoot, relJs);
    const mfPath = path.join(porfforTestsDir, `${path.basename(file, '.html')}.mf.json`);

    if (!dryRun) {
      fs.mkdirSync(porfforTestsDir, { recursive: true });
      fs.writeFileSync(jsPath, spec.js);
      fs.writeFileSync(mfPath, `${JSON.stringify(spec.mf, null, 2)}\n`);

      let next = replacePorfforTag(stripScripts(html), 'ready');
      next = addModuleAttr(next, spec.name);
      fs.writeFileSync(filePath, next);

      const manifest = JSON.parse(fs.readFileSync(manifestPath, 'utf8'));
      manifest.scripts = manifest.scripts ?? [];
      const entry = { name: spec.name, path: relJs, exports: ['onFetchDone'] };
      const idx = manifest.scripts.findIndex((s) => s.name === spec.name);
      if (idx >= 0) manifest.scripts[idx] = entry;
      else manifest.scripts.push(entry);
      fs.writeFileSync(manifestPath, `${JSON.stringify(manifest, null, 2)}\n`);
    }

    stats.fetch++;
    report.push({ file, result: 'ready+fetch', note: spec.name });
    console.log(`fetch-module ${file} -> ${spec.name}`);
    return;
  }

  if (disp.action === 'exec-module') {
    const spec = EXEC_MODULES[file];
    if (!spec) {
      stats.skip++;
      report.push({ file, result: 'skip', note: 'missing exec-module spec' });
      return;
    }
    const relJs = `examples/porffor/tests/${spec.name}.js`;
    const jsPath = path.join(dongRoot, relJs);
    if (!dryRun) {
      fs.mkdirSync(porfforTestsDir, { recursive: true });
      fs.writeFileSync(jsPath, spec.js);
      let next = replacePorfforTag(stripScripts(html), 'ready');
      next = addModuleAttr(next, spec.name);
      fs.writeFileSync(filePath, next);

      const manifest = JSON.parse(fs.readFileSync(manifestPath, 'utf8'));
      manifest.scripts = manifest.scripts ?? [];
      const entry = { name: spec.name, path: relJs };
      if (spec.exports.length > 0) entry.exports = spec.exports;
      const idx = manifest.scripts.findIndex((s) => s.name === spec.name);
      if (idx >= 0) manifest.scripts[idx] = entry;
      else manifest.scripts.push(entry);
      fs.writeFileSync(manifestPath, `${JSON.stringify(manifest, null, 2)}\n`);
    }
    stats.exec++;
    report.push({ file, result: 'ready+exec-module', note: spec.name });
    console.log(`exec-module ${file} -> ${spec.name}`);
    return;
  }

  if (disp.action === 'ready') {
    if (!dryRun) {
      fs.writeFileSync(filePath, replacePorfforTag(html, 'ready'));
    }
    stats.ready++;
    report.push({ file, result: 'ready', note: disp.note });
    console.log(`ready ${file}`);
    return;
  }

  if (disp.action === 'blocked-t20') {
    if (!dryRun) {
      fs.writeFileSync(filePath, replacePorfforTag(html, 'blocked(T20)'));
    }
    stats.t20++;
    report.push({ file, result: 'blocked(T20)', note: disp.note });
    console.log(`blocked(T20) ${file}`);
    return;
  }

  if (disp.action === 'dropped') {
    const reason = disp.note ?? 'out of scope';
    if (!dryRun) {
      fs.writeFileSync(filePath, replacePorfforTag(html, `dropped(${reason})`));
    }
    stats.dropped++;
    report.push({ file, result: 'dropped', note: reason });
    console.log(`dropped(${reason}) ${file}`);
  }
}

for (const file of fs.readdirSync(testsDir).filter((f) => f.endsWith('.html'))) {
  processFile(file);
}

const outPath = path.join(
  dongRoot,
  '..',
  'docs',
  'developer',
  'porffor',
  'tasks',
  'T13-remaining-batch-result.md',
);

const md = `# T13 剩余 blocked 批处理结果

> \`node dong/scripts/porffor_batch_migrate_remaining.mjs\` 生成

## 摘要

| 动作 | 数量 |
|------|------|
| ready | ${stats.ready} |
| ready + T20 exec 模块 | ${stats.exec} |
| ready + T10 fetch 模块 | ${stats.fetch} |
| blocked(T20) 保留 | ${stats.t20} |
| dropped | ${stats.dropped} |
| 跳过 | ${stats.skip} |

## 明细

| 文件 | 结果 | 说明 |
|------|------|------|
${report.map((r) => `| ${r.file} | ${r.result} | ${r.note ?? ''} |`).join('\n')}

## T10 fetch 模块

- \`t10_fetch_json\` / \`t10_fetch_text\` / \`t10_fetch_reject\`
- 各测试配套 \`.mf.json\`（\`frames: 2\`）等待 fetch 回调后截图
`;

if (!dryRun) {
  fs.mkdirSync(path.dirname(outPath), { recursive: true });
  fs.writeFileSync(outPath, md);
}

console.log(
  `[porffor_batch_migrate_remaining] ready=${stats.ready} exec=${stats.exec} fetch=${stats.fetch} ` +
    `t20=${stats.t20} dropped=${stats.dropped} skip=${stats.skip}` +
    (dryRun ? ' (dry-run)' : ''),
);
