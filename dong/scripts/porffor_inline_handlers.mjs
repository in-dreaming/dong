#!/usr/bin/env node
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { createRequire } from 'node:module';
import { dongRoot, ensurePorfforDeps, porfforRoot } from './porffor_paths.mjs';
import { lintPorfforSource } from './porffor_lint.mjs';

ensurePorfforDeps();

const require = createRequire(import.meta.url);
const acorn = require(path.join(porfforRoot, 'node_modules/acorn'));

export const EVENT_ATTRS = [
  'onclick',
  'ondblclick',
  'onmousedown',
  'onmouseup',
  'onmousemove',
  'onmouseover',
  'onmouseout',
  'onkeydown',
  'onkeyup',
  'onkeypress',
  'onfocus',
  'onblur',
  'oninput',
  'onchange',
  'onsubmit',
  'onreset',
  'onload',
  'onerror',
];

const JS_BUILTINS = new Set([
  'undefined',
  'null',
  'true',
  'false',
  'NaN',
  'Infinity',
  'String',
  'Number',
  'Boolean',
  'Object',
  'Array',
  'Math',
  'parseInt',
  'parseFloat',
  'isNaN',
  'isFinite',
  'event',
]);

let preludeNames = null;

function loadPreludeNames() {
  if (preludeNames) {
    return preludeNames;
  }
  const preludePath = path.join(dongRoot, 'src/script/porffor/dong_porffor_prelude.js');
  const src = fs.readFileSync(preludePath, 'utf8');
  const names = new Set(JS_BUILTINS);
  const fnRe = /function\s+(\w+)\s*\(/g;
  let m;
  while ((m = fnRe.exec(src)) !== null) {
    names.add(m[1]);
  }
  const varRe = /var\s+(\w+)\s*=/g;
  while ((m = varRe.exec(src)) !== null) {
    names.add(m[1]);
  }
  preludeNames = names;
  return names;
}

function parseAttrValue(raw) {
  const trimmed = raw.trim();
  if (
    (trimmed.startsWith('"') && trimmed.endsWith('"')) ||
    (trimmed.startsWith("'") && trimmed.endsWith("'"))
  ) {
    return trimmed.slice(1, -1);
  }
  return trimmed;
}

function parseAttributes(attrStr) {
  const attrs = new Map();
  const re = /([^\s=/>]+)(?:\s*=\s*("(?:[^"\\]|\\.)*"|'(?:[^'\\]|\\.)*'|[^\s>]+))?/g;
  let m;
  while ((m = re.exec(attrStr)) !== null) {
    const key = m[1].toLowerCase();
    const value = m[2] !== undefined ? parseAttrValue(m[2]) : '';
    attrs.set(key, value);
  }
  return attrs;
}

function eventTypeFromAttr(attr) {
  return attr.slice(2);
}

function exportNameFor(elementId, attr) {
  return `${elementId}__${attr}`;
}

function collectIdentifiers(code) {
  const builtins = loadPreludeNames();
  const assigned = new Set();
  const used = new Set();

  let ast;
  try {
    ast = acorn.parse(code, { ecmaVersion: 2020, sourceType: 'script' });
  } catch (e) {
    throw new Error(`syntax error: ${e.message}`);
  }

  function walk(node, scope) {
    if (!node || typeof node !== 'object') {
      return;
    }
    if (node.type === 'Identifier') {
      if (!scope.has(node.name) && !builtins.has(node.name)) {
        used.add(node.name);
      }
      return;
    }
    if (node.type === 'AssignmentExpression' && node.left?.type === 'Identifier') {
      if (!scope.has(node.left.name) && !builtins.has(node.left.name)) {
        assigned.add(node.left.name);
        used.add(node.left.name);
      }
    }
    if (node.type === 'UpdateExpression' && node.argument?.type === 'Identifier') {
      const name = node.argument.name;
      if (!scope.has(name) && !builtins.has(name)) {
        assigned.add(name);
        used.add(name);
      }
    }
    if (node.type === 'VariableDeclaration') {
      for (const decl of node.declarations) {
        if (decl.id?.type === 'Identifier') {
          scope.add(decl.id.name);
        }
      }
    }
    if (node.type === 'FunctionDeclaration' && node.id?.type === 'Identifier') {
      scope.add(node.id.name);
    }
    for (const key of Object.keys(node)) {
      const child = node[key];
      if (Array.isArray(child)) {
        for (const item of child) {
          if (item && typeof item.type === 'string') {
            walk(item, scope);
          }
        }
      } else if (child && typeof child.type === 'string') {
        walk(child, scope);
      }
    }
  }

  walk(ast, new Set());
  return { assigned, used };
}

function scanHtml(html, sourcePath) {
  if (/<!--\s*porffor:\s*dropped\s*\([^)]*\)\s*-->/i.test(html)) {
    return { skipped: true, reason: 'dropped' };
  }

  if (/<script\b/i.test(html)) {
    console.warn(
      `[porffor_inline_handlers] inline <script> in ${sourcePath} — extract via T14, not T12`,
    );
  }

  let autoId = 0;
  const handlers = [];
  const allElementIds = new Set();
  const tagRe = /<([a-zA-Z][\w:-]*)([^>]*?)(\/?)>/g;
  let cleaned = '';
  let lastIndex = 0;

  function lineAt(index) {
    return html.slice(0, index).split('\n').length;
  }

  let m;
  while ((m = tagRe.exec(html)) !== null) {
    const [full, tagName, attrStr, selfClose] = m;
    const tagLower = tagName.toLowerCase();
    if (tagLower === 'script' || tagLower === 'style' || tagLower.startsWith('!')) {
      continue;
    }

    const attrs = parseAttributes(attrStr);
    if (attrs.has('id') && attrs.get('id')) {
      allElementIds.add(attrs.get('id'));
    }

    const foundEvents = EVENT_ATTRS.filter((a) => attrs.has(a) && attrs.get(a) !== '');
    if (foundEvents.length === 0) {
      continue;
    }

    let elementId = attrs.get('id') ?? '';
    let newAttrStr = attrStr;
    if (!elementId) {
      elementId = `__porf_auto_${autoId++}`;
      newAttrStr = ` id="${elementId}"${attrStr}`;
    }
    allElementIds.add(elementId);

    const newAttrs = parseAttributes(newAttrStr);
    const kept = [];
    for (const [k, v] of newAttrs.entries()) {
      if (!EVENT_ATTRS.includes(k)) {
        kept.push(v === '' ? k : `${k}="${v.replace(/"/g, '&quot;')}"`);
      }
    }
    const rebuilt =
      kept.length > 0 ? ` ${kept.join(' ')}` : '';
    const replacement = `<${tagName}${rebuilt}${selfClose}>`;

    cleaned += html.slice(lastIndex, m.index);
    cleaned += replacement;
    lastIndex = m.index + full.length;

    for (const attr of foundEvents) {
      const body = attrs.get(attr);
      const handlerLine = lineAt(m.index);
      handlers.push({
        elementId,
        attr,
        eventType: eventTypeFromAttr(attr),
        exportName: exportNameFor(elementId, attr),
        body,
        sourcePath,
        line: handlerLine,
        tagName: tagLower,
      });
    }
  }

  cleaned += html.slice(lastIndex);
  return { skipped: false, handlers, cleanedHtml: cleaned, allElementIds: [...allElementIds] };
}

