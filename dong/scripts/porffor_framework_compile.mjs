#!/usr/bin/env node
/**
 * T22 — .porf template → HTML + Porffor JS module.
 * See docs/developer/porffor/framework-spec.md
 */
import fs from 'node:fs';
import path from 'node:path';
import { lintFrameworkSource } from './porffor_framework_lint.mjs';

const BIND_RE = /\{([a-zA-Z_][a-zA-Z0-9_]*)\}/g;
const STYLE_BIND_RE = /data-porf-style="([^:"]+):([^"]+)"/g;
const ON_RE = /data-porf-on="([^:"]+):([^"]+)"/g;
const IF_RE = /data-porf-if="([^"]+)"/g;
const WIDTH_PCT_RE = /data-porf-width-pct="([^"]+)"/g;
const EACH_START = /<!--\s*porf-each\s+(\w+)\s+as\s+(\w+)(?:\s+key\s+(\w+))?\s*-->/g;
const EACH_END = /<!--\s*\/porf-each\s*-->/g;

function parseFrontmatter(raw) {
  const m = raw.match(/^---\r?\n([\s\S]*?)\r?\n---\r?\n/);
  if (!m) {
    return { meta: {}, body: raw };
  }
  const meta = {};
  for (const line of m[1].split('\n')) {
    const kv = line.match(/^(\w+):\s*(.+)$/);
    if (!kv) continue;
    const [, k, v] = kv;
    if (k === 'exports') {
      meta.exports = v.split(',').map((s) => s.trim()).filter(Boolean);
    } else {
      meta[k] = v.trim();
    }
  }
  return { meta, body: raw.slice(m[0].length) };
}

function extractSections(body) {
  const tplMatch = body.match(/<template>([\s\S]*?)<\/template>/i);
  const scriptMatch = body.match(/<script>([\s\S]*?)<\/script>/i);
  if (!tplMatch || !scriptMatch) {
    throw new Error('`.porf` requires <template> and <script> sections');
  }
  return { template: tplMatch[1].trim(), script: scriptMatch[1].trim() };
}

function stripFrameworkAttrs(html) {
  return html
    .replace(/\s*data-porf-on="[^"]*"/g, '')
    .replace(/\s*data-porf-bind="[^"]*"/g, '')
    .replace(/\s*data-porf-style="[^"]*"/g, '')
    .replace(/\s*data-porf-if="[^"]*"/g, '')
    .replace(/\s*data-porf-width-pct="[^"]*"/g, '')
    .replace(BIND_RE, '')
    .replace(EACH_START, '')
    .replace(EACH_END, '');
}

function collectIds(html) {
  const ids = [];
  const re = /\bid="([^"]+)"/g;
  let m;
  while ((m = re.exec(html)) !== null) {
    ids.push(m[1]);
  }
  return [...new Set(ids)];
}

function idToVar(id) {
  return id.replace(/[^a-zA-Z0-9_]/g, '_') + 'Id';
}

function collectBindings(template) {
  const text = [];
  let m;
  const re = new RegExp(BIND_RE.source, 'g');
  while ((m = re.exec(template)) !== null) {
    text.push(m[1]);
  }
  return [...new Set(text)];
}

function collectStyleBindings(template) {
  const out = [];
  const re = new RegExp(STYLE_BIND_RE.source, 'g');
  let m;
  while ((m = re.exec(template)) !== null) {
    out.push({ prop: m[1], varName: m[2], elementId: findIdBeforeIndex(template, m.index) });
  }
  return out;
}

function collectEvents(template) {
  const out = [];
  const re = new RegExp(ON_RE.source, 'g');
  let m;
  while ((m = re.exec(template)) !== null) {
    out.push({
      event: m[1],
      handler: m[2],
      elementId: findIdBeforeIndex(template, m.index),
    });
  }
  return out;
}

function collectIfBlocks(template) {
  const out = [];
  const re = new RegExp(IF_RE.source, 'g');
  let m;
  while ((m = re.exec(template)) !== null) {
    out.push({ expr: m[1], elementId: findIdBeforeIndex(template, m.index) });
  }
  return out;
}

function collectWidthPct(template) {
  const out = [];
  const re = new RegExp(WIDTH_PCT_RE.source, 'g');
  let m;
  while ((m = re.exec(template)) !== null) {
    out.push({ expr: m[1], elementId: findIdBeforeIndex(template, m.index) });
  }
  return out;
}

/** Find nearest id="..." on same tag before attr index */
function findIdBeforeIndex(html, index) {
  const slice = html.slice(0, index);
  const tagStart = slice.lastIndexOf('<');
  const tagEnd = html.indexOf('>', index);
  const tag = html.slice(tagStart, tagEnd + 1);
  const idm = tag.match(/\bid="([^"]+)"/);
  return idm ? idm[1] : null;
}

function genPatchText(varName, elementId) {
  const fn = `porfPatch_${varName}`;
  const nodeVar = idToVar(elementId);
  return {
    fn,
    code: `function ${fn}() {
  setTextContent(${nodeVar}, String(${varName}));
}
`,
  };
}

function genPatchStyle({ prop, varName, elementId }) {
  const fn = `porfPatchStyle_${varName}_${prop.replace(/-/g, '_')}`;
  const nodeVar = idToVar(elementId);
  return {
    fn,
    code: `function ${fn}() {
  setStyle(${nodeVar}, '${prop}', String(${varName}));
}
`,
  };
}

