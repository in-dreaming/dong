var editorA = getElementById('editorA');
var logId = getElementById('log');

if (editorA !== 0) {
  focusNode(editorA);
  selectAll();
  var ok = execCommand('bold', '');
  setTextContent(logId, 'auto bold(dynamic): ' + (ok ? 'true' : 'false'));
}
