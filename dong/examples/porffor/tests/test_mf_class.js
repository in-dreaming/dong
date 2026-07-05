var boxId = getElementById('box');
dongLog('test_mf_class loaded');

export function afterFrame0() {
  classAdd(boxId, 'highlight');
}