function genPatchIf({ expr, elementId }) {
  const fn = `porfPatchIf_${elementId.replace(/[^a-zA-Z0-9_]/g, '_')}`;
  const nodeVar = idToVar(elementId);
  return {
    fn,
    code: `function ${fn}() {
  if (${expr}) {
    removeAttribute(${nodeVar}, 'hidden');
  } else {
    setAttribute(${nodeVar}, 'hidden', '1');
  }
}
`,
  };
}

function genPatchWidthPct({ expr, elementId }) {
  const fn = `porfPatchWidth_${elementId.replace(/[^a-zA-Z0-9_]/g, '_')}`;
  const nodeVar = idToVar(elementId);
  return {
    fn,
    code: `function ${fn}() {
  var pct = ${expr};
  if (pct < 0) { pct = 0; }
  if (pct > 100) { pct = 100; }
  setStyle(${nodeVar}, 'width', String(pct) + '%');
}
`,
  };
}

function genInit(ids, events, patches) {
  const lines = [];
  for (const id of ids) {
    lines.push(`  ${idToVar(id)} = getElementById('${id}');`);
  }
  for (const ev of events) {
    if (!ev.elementId) continue;
    lines.push(
      `  addEventListener(${idToVar(ev.elementId)}, '${ev.event}', '${ev.handler}');`,
    );
  }
  for (const p of patches) {
    lines.push(`  ${p.fn}();`);
  }
  return `function porfInit() {
${lines.join('\n')}
  dongLog('${'porfInit'}');
}
`;
}

/**
 * @param {string} source
 * @param {{ fileLabel?: string }} [opts]
 */
export function compilePorfSource(source, opts = {}) {
  const label = opts.fileLabel ?? 'source.porf';
  lintFrameworkSource(source, label);

  const { meta, body } = parseFrontmatter(source);
  const { template, script } = extractSections(body);

  const html = stripFrameworkAttrs(template);
  const ids = collectIds(template);
  const textBindings = collectBindings(template);
  const styleBindings = collectStyleBindings(template);
  const events = collectEvents(template);
  const ifBlocks = collectIfBlocks(template);
  const widthPct = collectWidthPct(template);

  const patches = [];
  const patchFns = [];

  for (const v of textBindings) {
    const el = findElementForBinding(template, v);
    if (!el) continue;
    const p = genPatchText(v, el);
    patches.push(p);
    patchFns.push(p.code);
  }
  for (const sb of styleBindings) {
    if (!sb.elementId) continue;
    const p = genPatchStyle(sb);
    patches.push(p);
    patchFns.push(p.code);
  }
  for (const ib of ifBlocks) {
    if (!ib.elementId) continue;
    const p = genPatchIf(ib);
    patches.push(p);
    patchFns.push(p.code);
  }
  for (const wp of widthPct) {
    if (!wp.elementId) continue;
    const p = genPatchWidthPct(wp);
    patches.push(p);
    patchFns.push(p.code);
  }

  const initFn = genInit(ids, events, patches);

  const idDecls = ids.map((id) => `var ${idToVar(id)} = 0;`).join('\n');

  const exportsFromScript = [...script.matchAll(/export function (\w+)/g)].map((m) => m[1]);
  const exports = meta.exports?.length ? meta.exports : exportsFromScript;

  const js = `${idDecls}

${patchFns.join('\n')}
${initFn}
${script}

porfInit();
`;

  lintFrameworkSource(js, `${label} (generated)`);

  return {
    module: meta.module ?? 'porf_module',
    html,
    js,
    exports,
  };
}

function findElementForBinding(template, varName) {
  const re = new RegExp(
    `id="([^"]+)"[^>]*data-porf-bind="${varName}"|data-porf-bind="${varName}"[^>]*id="([^"]+)"`,
  );
  const m = template.match(re);
  if (!m) {
    const idx = template.indexOf(`{${varName}}`);
    if (idx >= 0) return findIdBeforeIndex(template, idx);
    return null;
  }
  return m[1] || m[2];
}

/**
 * @param {string} porfPath
 * @param {{ outJs?: string, outHtml?: string }} [opts]
 */
export function compilePorfFile(porfPath, opts = {}) {
  const source = fs.readFileSync(porfPath, 'utf8');
  const result = compilePorfSource(source, { fileLabel: porfPath });
  const base = path.basename(porfPath, '.porf');
  const outJs = opts.outJs ?? path.join(path.dirname(porfPath), `${base}.js`);
  const outHtml = opts.outHtml ?? path.join(path.dirname(porfPath), `${base}.html`);
  fs.mkdirSync(path.dirname(outJs), { recursive: true });
  fs.mkdirSync(path.dirname(outHtml), { recursive: true });
  fs.writeFileSync(outJs, result.js);
  fs.writeFileSync(outHtml, wrapHtmlDocument(result.html, result.module));
  return { ...result, outJs, outHtml };
}

export function wrapHtmlDocument(body, moduleName) {
  return `<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body { width: 100%; height: 100%; }
  </style>
</head>
<body data-porffor-module="${moduleName}">
${body}
</body>
</html>
`;
}

if (import.meta.url === `file://${process.argv[1]?.replace(/\\/g, '/')}` ||
    process.argv[1]?.endsWith('porffor_framework_compile.mjs')) {
  const input = process.argv[2];
  if (!input) {
    console.error('Usage: node porffor_framework_compile.mjs <file.porf> [out.js] [out.html]');
    process.exit(1);
  }
  const out = compilePorfFile(input, {
    outJs: process.argv[3],
    outHtml: process.argv[4],
  });
  console.log(`[porffor_framework_compile] ${input} -> ${out.outJs}, ${out.outHtml}`);
}
