var editor = getElementById('editor');
var logId = getElementById('log');

if (editor !== 0) {
  focusNode(editor);
  selectAll();
  var ok = execCommand('bold', '');
  setTextContent(logId, 'auto: selectAll + bold -> ' + (ok ? 'true' : 'false'));
}
