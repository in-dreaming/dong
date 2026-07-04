/**
 * Resolve Porffor compiler from dong/third_party/porffor (git submodule).
 */
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
export const dongRoot = path.resolve(__dirname, '..');
export const porfforRoot = path.join(dongRoot, 'third_party', 'porffor');

export function ensurePorfforDeps() {
  const compilerIndex = path.join(porfforRoot, 'compiler', 'index.js');
  if (!fs.existsSync(compilerIndex)) {
    throw new Error(
      `Porffor submodule missing at ${porfforRoot}. Run: git submodule update --init dong/third_party/porffor`,
    );
  }
  const acorn = path.join(porfforRoot, 'node_modules', 'acorn');
  if (!fs.existsSync(acorn)) {
    throw new Error(
      `Porffor npm dependencies missing. Run: cd third_party/porffor && npm install`,
    );
  }
}

export const porfforCompilerUrls = {
  builtins: new URL('../third_party/porffor/compiler/builtins.js', import.meta.url),
  index: new URL('../third_party/porffor/compiler/index.js', import.meta.url),
};
