#!/usr/bin/env node
/** T22 framework lint — extends porffor_lint.mjs */
import { lintPorfforSource } from './porffor_lint.mjs';

const FRAMEWORK_FORBIDDEN = [
  { re: /=>/, label: 'arrow functions (closure risk)' },
  { re: /\beval\s*\(/, label: 'eval()' },
  { re: /\bnew\s+Function\b/, label: 'Function constructor' },
  { re: /\bimport\s*\(/, label: 'dynamic import()' },
  { re: /\.bind\s*\(/, label: '.bind() (closure risk)' },
];

/**
 * @param {string} source
 * @param {string} [fileLabel]
 */
export function lintFrameworkSource(source, fileLabel = 'source') {
  lintPorfforSource(source, fileLabel);
  for (const { re, label } of FRAMEWORK_FORBIDDEN) {
    if (re.test(source)) {
      throw new Error(
        `[porffor_framework_lint] ${fileLabel}: ${label} is forbidden — see docs/developer/porffor/framework-spec.md §8`,
      );
    }
  }
}
