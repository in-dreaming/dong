export function main() {
  var rootId = parseHtml('<div id="x"><span>hi</span></div>');
  if (rootId === 0) {
    dongLog('T20_FAIL:parse');
    return 1;
  }
  var childId = getFirstChildId(rootId);
  if (childId === 0) {
    dongLog('T20_FAIL:child');
    return 1;
  }
  var txt = getTextContent(childId);
  if (txt !== 'hi') {
    dongLog('T20_FAIL:text');
    return 1;
  }
  dongLog('T20_PASS:parse');
  return 0;
}
