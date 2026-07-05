var statusId = getElementById('status');
setTextContent(statusId, 'Before');
dongLog('test_mf_text loaded');

export function afterFrame0() {
  setTextContent(statusId, 'After');
}
