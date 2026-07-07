#!/usr/bin/env node
/**
 * Resolve T13 pending tests → ready | blocked(Txx) | dropped(reason).
 * See docs/developer/porffor/tasks/T13-inventory-t20.md
 */
import {
  classifyScriptTest,
  detectEvalSnippetRef,
} from './porffor_test_tags.mjs';

/** @type {Record<string, string | { ready?: boolean, dropped?: string }>>} */
export const FILE_OVERRIDES = {
  // T20 决策矩阵（细化阻塞任务）
  'test_selection_api.html': 'T14',
  'test_selection_mouse.html': 'T14',
  'test_selection_pseudo.html': 'T14',
  'test_contenteditable_basic.html': 'T14',
  'test_contenteditable_typing.html': 'T14',
  'test_contenteditable_features.html': 'T14',
  'test_contenteditable_auto.html': 'T14',
  'test_contenteditable_prebolded.html': 'T14',
  'test_contenteditable_bold_auto.html': 'T14',
  'test_ime_composition.html': 'T14',
  'test_ce_debug.html': 'T14',
  'test_ce_enter_offset.html': 'T14',
  'test_ce_interactive_sim.html': 'T14',
  'test_ce_mixed_multiline.html': 'T14',
  'test_ce_simulate.html': 'T14',
  'innerhtml_test.html': 'T06',
  'innerhtml_auto_test.html': 'T06',
  'test_formdata_api.html': 'T06',
  'test_formdata_name_serialization.html': 'T06',
  'test_domparser_api.html': 'T06',
  // T11 平台 API
  'test_cssom_matchmedia.html': 'T11',
  'test_cssom_supports.html': 'T11',
  'test_porffor_clipboard_cn.html': 'T11',
  'test_clipboard_api.html': 'T11',
  'test_event_clipboard.html': 'T11',
  'test_dialog_modal.html': 'T11',
  'test_dialog_show.html': 'T11',
  'test_dialog_backdrop.html': 'T11',
  'test_matchmedia.html': 'T11',
  // T10 fetch（回调已实现，测试待 Porffor 模块）
  'test_fetch_json_basic.html': 'T10',
  'test_fetch_text_basic.html': 'T10',
  'test_fetch_reject.html': 'T10',
  // frame-0 渲染冒烟
  'test_img_alt_text.html': { ready: true },
  // pending-review 定稿
  'queryselector_complex_test.html': 'T20',
  'stylesheets_deleterule_test.html': 'T20',
  'stylesheets_insertrule_test.html': 'T20',
  'test_gamepad_full_flow.html': { dropped: 'gamepad platform API' },
  'test_p0_comprehensive.html': { dropped: 'comprehensive demo page' },
  'skin_test.html': { dropped: 'skin/theme experiment' },
};

const BATCH_ORDER = [
  'T12',
  'T14',
  'T06',
  'T10',
  'T11',
  'T19',
  'T20',
  'dynamic',
  'review',
];

/**
 * @param {string} bucket
 * @returns {string|null}
 */
export function bucketToBlockedReason(bucket) {
  const m = bucket.match(/^blocked\(([^)]+)\)$/);
  if (m) return m[1];
  if (bucket === 'blocked') return 'dynamic';
  if (bucket === 'T14-snippet') return 'T14';
  return null;
}

/**
 * @param {string} html
 * @param {string} fileName
 * @returns {{ status: 'ready'|'blocked'|'dropped', reason?: string, batch?: string, bucket: string }}
 */
export function resolvePendingDisposition(html, fileName) {
  const bucket = classifyScriptTest(html, fileName).bucket;
  const override = FILE_OVERRIDES[fileName];

  if (override && typeof override === 'object') {
    if (override.ready) {
      return { status: 'ready', bucket };
    }
    if (override.dropped) {
      return { status: 'dropped', reason: override.dropped, batch: 'dropped', bucket };
    }
  }
  if (typeof override === 'string') {
    return { status: 'blocked', reason: override, batch: override, bucket };
  }

  if (detectEvalSnippetRef(html).usesEvalSnippet) {
    return { status: 'blocked', reason: 'T14', batch: 'T14', bucket: 'T14-snippet' };
  }

  const reason = bucketToBlockedReason(bucket);
  if (reason) {
    return { status: 'blocked', reason, batch: reason, bucket };
  }

  if (bucket === 'pending-review') {
    return { status: 'blocked', reason: 'review', batch: 'review', bucket };
  }

  return { status: 'blocked', reason: 'review', batch: 'review', bucket };
}

export function formatPorfforTag(disposition) {
  if (disposition.status === 'ready') {
    return '<!-- porffor: ready -->';
  }
  if (disposition.status === 'dropped') {
    return `<!-- porffor: dropped(${disposition.reason}) -->`;
  }
  return `<!-- porffor: blocked(${disposition.reason}) -->`;
}

/**
 * @param {Array<{ file: string, disposition: ReturnType<typeof resolvePendingDisposition> }>} rows
 */
export function summarizeBatches(rows) {
  /** @type {Record<string, string[]>} */
  const batches = {};
  for (const { file, disposition } of rows) {
    const key =
      disposition.status === 'ready'
        ? 'promoted-ready'
        : disposition.status === 'dropped'
          ? 'dropped'
          : `blocked(${disposition.batch ?? disposition.reason})`;
    if (!batches[key]) batches[key] = [];
    batches[key].push(file);
  }
  const ordered = [];
  for (const key of BATCH_ORDER.map((b) => `blocked(${b})`)) {
    if (batches[key]?.length) {
      ordered.push([key, batches[key]]);
    }
  }
  if (batches.dropped?.length) ordered.push(['dropped', batches.dropped]);
  if (batches['promoted-ready']?.length) {
    ordered.push(['promoted-ready', batches['promoted-ready']]);
  }
  for (const [key, files] of Object.entries(batches)) {
    if (!ordered.some(([k]) => k === key)) {
      ordered.push([key, files]);
    }
  }
  return ordered;
}
