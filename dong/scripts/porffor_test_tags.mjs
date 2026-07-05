#!/usr/bin/env node
/**
 * Parse <!-- porffor: ready|pending|blocked(Txx)|dropped(reason) --> from HTML tests.
 * Default: no <script> → ready; has <script> → pending.
 */
import fs from 'node:fs';

const TAG_RE =
  /<!--\s*porffor:\s*(ready|pending|blocked\(([^)]+)\)|dropped\(([^)]+)\))\s*-->/i;
const HAS_SCRIPT_RE = /<script[\s>]/i;

/**
 * @param {string} html
 * @returns {{ status: 'ready'|'pending'|'blocked'|'dropped', reason?: string, explicit: boolean }}
 */
export function parsePorfforTagFromHtml(html) {
  const m = html.match(TAG_RE);
  if (m) {
    const raw = m[1].toLowerCase();
    if (raw === 'ready') return { status: 'ready', explicit: true };
    if (raw === 'pending') return { status: 'pending', explicit: true };
    if (raw.startsWith('blocked')) {
      return { status: 'blocked', reason: (m[2] ?? '').trim(), explicit: true };
    }
    if (raw.startsWith('dropped')) {
      return { status: 'dropped', reason: (m[3] ?? '').trim(), explicit: true };
    }
  }
  const hasScript = HAS_SCRIPT_RE.test(html);
  return { status: hasScript ? 'pending' : 'ready', explicit: false };
}

/**
 * @param {string} filePath
 */
export function parsePorfforTagFile(filePath) {
  const html = fs.readFileSync(filePath, 'utf8');
  return parsePorfforTagFromHtml(html);
}

/**
 * @param {string} html
 * @returns {string|null}
 */
export function extractPorfforModule(html) {
  const m = html.match(/data-porffor-module=["']([^"']+)["']/i);
  return m ? m[1] : null;
}

/**
 * @param {string} html
 * @returns {{ usesEvalSnippet: boolean, snippetPath: string|null }}
 */
export function detectEvalSnippetRef(html) {
  const m = html.match(/--eval-after-frame0-file\s+([^\s<>]+\.js)/);
  if (m) return { usesEvalSnippet: true, snippetPath: m[1] };
  const note = html.match(/eval-after-frame0-file\s+snippets\/([^\s"'<>]+\.js)/i);
  if (note) return { usesEvalSnippet: true, snippetPath: `snippets/${note[1]}` };
  return { usesEvalSnippet: false, snippetPath: null };
}

/**
 * Classify a script-bearing test for migration planning.
 * @param {string} html
 * @param {string} fileName
 */
export function classifyScriptTest(html, fileName) {
  const lower = html.toLowerCase();
  const name = fileName.toLowerCase();

  if (name.includes('react') || name.includes('preact')) {
    return { bucket: 'blocked(T18)', note: 'framework bundle' };
  }
  if (/\breact\b|\bpreact\b|createelement\s*\(/.test(lower) && name.includes('game')) {
    return { bucket: 'blocked(T18)', note: 'framework-like' };
  }
  if (/execcommand|getselection|createrange|contenteditable/.test(lower)) {
    return { bucket: 'blocked(T20)', note: 'CE/selection/execCommand' };
  }
  if (detectEvalSnippetRef(html).usesEvalSnippet) {
    return { bucket: 'T14-snippet', note: 'multiframe eval snippet' };
  }
  if (/onclick\s*=|onchange\s*=|oninput\s*=|onload\s*=/.test(lower)) {
    return { bucket: 'blocked(T12)', note: 'inline event handler' };
  }
  if (/\beval\s*\(|\bnew\s+function\b|import\s*\(|dynamic\s+import/.test(lower)) {
    return { bucket: 'blocked', note: 'dynamic eval/import' };
  }
  if (/\basync\b|\bawait\b|\bpromise\b/.test(lower)) {
    return { bucket: 'blocked(T19)', note: 'async/Promise' };
  }
  if (/formdata|domparser|ime|clipboard|fetch\s*\(/.test(lower)) {
    return { bucket: 'blocked(T20)', note: 'platform API gap' };
  }
  if (/console\.log|performance\.now|getelementbyid|textcontent/.test(lower)) {
    return { bucket: 'direct', note: 'simple DOM/console/perf' };
  }
  return { bucket: 'pending-review', note: 'needs manual triage' };
}
