#!/usr/bin/env node
/**
 * Test harness for Porffor custom imports + 2cWasmImports.
 */
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const dongRoot = path.resolve(__dirname, '../..');
const outFile = path.join(dongRoot, 'scripts/porffor_spike/test_import.c');

process.argv.push('-O1', '--target=c', '--2c-wasm-imports', '--no-run', `-o=${outFile}`);

import { ensurePorfforDeps, porfforCompilerUrls } from '../porffor_paths.mjs';

ensurePorfforDeps();

const { createImport, setImports } = await import(porfforCompilerUrls.builtins.href);
const { default: compile } = await import(porfforCompilerUrls.index.href);

setImports();
createImport('dong_print', 1, 0, null, null);
createImport('dong_dom_getElementById', 1, 1, null, null);

const source = `
var el = dong_dom_getElementById('status');
dong_print('hello dom test');
`;

globalThis.file = 'test_import.js';
compile(source);
// compile() calls process.exit() on success when -o is set