function nodeVarForElementId(elementId) {
  return `${elementId}Id`;
}

function elementIdForNodeVar(varName, allElementIds) {
  if (!varName.endsWith('Id') || varName.length <= 2) {
    return null;
  }
  const candidate = varName.slice(0, -2);
  return allElementIds.includes(candidate) ? candidate : null;
}

function generateModule(handlers, allElementIds) {
  const globals = new Set();
  const handlerElements = new Set();

  for (const h of handlers) {
    handlerElements.add(h.elementId);
    const { assigned, used } = collectIdentifiers(h.body);
    for (const name of assigned) {
      globals.add(name);
    }
    for (const name of used) {
      globals.add(name);
    }
  }

  const nodeVars = new Set();
  for (const elementId of handlerElements) {
    nodeVars.add(nodeVarForElementId(elementId));
  }
  for (const g of globals) {
    const mapped = elementIdForNodeVar(g, allElementIds);
    if (mapped) {
      nodeVars.add(g);
    }
  }

  const lines = [];
  for (const g of [...globals].sort()) {
    lines.push(`var ${g} = 0;`);
  }
  for (const nv of [...nodeVars].sort()) {
    if (!globals.has(nv)) {
      lines.push(`var ${nv} = 0;`);
    }
  }
  if (lines.length > 0) {
    lines.push('');
  }

  for (const h of handlers) {
    lines.push(`export function ${h.exportName}() {`);
    lines.push(`  ${h.body}`);
    lines.push('}');
    lines.push('');
  }

  const assignedElements = new Set();
  for (const nv of [...nodeVars].sort()) {
    const elementId = elementIdForNodeVar(nv, allElementIds) ?? nv.slice(0, -2);
    if (!allElementIds.includes(elementId)) {
      continue;
    }
    assignedElements.add(elementId);
    lines.push(`${nv} = getElementById('${elementId}');`);
  }
  for (const elementId of [...handlerElements].sort()) {
    if (!assignedElements.has(elementId)) {
      const nv = nodeVarForElementId(elementId);
      lines.push(`${nv} = getElementById('${elementId}');`);
    }
  }
  for (const h of handlers) {
    const varName = nodeVarForElementId(h.elementId);
    lines.push(`addEventListener(${varName}, '${h.eventType}', '${h.exportName}');`);
  }

  return {
    source: `${lines.join('\n')}\n`,
    exports: handlers.map((h) => h.exportName),
    globals: [...globals],
  };
}

