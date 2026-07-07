var editor = getElementById('editor');
var logId = getElementById('log');
var btnBold = getElementById('btnBold');
var btnItalic = getElementById('btnItalic');
var btnUnderline = getElementById('btnUnderline');

function appendLog(msg) {
  var prev = getTextContent(logId);
  setTextContent(logId, prev + msg + '\n');
}

export function onBold() {
  focusNode(editor);
  selectAll();
  appendLog('bold: ' + (execCommand('bold', '') ? 'true' : 'false'));
}

export function onItalic() {
  focusNode(editor);
  selectAll();
  appendLog('italic: ' + (execCommand('italic', '') ? 'true' : 'false'));
}

export function onUnderline() {
  focusNode(editor);
  selectAll();
  appendLog('underline: ' + (execCommand('underline', '') ? 'true' : 'false'));
}

addEventListener(btnBold, 'click', 'onBold');
addEventListener(btnItalic, 'click', 'onItalic');
addEventListener(btnUnderline, 'click', 'onUnderline');
onBold();
onItalic();
onUnderline();
