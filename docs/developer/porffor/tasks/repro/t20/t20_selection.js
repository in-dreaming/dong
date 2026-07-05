export function main() {
  var txt = selectionText();
  if (txt !== '') {
    dongLog('T20_FAIL:selection_empty');
    return 1;
  }
  dongLog('T20_PASS:selection');
  return 0;
}