async function validateHandlerExport(prelude, exportName, body) {
  lintPorfforSource(body, `inline handler ${exportName}`);
  const probe = `${prelude}\nvar __probe = 0;\nexport function ${exportName}() {\n  ${body}\n}\n`;
  const argvBackup = process.argv.slice();
  const outFile = path.join(dongRoot, 'generated', 'porffor', '_inline_probe.c');
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
    '--2c-prefix=__t12_probe_',
    `-o=${outFile}`,
  ];

  const _realExit = process.exit.bind(process);
  process.exit = (code) => {
    if (code && code !== 0) {
      _realExit(code);
    }
  };

  const { createImport } = await import(
    new URL('../third_party/porffor/compiler/builtins.js', import.meta.url).href
  );
  const { default: compile } = await import(
    new URL('../third_party/porffor/compiler/index.js', import.meta.url).href
  );

  const imp = (name, params, returns = 0) => createImport(name, params, returns, null, null);
  imp('dong_dom_getElementById', 1, 1);
  imp('dong_dom_set_textContent', 2, 0);
  imp('dong_dom_get_textContent', 1, 0);
  imp('dong_stage_0', 1, 0);
  imp('dong_stage_1', 1, 0);
  imp('dong_stage_2', 1, 0);
  imp('dong_commit_set_textContent', 0, 0);
  imp('dong_commit_addEventListener', 0, 0);
  imp('dong_print', 1, 0);
  imp('dong_str_len', 0, 1);
  imp('dong_str_read', 2, 1);
  imp('dong_str_byte_at', 1, 1);
  imp('dong_str_pull', 0, 1);

  globalThis.file = '__probe__';
  try {
    const result = compile(probe);
    if (!result?.c) {
      throw new Error('Porffor compile produced no C');
    }
  } finally {
    process.argv = argvBackup;
    process.exit = _realExit;
    try {
      fs.unlinkSync(outFile);
    } catch {
      /* probe file may not exist */
    }
  }
}

export async function extractInlineEntry(entry) {
  const htmlPath = path.join(dongRoot, entry.html);
  if (!fs.existsSync(htmlPath)) {
    throw new Error(`inline HTML not found: ${entry.html}`);
  }
  const html = fs.readFileSync(htmlPath, 'utf8');
  const scan = scanHtml(html, entry.html);
  if (scan.skipped) {
    return { skipped: true, entry: null };
  }
  if (scan.handlers.length === 0) {
    return { skipped: false, entry: null, cleanedHtml: scan.cleanedHtml };
  }

  const preludePath = path.join(dongRoot, 'src/script/porffor/dong_porffor_prelude.js');
  const prelude = fs.readFileSync(preludePath, 'utf8');

  for (const h of scan.handlers) {
    try {
      await validateHandlerExport(prelude, h.exportName, h.body);
    } catch (e) {
      throw new Error(
        `${entry.html}:${h.line}: handler ${h.exportName} failed Porffor compile — ${e.message}. ` +
          'Use module globals / dong_state_* (T15) or move logic to C++.',
      );
    }
  }

  const generated = generateModule(scan.handlers, scan.allElementIds);
  const outJs = entry.out_js ?? `generated/porffor/${entry.name}.js`;
  const outHtml = entry.out_html ?? `generated/porffor/${entry.name}.html`;
  const jsAbs = path.join(dongRoot, outJs);
  const htmlAbs = path.join(dongRoot, outHtml);
  fs.mkdirSync(path.dirname(jsAbs), { recursive: true });
  fs.mkdirSync(path.dirname(htmlAbs), { recursive: true });
  fs.writeFileSync(jsAbs, generated.source);
  fs.writeFileSync(htmlAbs, scan.cleanedHtml);

  return {
    skipped: false,
    entry: {
      name: entry.name,
      path: outJs,
      html: outHtml,
      exports: generated.exports,
      source_html: entry.html,
    },
    cleanedHtml: scan.cleanedHtml,
  };
}

export async function extractInlineHandlers(manifest) {
  const inlineHtml = manifest.inline_html ?? [];
  const generatedScripts = [];

  for (const item of inlineHtml) {
    const result = await extractInlineEntry(item);
    if (result.entry) {
      generatedScripts.push(result.entry);
      console.log(
        `[porffor_inline_handlers] ${item.name}: ${result.entry.exports.length} handler(s) -> ${result.entry.path}`,
      );
    } else if (result.skipped) {
      console.log(`[porffor_inline_handlers] ${item.name}: skipped (dropped)`);
    }
  }

  return generatedScripts;
}

async function main() {
  const manifestPath = path.join(dongRoot, 'scripts', 'porffor_manifest.json');
  const manifest = JSON.parse(fs.readFileSync(manifestPath, 'utf8'));
  const generated = await extractInlineHandlers(manifest);
  const outPath = path.join(dongRoot, 'generated', 'porffor', 'inline_scripts.json');
  fs.mkdirSync(path.dirname(outPath), { recursive: true });
  fs.writeFileSync(outPath, `${JSON.stringify(generated, null, 2)}\n`);
  console.log(`[porffor_inline_handlers] wrote ${generated.length} script(s) -> ${outPath}`);
}

const __filename = fileURLToPath(import.meta.url);
if (process.argv[1] && path.resolve(process.argv[1]) === __filename) {
  main().catch((e) => {
    console.error(e.message || e);
    process.exit(1);
  });
}
