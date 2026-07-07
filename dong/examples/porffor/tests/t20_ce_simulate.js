var editor = getElementById('editor');
var logId = getElementById('log');

function appendLog(msg) {
  var prev = getTextContent(logId);
  setTextContent(logId, prev + msg + '\n');
}

focusNode(editor);
selectAll();
appendLog('bold: ' + (execCommand('bold', '') ? 'true' : 'false'));
setTextContent(editor, 'Hello World Test');
focusNode(editor);
selectAll();
appendLog('italic: ' + (execCommand('italic', '') ? 'true' : 'false'));
setTextContent(editor, 'Hello World Test');
focusNode(editor);
selectAll();
appendLog('underline: ' + (execCommand('underline', '') ? 'true' : 'false'));
