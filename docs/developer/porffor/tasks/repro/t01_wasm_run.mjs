#!/usr/bin/env node
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath, pathToFileURL } from 'node:url';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const reproDir = __dirname;
const dongRoot = path.resolve(reproDir, '../../../../../dong');
const porfforRoot = path.join(dongRoot, 'third_party', 'porffor');
const testJs = path.join(reproDir, 't01_multiarg_import.js');

function registerTestImports(createImport, setImports) {
  setImports();
  createImport('dong_test0', 0, 1, () => 0);
  createImport('dong_test1', 1, 1, a => a);
  createImport('dong_test2', 2, 1, (a, b) => a + b);
  createImport('dong_test3', 3, 1, (a, b, c) => a + b + c);
}

const wrapUrl = pathToFileURL(path.join(porfforRoot, 'compiler/wrap.js')).href;
const builtinsUrl = pathToFileURL(path.join(porfforRoot, 'compiler/builtins.js')).href;
const { createImport, setImports } = await import(builtinsUrl);
const { default: wrap } = await import(wrapUrl);

registerTestImports(createImport, setImports);
const { exports } = wrap(fs.readFileSync(testJs, 'utf8'));
const ret = exports.main();
if (ret !== 19) {
  console.error(`FAIL: expected 19, got ${ret}`);
  process.exit(1);
}
console.log('OK: wasm path sum=19');
