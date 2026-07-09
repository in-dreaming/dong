#!/usr/bin/env node
/**
 * Build-time Porffor compiler for Dong manifest scripts.
 * Generates generated/porffor/*.c + registry.{h,c}
 */
import fs from 'node:fs';
import path from 'node:path';
import { dongRoot, ensurePorfforDeps, porfforCompilerUrls } from './porffor_paths.mjs';
import { lintPorfforSource } from './porffor_lint.mjs';
import { extractInlineHandlers } from './porffor_inline_handlers.mjs';
import { compilePorfFile } from './porffor_framework_compile.mjs';

ensurePorfforDeps();

const outDir = path.join(dongRoot, 'generated', 'porffor');
const manifestPath = path.join(dongRoot, 'scripts', 'porffor_manifest.json');

const manifest = JSON.parse(fs.readFileSync(manifestPath, 'utf8'));

for (const fw of manifest.framework ?? []) {
  const porfPath = path.join(dongRoot, fw.porf);
  const outJs = path.join(dongRoot, fw.path ?? `generated/porffor/${fw.name}.js`);
  compilePorfFile(porfPath, { outJs });
  console.log(`[porffor_compile] framework ${fw.name} <- ${fw.porf}`);
}

const inlineScripts = await extractInlineHandlers(manifest);
const inlineNames = new Set(inlineScripts.map((s) => s.name));
const scriptEntries = [
  ...manifest.scripts.filter((s) => !inlineNames.has(s.name)),
  ...inlineScripts,
];

process.argv.push('-O1', '--target=c', '--2c-wasm-imports', '--no-run', '--no-opt-unused', '--quiet');

const _realExit = process.exit.bind(process);
process.exit = (code) => {
  if (code && code !== 0) _realExit(code);
};

const { createImport } = await import(porfforCompilerUrls.builtins.href);
const { default: compile } = await import(porfforCompilerUrls.index.href);

