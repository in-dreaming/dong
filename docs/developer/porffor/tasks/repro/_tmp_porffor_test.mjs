import path from 'node:path';
import { pathToFileURL } from 'node:url';
const porfforRoot = 'E:/ws/infra/dong/dong/third_party/porffor';
const wrapUrl = pathToFileURL(path.join(porfforRoot, 'compiler/wrap.js')).href;
const { default: wrap } = await import(wrapUrl);
const source = process.argv[2];
const label = process.argv[3];
let out = '';
try {
  wrap(source, undefined, s => out += s);
  console.log('OK ' + label, out.trim().slice(0, 300));
} catch (e) {
  console.error('FAIL ' + label, e.stack || e);
  process.exit(1);
}