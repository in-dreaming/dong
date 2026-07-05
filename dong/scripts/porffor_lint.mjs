#!/usr/bin/env node

const FORBIDDEN_RE = [
  { re: /\bPromise\b/, label: 'Promise' },
  { re: /\basync\b/, label: 'async' },
  { re: /\bawait\b/, label: 'await' },
];

/**
 * @param {string} source
 * @param {string} [fileLabel]
 */
export function lintPorfforSource(source, fileLabel = 'source') {
  for (const { re, label } of FORBIDDEN_RE) {
    if (re.test(source)) {
      throw new Error(
        `[porffor_lint] ${fileLabel}: ${label} is disabled (T19). Use callback exports + host result slots — see docs/developer/porffor/async-convention.md`,
      );
    }
  }
}

/**
 * @param {string} source
 * @param {string} [fileLabel]
 */
export function assertNoAsyncSyntax(source, fileLabel) {
  lintPorfforSource(source, fileLabel);
}