function registerAllImports() {
  createImport('print', 1, 0, () => {});
  createImport('printChar', 1, 0, () => {});
  createImport('time', 0, 1, () => 0);
  createImport('timeOrigin', 0, 1, () => 0);

  const imp = (name, params, returns = 0) => createImport(name, params, returns, null, null);

  imp('dong_print', 1, 0);
  imp('dong_bench_log', 1, 0);
  imp('dong_time_now', 0, 1);
  imp('dong_dom_getElementById', 1, 1);
  imp('dong_dom_set_textContent', 2, 0);
  imp('dong_dom_get_textContent', 1, 0);
  imp('dong_str_len', 0, 1);
  imp('dong_str_read', 2, 1);
  imp('dong_str_byte_at', 1, 1);
  imp('dong_str_pull', 0, 1);
  imp('dong_dom_addEventListener', 3, 0);
  imp('dong_timer_setTimeout', 2, 1);
  imp('dong_set_interval', 2, 1);
  imp('dong_clear_interval', 1, 0);
  imp('dong_clear_timeout', 1, 0);
  imp('dong_commit_setInterval', 0, 1);
  imp('dong_request_animation_frame', 1, 1);
  imp('dong_cancel_animation_frame', 1, 0);
  imp('dong_commit_requestAnimationFrame', 0, 1);
  imp('dong_raf_timestamp', 0, 1);
  imp('dong_state_set_num', 2, 0);
  imp('dong_state_get_num', 1, 1);
  imp('dong_stage_0', 1, 0);
  imp('dong_stage_1', 1, 0);
  imp('dong_stage_2', 1, 0);
  imp('dong_commit_set_textContent', 0, 0);
  imp('dong_commit_addEventListener', 0, 0);
  imp('dong_commit_setTimeout', 0, 1);

  imp('dong_get_value', 1, 0);
  imp('dong_set_value', 2, 0);
  imp('dong_get_checked', 1, 1);
  imp('dong_set_checked', 2, 0);
  imp('dong_get_disabled', 1, 1);
  imp('dong_set_disabled', 2, 0);
  imp('dong_get_attribute', 2, 0);
  imp('dong_set_attribute', 3, 0);
  imp('dong_remove_attribute', 2, 0);
  imp('dong_set_inner_html', 2, 0);
  imp('dong_query_selector', 2, 1);
  imp('dong_query_selector_all', 2, 0);
  imp('dong_get_elements_by_tag_name', 2, 0);
  imp('dong_class_add', 2, 0);
  imp('dong_class_remove', 2, 0);
  imp('dong_class_toggle', 2, 1);
  imp('dong_class_contains', 2, 1);
  imp('dong_style_set', 3, 0);
  imp('dong_style_get', 2, 0);
  imp('dong_computed_style_get', 2, 0);
  imp('dong_get_rect', 1, 0);
  imp('dong_get_metric', 2, 1);
  imp('dong_get_scroll_top', 1, 1);
  imp('dong_set_scroll_top', 2, 0);
  imp('dong_get_scroll_left', 1, 1);
  imp('dong_set_scroll_left', 2, 0);
  imp('dong_focus', 1, 0);
  imp('dong_blur', 1, 0);
  imp('dong_click', 1, 0);
  imp('dong_matches', 2, 1);
  imp('dong_closest', 2, 1);

  imp('dong_create_element', 1, 1);
  imp('dong_create_text_node', 1, 1);
  imp('dong_append_child', 2, 0);
  imp('dong_insert_before', 3, 0);
  imp('dong_remove', 1, 0);
  imp('dong_replace_child', 3, 0);
  imp('dong_parent', 1, 1);
  imp('dong_first_child', 1, 1);
  imp('dong_next_sibling', 1, 1);
  imp('dong_clone_node', 2, 1);

  imp('dong_parse_html', 1, 1);
  imp('dong_form_serialize', 1, 0);
  imp('dong_selection_text', 0, 0);
  imp('dong_select_all', 0, 1);
  imp('dong_exec_command', 2, 1);
  imp('dong_query_command_supported', 1, 1);

  imp('dong_event_type', 0, 0);
  imp('dong_event_target', 0, 1);
  imp('dong_event_key', 0, 0);
  imp('dong_event_key_code', 0, 1);
  imp('dong_event_x', 0, 1);
  imp('dong_event_y', 0, 1);
  imp('dong_event_button', 0, 1);
  imp('dong_event_modifiers', 0, 1);
  imp('dong_event_value', 0, 0);
  imp('dong_event_prevent_default', 0, 0);
  imp('dong_event_stop_propagation', 0, 0);

  imp('dong_fetch_start', 2, 1);
  imp('dong_commit_fetch_start', 0, 1);
  imp('dong_fetch_abort', 1, 0);
  imp('dong_fetch_request_id', 0, 1);
  imp('dong_fetch_status', 0, 1);
  imp('dong_fetch_ok', 0, 1);
  imp('dong_fetch_body', 0, 0);
  imp('dong_fetch_error', 0, 0);
  imp('dong_fetch_header', 1, 0);

  imp('dong_clipboard_write', 1, 0);
  imp('dong_clipboard_read', 0, 0);
  imp('dong_match_media', 1, 1);
  imp('dong_css_supports', 2, 1);
  imp('dong_dialog_show', 1, 0);
  imp('dong_dialog_show_modal', 1, 0);
  imp('dong_dialog_close', 2, 0);
  imp('dong_dialog_return_value', 1, 0);
  imp('dong_dialog_open', 1, 1);
  imp('dong_scene_add_node', 1, 1);
  imp('dong_scene_remove', 1, 0);
  imp('dong_scene_set', 3, 0);
  imp('dong_scene_find', 1, 1);
  imp('dong_scene_on', 3, 0);
  imp('dong_scene_clear', 0, 0);
  imp('dong_scene_count', 0, 1);
  imp('dong_text_layout', 1, 0);
  imp('dong_clear_overlay', 0, 0);
  imp('dong_render_text', 1, 0);
  imp('dong_draw_rect', 1, 0);
  imp('dong_draw_circle', 1, 0);
  imp('dong_pretext_typo_config', 2, 0);
  imp('dong_pretext_flow_columns_static', 0, 0);
  imp('dong_pretext_flow_columns_dynamic', 0, 0);
  imp('dong_pretext_flow_obstacles', 10, 0);
  imp('dong_text_layout_mount_lines', 4, 1);
  imp('dong_text_layout_render_overlay', 2, 1);
  imp('dong_pretext_dynamic_hud', 3, 0);
  imp('dong_pretext_flow_dynamic_tick', 8, 1);
  imp('dong_pretext_dual_mode_tick', 2, 0);
  imp('dong_pretext_domonly_init', 0, 0);
  imp('dong_pretext_domonly_tick', 1, 0);
  imp('dong_math_sin', 1, 1);
  imp('dong_math_cos', 1, 1);
  imp('dong_math_random', 0, 1);
  imp('dong_now_ms', 0, 1);
  imp('dong_style_set_px', 3, 0);
  imp('dong_num_to_str', 1, 0);
}

