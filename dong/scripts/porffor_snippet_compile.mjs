#!/usr/bin/env node
/**
 * Wrap eval snippets as Porffor export modules; merge into porffor_manifest.json snippets section.
 * Pilot snippets are hand-written in prelude style under examples/porffor/snippets/.
 */
import fs from 'node:fs';
import path from 'node:path';
import { dongRoot } from './porffor_paths.mjs';

const manifestPath = path.join(dongRoot, 'scripts', 'porffor_manifest.json');
const pilotSnippetsDir = path.join(dongRoot, 'examples', 'porffor', 'snippets');

function exportNameFromFile(fileName) {
  const base = fileName.replace(/\.js$/, '');
  return `__snippet_${base.replace(/[^a-zA-Z0-9_]/g, '_')}`;
}

function wrapSnippetAsModule(source, exportName) {
  return `${source.trim()}\n\nexport function ${exportName}() {}\n`;
}

const manifest = JSON.parse(fs.readFileSync(manifestPath, 'utf8'));
if (!manifest.snippets) manifest.snippets = [];

const existing = new Set(manifest.snippets.map((s) => s.name));

if (fs.existsSync(pilotSnippetsDir)) {
  for (const file of fs.readdirSync(pilotSnippetsDir).filter((f) => f.endsWith('.js'))) {
    const name = file.replace(/\.js$/, '');
    if (existing.has(name)) continue;
    const relPath = `examples/porffor/snippets/${file}`;
    const raw = fs.readFileSync(path.join(dongRoot, relPath), 'utf8');
    const exportName = exportNameFromFile(file);
    manifest.snippets.push({
      name,
      path: relPath,
      export: exportName,
      hasExport: raw.includes('export function'),
    });
    existing.add(name);
  }
}

const snippetDir = path.join(dongRoot, 'examples', 'data', 'tests', 'snippets');
if (fs.existsSync(snippetDir)) {
  for (const file of fs.readdirSync(snippetDir).filter((f) => f.endsWith('.js'))) {
    const name = `qj_${file.replace(/\.js$/, '')}`;
    if (existing.has(name)) continue;
    const srcPath = `examples/data/tests/snippets/${file}`;
    const raw = fs.readFileSync(path.join(dongRoot, srcPath), 'utf8');
    const exportName = exportNameFromFile(file);
    const wrappedPath = `examples/porffor/snippets/generated/${file}`;
    const wrappedFull = path.join(dongRoot, wrappedPath);
    fs.mkdirSync(path.dirname(wrappedFull), { recursive: true });
    if (!raw.includes('export function')) {
      fs.writeFileSync(
        wrappedFull,
        `// Auto-wrapped legacy snippet — needs prelude rewrite before Porffor use\n// Source: ${srcPath}\n${wrapSnippetAsModule(`// TODO prelude rewrite\n`, exportName)}`,
      );
    }
    manifest.snippets.push({
      name,
      path: wrappedPath,
      export: exportName,
      source: srcPath,
      status: 'plan_b_stub',
    });
    existing.add(name);
  }
}

fs.writeFileSync(manifestPath, `${JSON.stringify(manifest, null, 2)}\n`);
console.log(`[porffor_snippet_compile] manifest snippets: ${manifest.snippets.length}`);
