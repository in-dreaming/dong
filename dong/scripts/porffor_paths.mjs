/**
 * Resolve Porffor compiler from dong/third_party/porffor (git submodule).
 */
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { execFileSync } from 'node:child_process';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
export const dongRoot = path.resolve(__dirname, '..');
export const porfforRoot = path.join(dongRoot, 'third_party', 'porffor');

function gitOutput(args, cwd) {
  try {
    return execFileSync('git', args, {
      cwd,
      encoding: 'utf8',
      stdio: ['ignore', 'pipe', 'ignore'],
    }).trim();
  } catch {
    return '';
  }
}

function assertPorfforSubmodulePinned() {
  const lsFiles = gitOutput(['ls-files', '-s', 'third_party/porffor'], dongRoot);
  const expected = lsFiles.match(/^160000\s+([0-9a-f]{40})\s+/)?.[1] ?? '';
  if (!expected) {
    return;
  }

  const actual = gitOutput(['rev-parse', 'HEAD'], porfforRoot);
  if (!actual) {
    throw new Error(
      `Porffor submodule is not a git checkout at ${porfforRoot}. ` +
        'Run: git submodule update --init --recursive third_party/porffor',
    );
  }
  if (actual !== expected) {
    throw new Error(
      `Porffor submodule checkout mismatch.\n` +
        `  expected: ${expected}\n` +
        `  actual:   ${actual}\n` +
        'Run: git submodule update --init --recursive third_party/porffor',
    );
  }
}

export function ensurePorfforDeps() {
  const compilerIndex = path.join(porfforRoot, 'compiler', 'index.js');
  if (!fs.existsSync(compilerIndex)) {
    throw new Error(
      `Porffor submodule missing at ${porfforRoot}. ` +
        'Run: git submodule update --init --recursive third_party/porffor',
    );
  }
  assertPorfforSubmodulePinned();
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