registerAllImports();

function stripDuplicateTypes(cSource) {
  const marker = 'struct ReturnValue';
  const idx = cSource.indexOf(marker);
  if (idx === -1) return cSource;
  const after = cSource.indexOf('\n\n', idx);
  if (after === -1) return cSource;
  return cSource.slice(after + 2);
}

function detectExports(source) {
  const names = [];
  const re = /export\s+function\s+(\w+)/g;
  let m;
  while ((m = re.exec(source)) !== null) {
    names.push(m[1]);
  }
  return names;
}

function detectExportParamCounts(source) {
  const counts = new Map();
  const re = /export\s+function\s+(\w+)\s*\(([^)]*)\)/g;
  let m;
  while ((m = re.exec(source)) !== null) {
    const params = m[2].split(',').map((p) => p.trim()).filter(Boolean);
    counts.set(m[1], params.length);
  }
  return counts;
}

function estimateStructSize(globals) {
  const sizeOf = { f64: 8, i32: 4, u32: 4, u8: 1, u16: 2, i64: 8, u64: 8 };
  let offset = 0;
  for (const g of globals) {
    const sz = sizeOf[g.type] ?? 4;
    const align = sz >= 8 ? 8 : sz >= 4 ? 4 : sz;
    offset = Math.ceil(offset / align) * align;
    offset += sz;
  }
  if (offset === 0) return 0;
  const tailAlign = 8;
  return Math.ceil(offset / tailAlign) * tailAlign;
}

function parseModuleStateGlobals(cSource, modPrefix) {
  const globals = [];
  const esc = modPrefix.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
  const re = new RegExp(
    `^static (f64|i32|u32|u8|u16|i64|u64) (${esc}\\w+)\\s*=`,
    'gm',
  );
  let m;
  while ((m = re.exec(cSource)) !== null) {
    globals.push({ type: m[1], name: m[2] });
  }
  return globals;
}

function appendModuleStateSnapshot(cSource, modPrefix, globals) {
  if (globals.length === 0) {
    return cSource;
  }
  const structName = `${modPrefix}state_t`;
  const structFields = globals.map((g) => `  ${g.type} ${g.name};`).join('\n');
  const captureBody = globals.map((g) => `  out->${g.name} = ${g.name};`).join('\n');
  const applyBody = globals.map((g) => `  ${g.name} = in->${g.name};`).join('\n');
  return `${cSource}

typedef struct {
${structFields}
} ${structName};

void ${modPrefix}state_capture(${structName}* out) {
${captureBody}
}

void ${modPrefix}state_apply(const ${structName}* in) {
${applyBody}
}
`;
}

function fixHostPulledStringTypes(cSource) {
  return cSource.replace(
    /(jjreturn\s*=\s*__porf_import_dong_str_pull\(\);\s*\n\s*jjreturnjjtype\s*=\s*)1(\s*;)/g,
    (_m, prefix, suffix) => `${prefix}195${suffix}`,
  );
}

function fixSetInnerHTMLStringTypes(cSource) {
  let out = cSource.replace(
    /(extern void __porf_import_dong_set_inner_html\(f64,\s*f64\);\n)/,
    `$1extern void __porf_import_dong_set_inner_html_typed(f64, f64, f64);\n`,
  );
  out = out.replace(
    /(_get0\s*=\s*nodeId;\s*\n\s*_get1\s*=\s*html;\s*\n\s*_get2\s*=\s*htmljjtype;\s*\n)\s*const struct ReturnValue _0 = ([A-Za-z0-9_]+)_toUtf8\(0, 0, 0, 0, _get1, _get2\);\s*\n\s*\(void\) _0\.type;\s*\n\s*__porf_import_dong_set_inner_html\(_get0, _0\.value\);/g,
    (_m, prefix) => `${prefix}    __porf_import_dong_set_inner_html_typed(_get0, _get1, _get2);`,
  );
  out = out.replace(
    /(_get\d+\s*=\s*html;\s*\n\s*const struct ReturnValue _\d+ = [A-Za-z0-9_]+_setInnerHTML\(0, 0, 0, 0, [^;]+, )195(\);)/g,
    '$1htmljjtype$2',
  );
  return out;
}

