#!/usr/bin/env node
/**
 * Build-time Porffor compiler for Dong manifest scripts.
 * Generates generated/porffor/*.c + registry.{h,c}
 */
import fs from 'node:fs';
import path from 'node:path';
import { dongRoot, ensurePorfforDeps, porfforCompilerUrls } from './porffor_paths.mjs';

ensurePorfforDeps();

const outDir = path.join(dongRoot, 'generated', 'porffor');
const manifestPath = path.join(dongRoot, 'scripts', 'porffor_manifest.json');

const manifest = JSON.parse(fs.readFileSync(manifestPath, 'utf8'));

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
  imp('dong_dom_get_textContent', 1, 1);
  imp('dong_dom_addEventListener', 3, 0);
  imp('dong_timer_setTimeout', 2, 1);
  imp('dong_state_set_num', 2, 0);
  imp('dong_state_get_num', 1, 1);
  imp('dong_stage_0', 1, 0);
  imp('dong_stage_1', 1, 0);
  imp('dong_stage_2', 1, 0);
  imp('dong_commit_set_textContent', 0, 0);
  imp('dong_commit_addEventListener', 0, 0);
  imp('dong_commit_setTimeout', 0, 1);
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

function emitModuleC(item) {
  const body = stripDuplicateTypes(item.c);
  const out = `// Porffor module: ${item.name}\n#include "dong_porf_runtime.h"\n${body}`;
  fs.writeFileSync(path.join(outDir, `${item.name}.c`), out);
}

function compileSource(entry, source, logicalName, useModule) {
  process.argv = process.argv.filter(
    (a) => !a.startsWith('--2c-prefix=') && a !== '--module',
  );
  process.argv.push(`--2c-prefix=dong_porf_${logicalName}_`);
  if (useModule) {
    process.argv.push('--module');
  }
  globalThis.argvChanged?.();

  globalThis.file = entry.path ?? logicalName;
  const result = compile(source);
  const c = result?.c;
  if (!c) {
    throw new Error(`Porffor compile produced no C for ${logicalName}`);
  }
  return { name: logicalName, c };
}

function compileScript(entry) {
  const scriptPath = path.join(dongRoot, entry.path);
  const preludePath = path.join(dongRoot, manifest.prelude);
  const prelude = fs.readFileSync(preludePath, 'utf8');
  const user = fs.readFileSync(scriptPath, 'utf8');
  const manifestExports = entry.exports ?? [];
  const detectedExports = detectExports(user);
  const exports = manifestExports.length > 0 ? manifestExports : detectedExports;
  const useModule = exports.length > 0;
  const item = compileSource(entry, `${prelude}\n${user}`, entry.name, useModule);
  item.exports = exports;
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

for (const entry of manifest.scripts) {
  const item = compileScript(entry);
  compiled.push(item);
  emitModuleC(item);

  if (!process.argv.some((a) => a.startsWith('-o='))) {
    process.argv.push(`-o=${path.join(outDir, '_unused.c')}`);
  }

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
fs.writeFileSync(
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

typedef struct dong_porf_module {
    const char* name;
    dong_porf_main_fn main_fn;
    char** memory;
    unsigned int* memory_pages;
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
  registryC += `extern int ${m.symbol}(void);\n`;
  registryC += `extern char* dong_porf_${m.name}_memory;\n`;
  registryC += `extern unsigned int dong_porf_${m.name}_memory_pages;\n\n`;
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
  registryC += `  { "${m.name}", ${m.symbol}, &dong_porf_${m.name}_memory, &dong_porf_${m.name}_memory_pages },\n`;
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

fs.writeFileSync(path.join(outDir, 'registry.h'), registryH);
fs.writeFileSync(path.join(outDir, 'registry.c'), registryC);

console.log(
  `[porffor_compile] ${modules.length} module(s) + ${handlers.length} handler(s) -> ${outDir}`,
);
