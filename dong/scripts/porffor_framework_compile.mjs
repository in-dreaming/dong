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
const EACH_START_RE =
  /<!--\s*porf-each\s+(\w+)\s+as\s+(\w+)(?:\s+key\s+(\w+))?(?:\s+container=([\w-]+))?(?:\s+filter=([^\s>]+))?(?:\s+row-fn=(\w+))?\s*-->/;
const EACH_END_RE = /<!--\s*\/porf-each\s*-->/;
const PARTIAL_DEF_RE =
  /<!--\s*porf-partial\s+name="([^"]+)"\s*-->([\s\S]*?)<!--\s*\/porf-partial\s*-->/g;
const PARTIAL_USE_RE =
  /<!--\s*porf-use\s+partial="([^"]+)"\s+props="([^"]*)"\s*-->/g;
const MAX_EACH_ITEMS = 32;

function parseFrontmatter(raw) {
  const m = raw.match(/^---\r?\n([\s\S]*?)\r?\n---\r?\n/);
  if (!m) {
    return { meta: {}, body: raw };
  }
  const meta = {};
  for (const rawLine of m[1].split('\n')) {
    const line = rawLine.trim();
    if (!line) continue;
    const kv = line.match(/^([\w-]+):\s*(.+)$/);
    if (!kv) continue;
    const k = kv[1];
    const v = kv[2].trim();
    if (k === 'exports') {
      meta.exports = v.split(',').map((s) => s.trim()).filter(Boolean);
    } else {
      meta[k] = v;
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

/** @param {string} propsStr e.g. `label=HP fillId=hp-fill` */
function parsePartialProps(propsStr) {
  const props = {};
  if (!propsStr?.trim()) return props;
  for (const token of propsStr.trim().split(/\s+/)) {
    const eq = token.indexOf('=');
    if (eq <= 0) continue;
    const key = token.slice(0, eq);
    let val = token.slice(eq + 1);
    if (
      (val.startsWith('"') && val.endsWith('"')) ||
      (val.startsWith("'") && val.endsWith("'"))
    ) {
      val = val.slice(1, -1);
    }
    props[key] = val;
  }
  return props;
}

/**
 * Extract partial definitions, expand porf-use sites, strip defs from template.
 * Props use `{{name}}` placeholders in partial bodies.
 * @param {string} template
 */
function expandPartials(template) {
  /** @type {Map<string, string>} */
  const partials = new Map();
  let withoutDefs = template.replace(PARTIAL_DEF_RE, (_m, name, body) => {
    partials.set(name, body.trim());
    return '';
  });

  withoutDefs = withoutDefs.replace(PARTIAL_USE_RE, (_m, name, propsStr) => {
    const body = partials.get(name);
    if (!body) {
      throw new Error(`porf-use: unknown partial "${name}"`);
    }
    const props = parsePartialProps(propsStr);
    let expanded = body;
    for (const [key, val] of Object.entries(props)) {
      expanded = expanded.replaceAll(`{{${key}}}`, val);
    }
    if (/\{\{[a-zA-Z_][a-zA-Z0-9_]*\}\}/.test(expanded)) {
      throw new Error(`porf-use partial="${name}": unresolved {{prop}} placeholder`);
    }
    return expanded;
  });

  return withoutDefs;
}

function stripFrameworkAttrs(html) {
  return html
    .replace(/\s*data-porf-on="[^"]*"/g, '')
    .replace(/\s*data-porf-bind="[^"]*"/g, '')
    .replace(/\s*data-porf-style="[^"]*"/g, '')
    .replace(/\s*data-porf-if="[^"]*"/g, '')
    .replace(/\s*data-porf-width-pct="[^"]*"/g, '')
    .replace(BIND_RE, '')
    .replace(/<!--\s*porf-each[\s\S]*?-->/g, '')
    .replace(/<!--\s*\/porf-each\s*-->/g, '');
}

/**
 * @param {string} template
 * @returns {Array<{
 *   listVar: string,
 *   itemAlias: string,
 *   keyName: string,
 *   containerId: string,
 *   itemTemplate: string,
 *   fields: string[],
 *   filterExpr: string|null,
 *   rowFn: string|null,
 *   start: number,
 *   end: number,
 * }>}
 */
function collectEachBlocks(template) {
  const blocks = [];
  let searchFrom = 0;
  while (searchFrom < template.length) {
    const slice = template.slice(searchFrom);
    const startM = slice.match(EACH_START_RE);
    if (!startM) break;
    const start = searchFrom + (startM.index ?? 0);
    const afterStart = start + startM[0].length;
    const endM = template.slice(afterStart).match(EACH_END_RE);
    if (!endM) {
      throw new Error('unclosed <!-- porf-each --> block');
    }
    const end = afterStart + (endM.index ?? 0) + endM[0].length;
    const itemAlias = startM[2];
    const keyName = startM[3] ?? 'id';
    const containerId =
      startM[4] ?? findContainerBefore(template, start) ?? null;
    if (!containerId) {
      throw new Error(
        `porf-each (${startM[1]}): missing container — use container=id on comment or empty <div id="..."></div> before block`,
      );
    }
    const itemTemplate = template.slice(afterStart, afterStart + (endM.index ?? 0)).trim();
    const fields = collectEachFields(itemTemplate, itemAlias, keyName);
    blocks.push({
      listVar: startM[1],
      itemAlias,
      keyName,
      containerId,
      itemTemplate,
      fields,
      filterExpr: startM[5] ?? null,
      rowFn: startM[6] ?? null,
      start,
      end,
    });
    searchFrom = end;
  }
  return blocks;
}

function findContainerBefore(template, startIdx) {
  const before = template.slice(0, startIdx);
  const emptyTag = before.match(
    /<([a-z][a-z0-9]*)\s+[^>]*\bid="([^"]+)"[^>]*>\s*<\/\1>\s*$/i,
  );
  if (emptyTag) return emptyTag[2];
  const allIds = [...before.matchAll(/\bid="([^"]+)"/g)];
  return allIds.length ? allIds[allIds.length - 1][1] : null;
}

function collectEachFields(itemTemplate, itemAlias, keyName) {
  const fields = new Set();
  const re = new RegExp(`\\{${itemAlias}([A-Za-z][A-Za-z0-9_]*)\\}`, 'g');
  let m;
  while ((m = re.exec(itemTemplate)) !== null) {
    fields.add(m[1]);
  }
  const keyField = keyName.charAt(0).toUpperCase() + keyName.slice(1);
  if (keyName === 'id') {
    fields.add('Id');
  } else {
    fields.add(keyField);
  }
  return [...fields];
}

function removeEachRegions(template, blocks) {
  let out = template;
  for (let i = blocks.length - 1; i >= 0; i--) {
    const b = blocks[i];
    out = out.slice(0, b.start) + out.slice(b.end);
  }
  return out;
}

function genAtAccessor(itemAlias, fieldSuffix) {
  const fn = `${itemAlias}${fieldSuffix}At`;
  const lines = [];
  for (let i = 0; i < MAX_EACH_ITEMS; i++) {
    lines.push(`  if (i === ${i}) return ${itemAlias}${fieldSuffix}${i};`);
  }
  lines.push(`  return ${itemAlias}${fieldSuffix}${MAX_EACH_ITEMS - 1};`);
  return `function ${fn}(i) {
${lines.join('\n')}
}
`;
}

function genSetAtAccessor(itemAlias, fieldSuffix) {
  const fn = `set${itemAlias.charAt(0).toUpperCase()}${itemAlias.slice(1)}${fieldSuffix}At`;
  const lines = [];
  for (let i = 0; i < MAX_EACH_ITEMS; i++) {
    lines.push(`  if (i === ${i}) { ${itemAlias}${fieldSuffix}${i} = v; return; }`);
  }
  lines.push(`  ${itemAlias}${fieldSuffix}${MAX_EACH_ITEMS - 1} = v;`);
  return `function ${fn}(i, v) {
${lines.join('\n')}
}
`;
}

function genSetSlotAccessor(itemAlias, fields) {
  const fn = `set${itemAlias.charAt(0).toUpperCase()}${itemAlias.slice(1)}Slot`;
  const lines = [];
  for (let i = 0; i < MAX_EACH_ITEMS; i++) {
    const assigns = fields.map((f) => `${itemAlias}${f}${i} = ${f.toLowerCase() === f ? f : f}`).join('; ');
    const params = fields.map((f) => {
      const p = f.charAt(0).toLowerCase() + f.slice(1);
      return p === 'Id' ? 'id' : p === 'Text' ? 'text' : p === 'Done' ? 'done' : p;
    });
    const assignLine = fields
      .map((f) => {
        const p = f === 'Id' ? 'id' : f === 'Text' ? 'text' : f === 'Done' ? 'done' : f;
        return `${itemAlias}${f}${i} = ${p}`;
      })
      .join('; ');
    lines.push(`  if (i === ${i}) { ${assignLine}; return; }`);
  }
  const last = MAX_EACH_ITEMS - 1;
  const lastAssign = fields
    .map((f) => {
      const p = f === 'Id' ? 'id' : f === 'Text' ? 'text' : f === 'Done' ? 'done' : f;
      return `${itemAlias}${f}${last} = ${p}`;
    })
    .join('; ');
  lines.push(`  ${lastAssign};`);
  const paramList = fields
    .map((f) => (f === 'Id' ? 'id' : f === 'Text' ? 'text' : f === 'Done' ? 'done' : f))
    .join(', ');
  return `function ${fn}(i, ${paramList}) {
${lines.join('\n')}
}
`;
}

function templateToHtmlExpr(itemTpl, itemAlias) {
  const re = new RegExp(`\\{${itemAlias}([A-Za-z][A-Za-z0-9_]*)\\}`, 'g');
  let code = '';
  let last = 0;
  let m;
  while ((m = re.exec(itemTpl)) !== null) {
    const staticPart = itemTpl
      .slice(last, m.index)
      .replace(/\\/g, '\\\\')
      .replace(/'/g, "\\'");
    code += `'${staticPart}' + String(${itemAlias}${m[1]}At(i)) + `;
    last = m.index + m[0].length;
  }
  const tail = itemTpl.slice(last).replace(/\\/g, '\\\\').replace(/'/g, "\\'");
  code += `'${tail}'`;
  return code;
}

function genEachRebuild(block) {
  const { listVar, itemAlias, containerId, filterExpr, rowFn, itemTemplate } = block;
  const countVar = `${itemAlias}Count`;
  const fn = `porfRebuild_${listVar}`;
  const containerVar = idToVar(containerId);
  const filterGuard = filterExpr ? `if (${filterExpr}) {` : '';
  const filterClose = filterExpr ? '}' : '';
  let rowExpr;
  if (rowFn) {
    rowExpr = `${rowFn}(i)`;
  } else {
    const cleanTpl = stripFrameworkAttrs(itemTemplate);
    rowExpr = templateToHtmlExpr(cleanTpl, itemAlias);
  }
  return {
    fn,
    code: `function ${fn}() {
  var html = '<div>';
  var i = 0;
  while (i < ${countVar}) {
    if (i >= ${MAX_EACH_ITEMS}) {
      dongLog('${fn}: MAX_ITEMS ${MAX_EACH_ITEMS} truncated');
      break;
    }
    ${filterGuard}
    html = html + ${rowExpr};
    ${filterClose}
    i = i + 1;
  }
  html = html + '</div>';
  setInnerHTML(${containerVar}, html);
}
`,
  };
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
  const re = /data-porf-on="([^"]+)"/g;
  let m;
  while ((m = re.exec(template)) !== null) {
    const parts = m[1].split(',');
    for (const part of parts) {
      const kv = part.trim().match(/^([^:]+):(.+)$/);
      if (!kv) continue;
      out.push({
        event: kv[1].trim(),
        handler: kv[2].trim(),
        elementId: findIdBeforeIndex(template, m.index),
      });
    }
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

function genInit(ids, events, patches, afterInitCalls = []) {
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
  for (const call of afterInitCalls) {
    lines.push(`  ${call}();`);
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
  const { template: rawTemplate, script } = extractSections(body);
  const template = expandPartials(rawTemplate);

  const eachBlocks = collectEachBlocks(template);
  const html = stripFrameworkAttrs(removeEachRegions(template, eachBlocks));
  const ids = collectIds(html);
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

  const eachFns = [];
  const eachFieldAccessors = new Set();
  const eachSlotFields = new Map();
  for (const block of eachBlocks) {
    for (const field of block.fields) {
      const key = `${block.itemAlias}:${field}`;
      if (!eachFieldAccessors.has(key)) {
        eachFieldAccessors.add(key);
        eachFns.push(genAtAccessor(block.itemAlias, field));
        if (field !== 'Id') {
          eachFns.push(genSetAtAccessor(block.itemAlias, field));
        }
      }
      if (!eachSlotFields.has(block.itemAlias)) {
        eachSlotFields.set(block.itemAlias, new Set());
      }
      eachSlotFields.get(block.itemAlias).add(field);
    }
    const rb = genEachRebuild(block);
    eachFns.push(rb.code);
    patches.push(rb);
  }
  for (const [alias, fieldSet] of eachSlotFields) {
    const fields = [...fieldSet];
    if (fields.length >= 2) {
      eachFns.push(genSetSlotAccessor(alias, fields));
    }
  }

  const afterInitCalls = /\bfunction\s+porfRefresh\s*\(/.test(script) ? ['porfRefresh'] : [];
  const initFn = genInit(ids, events, patches, afterInitCalls);

  const idDecls = ids.map((id) => `var ${idToVar(id)} = 0;`).join('\n');

  const exportsFromScript = [...script.matchAll(/export function (\w+)/g)].map((m) => m[1]);
  const exports = meta.exports?.length ? meta.exports : exportsFromScript;

  const js = `${idDecls}

${eachFns.join('\n')}
${patchFns.join('\n')}
${initFn}
${script}

porfInit();
`;

  lintFrameworkSource(js, `${label} (generated)`);

  return {
    module: meta['module'] ?? 'porf_module',
    html,
    js,
    exports,
    bodyStyle: meta['body-style'] ?? meta.bodyStyle,
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
  fs.writeFileSync(
    outHtml,
    wrapHtmlDocument(result.html, result.module, {
      bodyStyle: result.bodyStyle ?? undefined,
    }),
  );
  return { ...result, outJs, outHtml };
}

export function wrapHtmlDocument(body, moduleName, opts = {}) {
  const bodyStyle = opts.bodyStyle ?? 'width: 100%; height: 100%;';
  return `<!-- porffor: ready -->
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body { ${bodyStyle} }
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