function fixStrcatResultTypes(cSource) {
  let out = cSource.replace(
    /(([A-Za-z0-9_]+__Porffor_strcat\()\s*(?:\(u32\)\()?(_\d+)\.value\)?\s*,\s*)195(\s*,)/g,
    (_m, prefix, _fn, retVar, suffix) => `${prefix}${retVar}.type${suffix}`,
  );
  out = out.replace(
    /(([A-Za-z0-9_]+__Porffor_concatStrings\()\s*(?:\(f64\)\()?(_\d+)\.value\)?\s*,\s*)195(\s*,)/g,
    (_m, prefix, _fn, retVar, suffix) => `${prefix}${retVar}.type${suffix}`,
  );
  return out;
}

function fixHtmlResultTypeAssignments(cSource) {
  let out = cSource.replace(
    /(html\s*=\s*(?:\(f64\))?\((_\d+)\.value\);\s*\n\s*_get\d+\s*=\s*html;\s*\n\s*)htmljjtype\s*=\s*195(\s*;)/g,
    (_m, prefix, retVar, suffix) => `${prefix}htmljjtype = ${retVar}.type${suffix}`,
  );
  out = out.replace(
    /(_get\d+\s*=\s*html;\s*\n\s*const struct ReturnValue _\d+ = [A-Za-z0-9_]+__Porffor_strcat\(\(u32\)\(_get\d+\),\s*)195(\s*,)/g,
    '$1htmljjtype$2',
  );
  return out;
}

function fixMallocReturnPointers(cSource) {
  return cSource.replace(
    /(\b([A-Za-z0-9_]+jjporfjjcurrentPtr)\s*=\s*\2\s*\+\s*(_get\d+);\s*\n\s*_r\d+\s*=\s*)\2(\s*;)/g,
    (_m, prefix, _currentPtr, lenVar, suffix) => `${prefix}${_currentPtr} - ${lenVar}${suffix}`,
  );
}

function prefixed2cSymbol(symbol, cPrefix) {
  if (symbol === '_memory') return `${cPrefix}memory`;
  if (symbol === '_memoryPages') return `${cPrefix}memory_pages`;
  if (symbol.startsWith('_')) return `${cPrefix}${symbol.slice(1)}`;
  return `${cPrefix}${symbol}`;
}

function collect2cSymbols(cSource) {
  const symbols = new Set();
  const add = (name) => {
    if (!name || name.startsWith('__porf_import_')) return;
    symbols.add(name);
  };

  let m;
  const memoryRe = /^char\*\s+([A-Za-z_]\w*)\s*;\s*u32\s+([A-Za-z_]\w*)\s*=/gm;
  while ((m = memoryRe.exec(cSource)) !== null) {
    add(m[1]);
    add(m[2]);
  }

  const globalRe = /^static\s+(?:f64|i32|u32|u8|u16|i64|u64|f32)\s+([A-Za-z_]\w*)\s*=/gm;
  while ((m = globalRe.exec(cSource)) !== null) {
    add(m[1]);
  }

  const fnRe =
    /^(?:static\s+)?(?:struct\s+ReturnValue|void|int|i32|u32|u8|u16|i64|u64|f32|f64|char\*)\s+([A-Za-z_]\w*)\s*\(/gm;
  while ((m = fnRe.exec(cSource)) !== null) {
    add(m[1]);
  }

  return symbols;
}

function replaceCIdentifiers(cSource, replacements) {
  let out = '';
  let i = 0;
  while (i < cSource.length) {
    const ch = cSource[i];
    const next = cSource[i + 1];

    if (ch === '"' || ch === "'") {
      const quote = ch;
      const start = i++;
      while (i < cSource.length) {
        if (cSource[i] === '\\') {
          i += 2;
          continue;
        }
        if (cSource[i] === quote) {
          i++;
          break;
        }
        i++;
      }
      out += cSource.slice(start, i);
      continue;
    }

    if (ch === '/' && next === '/') {
      const start = i;
      i += 2;
      while (i < cSource.length && cSource[i] !== '\n') i++;
      out += cSource.slice(start, i);
      continue;
    }

    if (ch === '/' && next === '*') {
      const start = i;
      i += 2;
      while (i < cSource.length && !(cSource[i] === '*' && cSource[i + 1] === '/')) i++;
      if (i < cSource.length) i += 2;
      out += cSource.slice(start, i);
      continue;
    }

    if (/[A-Za-z_]/.test(ch)) {
      const start = i++;
      while (i < cSource.length && /[A-Za-z0-9_]/.test(cSource[i])) i++;
      const ident = cSource.slice(start, i);
      out += replacements.get(ident) ?? ident;
      continue;
    }

    out += ch;
    i++;
  }
  return out;
}

function normalizePorffor2cPrefix(cSource, cPrefix) {
  if (cSource.includes(`int ${cPrefix}main(`)) return cSource;
  if (!/\bint\s+main\s*\(/.test(cSource)) return cSource;

  const replacements = new Map();
  for (const symbol of collect2cSymbols(cSource)) {
    if (!symbol.startsWith(cPrefix)) {
      replacements.set(symbol, prefixed2cSymbol(symbol, cPrefix));
    }
  }
  return replaceCIdentifiers(cSource, replacements);
}

function parseExportShims(cSource, modPrefix) {
  const shims = new Map();
  const esc = modPrefix.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
  const reVoid = new RegExp(`int ${esc}export_(\\w+)\\(void\\)`, 'g');
  let m;
  while ((m = reVoid.exec(cSource)) !== null) {
    shims.set(m[1], { symbol: `${modPrefix}export_${m[1]}`, params: 0 });
  }
  const re1 = new RegExp(`int ${esc}export_(\\w+)\\(f64 p0\\)`, 'g');
  while ((m = re1.exec(cSource)) !== null) {
    shims.set(m[1], { symbol: `${modPrefix}export_${m[1]}`, params: 1 });
  }
  const reN = new RegExp(`int ${esc}export_(\\w+)_p(\\d+)\\(`, 'g');
  while ((m = reN.exec(cSource)) !== null) {
    shims.set(m[1], {
      symbol: `${modPrefix}export_${m[1]}_p${m[2]}`,
      params: parseInt(m[2], 10),
    });
  }
  return shims;
}

function appendMissingExportShims(cSource, modPrefix, exports, exportParamCounts = new Map()) {
  if (!exports?.length) return cSource;

  let out = cSource;
  const shims = parseExportShims(out, modPrefix);
  const escPrefix = modPrefix.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');

  for (const exportName of exports) {
    if (shims.has(exportName)) continue;

    const escExport = exportName.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
    const fnRe = new RegExp(
      `(?:^|\\n)(?:static\\s+)?struct\\s+ReturnValue\\s+(${escPrefix}${escExport})\\(([^)]*)\\)`,
      'm',
    );
    const m = fnRe.exec(out);
    const fnSymbol = m?.[1] ?? null;
    const params = (m?.[2] ?? '').split(',').map((p) => p.trim()).filter(Boolean);
    const userPairs = fnSymbol
      ? Math.max(0, Math.floor((params.length - 4) / 2))
      : exportParamCounts.get(exportName) ?? 0;
    const shimSymbol =
      userPairs <= 1
        ? `${modPrefix}export_${exportName}`
        : `${modPrefix}export_${exportName}_p${userPairs}`;
    const cParams = [];
    const cArgs = ['0', '0', '0', '0'];
    for (let i = 0; i < userPairs; i++) {
      cParams.push(`f64 p${i}`);
      cArgs.push(`p${i}`, '1');
    }

    out += `
int ${shimSymbol}(${cParams.length ? cParams.join(', ') : 'void'}) {
  ${modPrefix}_porf_init();
  ${fnSymbol ? `(void)${fnSymbol}(${cArgs.join(', ')});` : '(void)0;'}
  return 0;
}
`;
    shims.set(exportName, { symbol: shimSymbol, params: userPairs });
  }

  return out;
}

function emitModuleC(item) {
  const modPrefix = `dong_porf_${item.name}_`;
  const globals = parseModuleStateGlobals(item.c, modPrefix);
  item.stateGlobals = globals;
  item.stateSizeBytes = estimateStructSize(globals);
  item.modPrefix = modPrefix;
  let body = stripDuplicateTypes(item.c);
  body = body.replace(/\bNaN\b/g, 'NAN').replace(/\bInfinity\b/g, 'INFINITY');
  body = appendModuleStateSnapshot(body, modPrefix, globals);
  const out = `// Porffor module: ${item.name}\n#include "dong_porf_runtime.h"\n${body}`;
  writeTextFileIfChanged(path.join(outDir, `${item.name}.c`), out);
}

function sleepMs(ms) {
  Atomics.wait(new Int32Array(new SharedArrayBuffer(4)), 0, 0, ms);
}

function writeTextFileIfChanged(filePath, content) {
  try {
    if (fs.existsSync(filePath) && fs.readFileSync(filePath, 'utf8') === content) {
      return;
    }
  } catch {
    // If reading fails because another process/tool has the file momentarily,
    // fall through to the retrying write path.
  }

  let lastError;
  for (let attempt = 0; attempt < 30; attempt += 1) {
    try {
      fs.writeFileSync(filePath, content);
      return;
    } catch (err) {
      lastError = err;
      sleepMs(50 + attempt * 25);
    }
  }
  throw lastError;
}

function sanitizeOutputStem(name) {
  return String(name).replace(/[^A-Za-z0-9_.-]/g, '_');
}

function setPorfforScratchOutput(logicalName) {
  const outFile = path.join(outDir, `_tmp_${process.pid}_${sanitizeOutputStem(logicalName)}.c`);
  process.argv = process.argv.filter((a) => !a.startsWith('-o='));
  process.argv.push(`-o=${outFile}`);
  globalThis.argvChanged?.();
  if (globalThis.Prefs) {
    globalThis.Prefs.o = outFile;
  }
  return outFile;
}

function compileSource(entry, source, logicalName, useModule) {
  lintPorfforSource(source, entry.path ?? logicalName);
  const cPrefix = `dong_porf_${logicalName}_`;
  const scratchOut = setPorfforScratchOutput(logicalName);
  const forcePrefixRewrite = process.env.DONG_PORFFOR_FORCE_PREFIX_REWRITE === '1';
  process.argv = process.argv.filter(
    (a) => !a.startsWith('--2c-prefix=') && !a.startsWith('--2cPrefix=') && a !== '--module',
  );
  if (!forcePrefixRewrite) {
    process.argv.push(`--2c-prefix=${cPrefix}`);
    process.argv.push(`--2cPrefix=${cPrefix}`);
  }
  if (useModule) {
    process.argv.push('--module');
  }
  globalThis.argvChanged?.();
  if (globalThis.Prefs) {
    globalThis.Prefs['2cPrefix'] = forcePrefixRewrite ? '' : cPrefix;
    globalThis.Prefs['2cSharedInit'] = true;
    globalThis.Prefs.module = useModule;
  }

  globalThis.file = entry.path ?? logicalName;
  let result;
  try {
    result = compile(source);
  } finally {
    try {
      fs.unlinkSync(scratchOut);
    } catch {
      // Best-effort cleanup only; the C text is returned by Porffor and emitted
      // by this script under its final generated/porffor/<module>.c path.
    }
  }
  let c = result?.c;
  if (!c) {
    throw new Error(`Porffor compile produced no C for ${logicalName}`);
  }
  c = normalizePorffor2cPrefix(c, cPrefix);
  if (!c.includes(`int ${cPrefix}main(`)) {
    throw new Error(
      `Porffor 2c prefix failed for ${logicalName}; expected symbol ${cPrefix}main in generated C`,
    );
  }
  return {
    name: logicalName,
    c: fixMallocReturnPointers(
      fixHtmlResultTypeAssignments(
        fixStrcatResultTypes(fixSetInnerHTMLStringTypes(fixHostPulledStringTypes(c))),
      ),
    ),
  };
}

function compileScript(entry) {
  const scriptPath = path.join(dongRoot, entry.path);
  const preludePath = path.join(dongRoot, manifest.prelude);
  const prelude = fs.readFileSync(preludePath, 'utf8');
  lintPorfforSource(prelude, manifest.prelude);
  const user = fs.readFileSync(scriptPath, 'utf8');
  const manifestExports = entry.exports ?? [];
  const detectedExports = detectExports(user);
  const exportParamCounts = detectExportParamCounts(user);
  const exports = manifestExports.length > 0 ? manifestExports : detectedExports;
  const useModule = exports.length > 0;
  const item = compileSource(entry, `${prelude}\n${user}`, entry.name, useModule);
  item.exports = exports;
  item.c = appendMissingExportShims(
    item.c,
    `dong_porf_${entry.name}_`,
    exports,
    exportParamCounts,
  );
  item.exportShims = useModule
    ? parseExportShims(item.c, `dong_porf_${entry.name}_`)
    : new Map();
  return item;
}

function compileHandler(parentName, handlerName, handlerPath) {
  const logicalName = `${parentName}__${handlerName}`;
  const preludePath = path.join(dongRoot, manifest.prelude);
  const prelude = fs.readFileSync(preludePath, 'utf8');
  const user = fs.readFileSync(path.join(dongRoot, handlerPath), 'utf8');
  return compileSource({ path: handlerPath }, `${prelude}\n${user}`, logicalName, false);
}

fs.mkdirSync(outDir, { recursive: true });

const compiled = [];
const handlers = [];

for (const entry of scriptEntries) {
  const item = compileScript(entry);
  compiled.push(item);
  emitModuleC(item);

  for (const exportName of item.exports ?? []) {
    const shim = item.exportShims.get(exportName);
    if (!shim) {
      throw new Error(`export ${exportName} not found in ${entry.name} 2c output`);
    }
    handlers.push({
      parent: entry.name,
      export_name: exportName,
      module_name: null,
      symbol: shim.symbol,
      params: shim.params,
    });
  }

  if (entry.handlers) {
    for (const [handlerName, handlerPath] of Object.entries(entry.handlers)) {
      const h = compileHandler(entry.name, handlerName, handlerPath);
      compiled.push(h);
      emitModuleC(h);
      handlers.push({
        parent: entry.name,
        export_name: handlerName,
        module_name: h.name,
        symbol: `dong_porf_${h.name}_main`,
        params: 0,
      });
    }
  }
}

const modules = compiled
  .filter((c) => !c.name.includes('__'))
  .map((c) => ({ name: c.name, symbol: `dong_porf_${c.name}_main` }));

const generatedSources = modules.map((m) => m.name);
for (const h of handlers) {
  if (h.module_name) {
    generatedSources.push(h.module_name);
  }
}
writeTextFileIfChanged(
  path.join(outDir, 'sources.cmake'),
  `set(DONG_PORFFOR_MODULE_SOURCES\n${generatedSources.map((s) => `  "\${DONG_PORFFOR_OUT_DIR}/${s}.c"`).join('\n')}\n)\n`,
);

const registryH = `// Auto-generated by porffor_compile.mjs — do not edit.
#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*dong_porf_main_fn)(void);
typedef int (*dong_porf_export_fn0)(void);
typedef int (*dong_porf_export_fn1)(double);

typedef void (*dong_porf_init_fn)(void);
typedef void (*dong_porf_state_capture_fn)(void* out);
typedef void (*dong_porf_state_apply_fn)(const void* in);

typedef struct dong_porf_module {
    const char* name;
    dong_porf_main_fn main_fn;
    char** memory;
    unsigned int* memory_pages;
    dong_porf_init_fn init_fn;
    dong_porf_state_capture_fn state_capture;
    dong_porf_state_apply_fn state_apply;
    size_t state_size;
} dong_porf_module_t;

typedef struct dong_porf_handler {
    const char* parent_module;
    const char* export_name;
    const char* legacy_handler_module;
    int param_count;
    dong_porf_export_fn0 fn0;
    dong_porf_export_fn1 fn1;
    char** memory;
    unsigned int* memory_pages;
} dong_porf_handler_t;

extern const dong_porf_module_t dong_porf_modules[];
extern const size_t dong_porf_module_count;

extern const dong_porf_handler_t dong_porf_handlers[];
extern const size_t dong_porf_handler_count;

const dong_porf_module_t* dong_porf_find_module(const char* name);
const dong_porf_handler_t* dong_porf_find_handler(const char* parent_module, const char* export_name);

#ifdef __cplusplus
}
#endif
`;

let registryC = `// Auto-generated by porffor_compile.mjs — do not edit.
#include "registry.h"
#include <string.h>

`;

for (const m of modules) {
  const item = compiled.find((c) => c.name === m.name);
  const prefix = `dong_porf_${m.name}_`;
  registryC += `extern int ${m.symbol}(void);\n`;
  registryC += `extern char* ${prefix}memory;\n`;
  registryC += `extern unsigned int ${prefix}memory_pages;\n`;
  registryC += `extern void ${prefix}_porf_init(void);\n`;
  if (item?.stateGlobals?.length) {
    registryC += `extern void ${prefix}state_capture(void* out);\n`;
    registryC += `extern void ${prefix}state_apply(const void* in);\n`;
  }
  registryC += `\n`;
}

for (const h of handlers) {
  if (h.params === 0) {
    registryC += `extern int ${h.symbol}(void);\n`;
  } else if (h.params === 1) {
    registryC += `extern int ${h.symbol}(double);\n`;
  }
  if (h.module_name) {
    registryC += `extern char* dong_porf_${h.module_name}_memory;\n`;
    registryC += `extern unsigned int dong_porf_${h.module_name}_memory_pages;\n\n`;
  } else {
    registryC += `\n`;
  }
}

registryC += `const dong_porf_module_t dong_porf_modules[] = {\n`;
for (const m of modules) {
  const item = compiled.find((c) => c.name === m.name);
  const prefix = `dong_porf_${m.name}_`;
  const hasState = item?.stateGlobals?.length > 0;
  const capture = hasState ? `${prefix}state_capture` : 'NULL';
  const apply = hasState ? `${prefix}state_apply` : 'NULL';
  const stateSize = hasState ? String(item.stateSizeBytes ?? 0) : '0';
  registryC += `  { "${m.name}", ${m.symbol}, &${prefix}memory, &${prefix}memory_pages, ${prefix}_porf_init, ${capture}, ${apply}, ${stateSize} },\n`;
}
registryC += `};\n\n`;
registryC += `const size_t dong_porf_module_count = ${modules.length};\n\n`;

registryC += `const dong_porf_handler_t dong_porf_handlers[] = {\n`;
for (const h of handlers) {
  const legacy = h.module_name ? `"${h.module_name}"` : 'NULL';
  const memMod = h.module_name ?? h.parent;
  const fn0 = h.params === 0 ? `(dong_porf_export_fn0)${h.symbol}` : 'NULL';
  const fn1 = h.params === 1 ? `(dong_porf_export_fn1)${h.symbol}` : 'NULL';
  registryC += `  { "${h.parent}", "${h.export_name}", ${legacy}, ${h.params}, ${fn0}, ${fn1}, &dong_porf_${memMod}_memory, &dong_porf_${memMod}_memory_pages },\n`;
}
registryC += `};\n\n`;
registryC += `const size_t dong_porf_handler_count = ${handlers.length};\n\n`;

registryC += `const dong_porf_module_t* dong_porf_find_module(const char* name) {
  if (!name) return NULL;
  for (size_t i = 0; i < dong_porf_module_count; ++i) {
    if (strcmp(dong_porf_modules[i].name, name) == 0) return &dong_porf_modules[i];
  }
  return NULL;
}

const dong_porf_handler_t* dong_porf_find_handler(const char* parent_module, const char* export_name) {
  if (!parent_module || !export_name) return NULL;
  for (size_t i = 0; i < dong_porf_handler_count; ++i) {
    if (strcmp(dong_porf_handlers[i].parent_module, parent_module) == 0 &&
        strcmp(dong_porf_handlers[i].export_name, export_name) == 0) {
      return &dong_porf_handlers[i];
    }
  }
  return NULL;
}
`;

writeTextFileIfChanged(path.join(outDir, 'registry.h'), registryH);
writeTextFileIfChanged(path.join(outDir, 'registry.c'), registryC);

console.log(
  `[porffor_compile] ${modules.length} module(s) + ${handlers.length} handler(s) -> ${outDir}`,
);
